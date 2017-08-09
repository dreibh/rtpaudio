// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Server                                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   https://www.uni-due.de/~be0001/rtpaudio                ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software: you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License as   ####
// #### published by the Free Software Foundation, either version 3 of   ####
// #### the License, or (at your option) any later version.              ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// #### You should have received a copy of the GNU General Public        ####
// #### License along with this program.  If not, see                    ####
// #### <http://www.gnu.org/licenses/>.                                  ####
// ####                                                                  ####
// ##########################################################################


#include "tdsystem.h"
#include "tdsocket.h"
#include "rtpsender.h"
#include "rtcppacket.h"
#include "rtcpreceiver.h"
#include "rtcpabstractserver.h"
#include "audioclientapppacket.h"
#include "audioserver.h"
#include "tools.h"
#include "breakdetector.h"

#define WITH_QOSMGR
#ifdef WITH_QOSMGR
#include "bandwidthmanager.h"
#include "servicelevelagreement.h"
#if 0
#include "roundtriptimepinger.h"
#endif
#endif

#include <fstream>



// Fast Break: Disable break detector to debug thread deadlocks
// #define FAST_BREAK


static Socket*                rtcpServerSocket  = NULL;
static RTCPReceiver*          rtcpReceiver      = NULL;
static AudioServer*           server            = NULL;
static BandwidthManager*      qosManager        = NULL;
static ServiceLevelAgreement* sla               = NULL;
static Socket*                pingSocket4       = NULL;
static Socket*                pingSocket6       = NULL;
static RoundTripTimePinger*   pinger            = NULL;
static std::ofstream*         logStream         = NULL;


void cleanUp(const cardinal exitCode = 0);


// ###### Initialize ########################################################
void initAll(const char*    directory,
             const card16   port,
             const card64   timeout,
             const cardinal maxPacketSize,
             const bool     lossScalability,
             const bool     useSCTP,
             const char*    neatProperties)
{
   const InternetAddress localAddress(port);
   rtcpServerSocket = new Socket(Socket::IP,
                                 useSCTP ? Socket::SeqPacket : Socket::Datagram,
                                 useSCTP ? Socket::SCTP : Socket::Default,
                                 neatProperties);
   if(rtcpServerSocket == NULL) {
      std::cerr << "ERROR: Server::initAll() - Out of memory!" << std::endl;
      cleanUp(1);
   }
   if(rtcpServerSocket->bind(localAddress) == false) {
      std::cerr << "ERROR: Server::initAll() - Unable to bind socket!" << std::endl;
      cleanUp(1);
   }
   if(useSCTP) {
      rtcpServerSocket->listen(10);
   }
   server = new AudioServer(qosManager, maxPacketSize, useSCTP);
   if(server == NULL) {
      std::cerr << "ERROR: Server::initAll() - Out of memory!" << std::endl;
      cleanUp(1);
   }
   server->setDefaultTimeout(timeout);
   server->setLossScalability(lossScalability);
   rtcpReceiver = new RTCPReceiver(server,rtcpServerSocket);
   if(rtcpReceiver == NULL) {
      std::cerr << "ERROR: Server::initAll() - Out of memory!" << std::endl;
      cleanUp(1);
   }
   if(server->start() == false) {
      std::cerr << "ERROR: Server::initAll() - Unable to start server thread!" << std::endl;
      cleanUp(1);
   }
   if(rtcpReceiver->start() == false) {
      std::cerr << "ERROR: Server::initAll() - Unable to start RTCP receiver thread!" << std::endl;
      cleanUp(1);
   }

   // ====== Change directory ===============================================
   if(directory != NULL) {
      if(chdir(directory) != 0) {
         std::cerr << "ERROR: Server::initAll() - Unable to change directory!" << std::endl;
         cleanUp(1);
      }
   }
   else {
      directory = "./";
   }
}


// ###### Clean up ##########################################################
void cleanUp(const cardinal exitCode)
{
   if(rtcpReceiver != NULL) {
      rtcpReceiver->stop();
      delete rtcpReceiver;
      rtcpReceiver = NULL;
   }
   if(server != NULL) {
      server->stop();
      delete server;
      server = NULL;
   }
   if(rtcpServerSocket != NULL) {
      delete rtcpServerSocket;
      rtcpServerSocket = NULL;
   }
   if(qosManager != NULL) {
      delete qosManager;
      qosManager = NULL;
   }
   if(exitCode == 0) {
      std::cout << "Terminated!" << std::endl;
   }
   exit(exitCode);
}


// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Initialize ======================================================
   bool     optForceIPv4           = false;
   bool     optUseSCTP             = false;
   bool     lossScalability        = true;
   bool     disableQM              = false;
   double   fairnessSession        = 0.0;
   double   fairnessStream         = 1.0;
   bool     prEnabled              = true;
   double   prReservedPortion      = 0.1;
   double   prUtilizationTolerance = 0.05;
   double   prMaxRemappingInterval = 5000000.0;
   cardinal maxRUPoints            = 32;
   double   utThreshold            = 0.01;
   card64   bwThreshold            = (card64)-1;
   double   sdTolerance            = 50000.0;
   bool     unlayered              = false;
   cardinal maxPacketSize          = 1500;
   card64   timeout                = 10000000;
   card16   port                   = RTPAudioDefaultPort;
   char*    logName                = NULL;
   String   slaFile("SLA.config");
   String   directory;

   const char* neatProperties = NULL;
//    "{\
//       \"transport\": [\
//          {\
//                \"value\": \"SCTP\",\
//                \"precedence\": 1\
//          }\
//       ]\
//    }";\


   // ====== Read configuration from file ===================================
   FILE* inputFD = fopen("AudioServer.config","r");
   if(inputFD != NULL) {
      char str[256];
      char* result = fgets((char*)&str,256,inputFD);
      cardinal line = 0;
      while( (result != NULL) && (!feof(inputFD)) ) {
         line++;
         const cardinal inputLength = strlen((char*)&str);
         if(inputLength > 1) {
            str[inputLength - 1] = 0x00;
            switch(str[0]) {
               // ====== Line is a comment =====================================
               case '#':
                break;

               // ====== Line is a setting =====================================
               default:
                  const String input(str);
                  String name;
                  String value;
                  if(input.scanSetting(name,value)) {
                     if(name == "PORT") {
                        int portNumber;
                        if(sscanf(value.getData(),"%d",&portNumber) != 1) {
                           std::cerr << "ERROR: Bad port setting, "
                                   "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: Port = <number>" << std::endl;
                           exit(1);
                        }
                        port = (card16)portNumber;
                     }
                     else if(name == "TIMEOUT") {
                        int timeoutValue;
                        if(sscanf(value.getData(),"%d",&timeoutValue) != 1) {
                           std::cerr << "ERROR: Bad timeout setting, "
                                        "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: Timeout = <seconds>" << std::endl;
                           exit(1);
                        }
                        timeout = 1000000 * (card64)timeoutValue;
                     }
                     else if(name == "MAX PACKET SIZE") {
                        int size;
                        if(sscanf(value.getData(),"%d",&size) != 1) {
                           std::cerr << "ERROR: Bad maximum packet size setting, "
                                        "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: Max Packet Size = <Bytes>" << std::endl;
                           exit(1);
                        }
                        maxPacketSize = (cardinal)size;
                     }
                     else if(name == "DIRECTORY") {
                        directory = value;
                     }
                     else if(name == "DISABLE QOS MANAGER") {
                        int off;
                        if(sscanf(value.getData(),"%d",&off) != 1) {
                           std::cerr << "ERROR: Bad QoS manager setting, "
                                        "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: Disable QoS Manager = <0|1>" << std::endl;
                           exit(1);
                        }
                        disableQM = (off != 0) ? true : false;
                     }
                     else if(name == "QOS MANAGER") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           std::cerr << "ERROR: Bad QoS manager setting, "
                                   "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: QoS Manager = <0|1>" << std::endl;
                           exit(1);
                        }
                        disableQM = (on != 0) ? false : true;
                     }
                     else if(name == "LOSS SCALABILITY") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           std::cerr << "ERROR: Bad loss scalability setting, "
                                        "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: Loss Scalability = <0|1>" << std::endl;
                           exit(1);
                        }
                        lossScalability = (on != 0) ? true : false;
                     }
                     else if(name == "FORCE IPV4") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           std::cerr << "ERROR: Bad force IPv4 setting, "
                                        "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: Force IPv4 = <0|1>" << std::endl;
                           exit(1);
                        }
                        optForceIPv4 = (on != 0) ? true : false;
                     }
                     else if(name == "SET PARTIAL REMAPPING") {
                        int    on;
                        if(sscanf(value.getData(),
                                  "%d R=%lf T=%lf X=%lf",
                                  &on,&prReservedPortion,&prUtilizationTolerance,
                                  &prMaxRemappingInterval) > 0) {
                           prEnabled = (on != 0);
                        }
                     }
                     else if(name == "SET FAIRNESS") {
                        sscanf(value.getData(),
                               "%lf %lf",
                               &fairnessSession,&fairnessStream);
                     }
                     else if(name == "SET QOS OPTIMIZATION") {
                        int maxInt;
                        if(sscanf(value.getData(),
                                  "%u U=%lf B=%llu T=%lf",
                                  &maxInt,&utThreshold,
                                  (unsigned long long*)&bwThreshold,
                                  &sdTolerance) > 0) {
                           maxRUPoints = (cardinal)maxInt;
                        }
                     }
                     else if(name == "SCTP") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           std::cerr << "ERROR: Bad SCTP setting, "
                                        "line " << line << "!" << std::endl;
                           std::cerr << "       Syntax: SCTP = <0|1>" << std::endl;
                           exit(1);
                        }
                        optUseSCTP = (on != 0) ? true : false;
                     }
                     else {
                        std::cerr << "ERROR: Unknown option <"
                                  << name << " = " << value << ">, "
                                  "line " << line << "!" << std::endl;
                        exit(1);
                     }
                  }
                break;
            }
         }
         result = fgets((char*)&str,256,inputFD);
      }
      fclose(inputFD);
   }


   // ===== Check arguments =================================================
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strcasecmp(argv[i],"-force-ipv4")))           optForceIPv4 = true;
      else if(!(strcasecmp(argv[i],"-use-ipv6")))        optForceIPv4 = false;
      else if(!(strcasecmp(argv[i],"-sctp")))            optUseSCTP   = true;
      else if(!(strcasecmp(argv[i],"-nosctp")))          optUseSCTP   = false;
      else if(!(strncasecmp(argv[i],"-port=",6)))        port      = (card16)atol(&argv[i][6]);
      else if(!(strncasecmp(argv[i],"-timeout=",9)))     timeout   = 1000000 * (card64)atol(&argv[i][9]);
      else if(!(strncasecmp(argv[i],"-maxpktsize=",12))) maxPacketSize = (cardinal)atol(&argv[i][12]);
      else if(!(strcasecmp(argv[i],"-disable-qm")))      disableQM = true;
      else if(!(strcasecmp(argv[i],"-enable-qm")))       disableQM = false;
      else if(!(strcasecmp(argv[i],"-disable-ls")))      lossScalability = false;
      else if(!(strcasecmp(argv[i],"-enable-ls")))       lossScalability = true;
      else if(!(strncasecmp(argv[i],"-sla=",5)))         slaFile       = &argv[i][5];
      else if(!(strncasecmp(argv[i],"-log=",5)))         logName      = &argv[i][5];
      else if(!(strncasecmp(argv[i],"-directory=",11)))  directory = String(&argv[i][11]);
      else {
         std::cerr << "Usage: " << argv[0] << " {-port=port} {-directory=path} {-manager=host:port} {-timeout=secs} {-maxpktsize=bytes} {-disable-qm|-enable-qm} {-disable-ls|-enable-ls} {-force-ipv4|-use-ipv6}" << std::endl;
         exit(1);
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         std::cerr << "NOTE: IPv6 support disabled!" << std::endl;
      }
   }
   if(port < 1024) {
      std::cerr << "ERROR: Invalid port number!" << std::endl;
      exit(1);
   }
   if(timeout < 5000000) {
      timeout = 5000000;
   }
   else if(timeout > 1800000000) {
      timeout = 1800000000;
   }
   if(maxPacketSize < 256) {
      maxPacketSize = 256;
   }
   else if(maxPacketSize > 1024 * 1024) {
      maxPacketSize = 1024 * 1024;
   }


   // ====== Initialize QoS manager =========================================
   if(disableQM == false) {
      if(logName != NULL) {
         logStream = new std::ofstream(logName);
         if((logStream == NULL) || (!logStream->good())) {
            std::cerr << "ERROR: Unable to create log file!" << std::endl;
            exit(1);
         }
      }

      // ====== Check EUID ==================================================
      uid_t uid = geteuid();
      if(uid != 0) {
         std::cerr << "ERROR: Only root is allowed to run this program!" << std::endl;
         exit(1);
      }

      // ====== Open sockets ================================================
      pingSocket4 = new Socket(Socket::IPv4,Socket::Raw,Socket::ICMPv4);
      if(pingSocket4 == NULL) {
         std::cerr << "ERROR: Server::main() - Out of memory!" << std::endl;
         cleanUp(1);
      }
      if(!pingSocket4->bind()) {
         std::cerr << "ERROR: Unable to bind socket for ICMPv4!" << std::endl;
         exit(1);
      }
      pingSocket6 = NULL;
      if(InternetAddress::hasIPv6()) {
         pingSocket6 = new Socket(Socket::IPv6,Socket::Raw,Socket::ICMPv6);
         if(pingSocket6 == NULL) {
            std::cerr << "ERROR: Out of memory!" << std::endl;
            exit(1);
         }
         if(!pingSocket6->bind()) {
            std::cerr << "ERROR: Unable to bind socket for ICMPv6!" << std::endl;
            exit(1);
         }
      }

      // ====== Create RoundTripTimePinger =====================================
      pinger = new RoundTripTimePinger(pingSocket4,pingSocket6);
      if(pinger == NULL) {
         std::cerr << "ERROR: Server::main() - Out of memory!" << std::endl;
         cleanUp(1);
      }
      if(!pinger->ready()) {
         std::cerr << "ERROR: RoundTripTimePinger not ready!" << std::endl;
         exit(1);
      }

      // ====== Initialize QoS manager ======================================
      sla = new ServiceLevelAgreement();
      if(sla->load(slaFile.getData()) == false) {
         std::cerr << "ERROR: Unable to load SLA configuration file <"
                   << slaFile << ">!"<< std::endl;
         cleanUp(1);
      }
      qosManager = new BandwidthManager(sla,NULL);
      if(qosManager == NULL) {
         std::cerr << "ERROR: Server::main() - Out of memory!" << std::endl;
         cleanUp(1);
      }
      qosManager->setLogStream(logStream);
      qosManager->setFairness(fairnessSession,fairnessStream);
      qosManager->setQoSOptimizationParameters(maxRUPoints,utThreshold,bwThreshold,sdTolerance,unlayered);
      qosManager->setPartialRemapping(prEnabled,prReservedPortion,
                                     prUtilizationTolerance,prMaxRemappingInterval);
      qosManager->getFairness(fairnessSession,fairnessStream);
      qosManager->getQoSOptimizationParameters(maxRUPoints,utThreshold,bwThreshold,sdTolerance,unlayered);
      qosManager->getPartialRemapping(prEnabled,prReservedPortion,
                                     prUtilizationTolerance,prMaxRemappingInterval);
      qosManager->start();
#if 0
      pinger->start();
#endif
   }


   // ====== Initialize =====================================================
   initAll(directory.getData(), port,
           timeout, maxPacketSize, lossScalability,
           optUseSCTP, neatProperties);
#ifndef FAST_BREAK
   installBreakDetector();
#endif

   InternetAddress ourAddress;
   rtcpServerSocket->getSocketAddress(ourAddress);
   ourAddress.setPrintFormat(InternetAddress::PF_Address);


   // ====== Print status ===================================================
   std::cout << "RTP Audio Server - Copyright (C) 1999-2017 Thomas Dreibholz" << std::endl
             << "-----------------------------------------------------------" << std::endl
             << std::endl;
   if(optUseSCTP) {
      std::cout << "SCTP:             on" << std::endl;
   }
   else {
      std::cout << "SCTP:             off" << std::endl;
   }
   std::cout << "Server Port:      " << port << std::endl;
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",server->getOurSSRC());
   std::cout << "Server SSRC:      " << str << std::endl
             << "Client Timeout:   " << (timeout / 1000000) << " [s]" << std::endl
             << "Input Directory:  " << directory << std::endl
             << "Max Packet Size:  " << maxPacketSize << std::endl
             << "Loss Scalability: " << (lossScalability ? "on" : "off") << std::endl
             << std::endl;


   // ====== Main loop ======================================================
   for(;;) {
#ifndef FAST_BREAK
      if(breakDetected())
         break;
#endif
      Thread::delay(10000000,true);
   }


   // ====== Clean up =======================================================
   cleanUp(0);
}
