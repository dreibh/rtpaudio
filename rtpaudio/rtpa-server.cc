// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Server                                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de.de                               ####
// ####    WWW:   http://www.iem.uni-due.de.de/~dreibh/rn                ####
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
// $Id$


#include "tdsystem.h"
#include "tdsocket.h"
#include "rtpsender.h"
#include "rtcppacket.h"
#include "rtcpreceiver.h"
#include "rtcpabstractserver.h"
#include "audioclientapppacket.h"
#include "tools.h"
#include "breakdetector.h"


#include "audioserver.h"


#define SCTP_MAXADDRESSES 20


// Fast Break: Disable break detector to debug thread deadlocks
// #define FAST_BREAK


// Globals
Socket*                  rtcpServerSocket  = NULL;
RTCPReceiver*            rtcpReceiver      = NULL;
AudioServer*             server            = NULL;
BandwidthManager*        qosManager        = NULL;


void cleanUp(const cardinal exitCode = 0);


// ###### Initialize ########################################################
void initAll(const char*    directory,
             const card16   port,
             const card64   timeout,
             const cardinal maxPacketSize,
             const bool     lossScalability,
             const bool     useSCTP)
{
   const InternetAddress localAddress(port);
   rtcpServerSocket = new Socket(Socket::IP,
                                 useSCTP ? Socket::SeqPacket : Socket::Datagram,
                                 useSCTP ? Socket::SCTP : Socket::Default);
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
      delete rtcpReceiver;
   }
   if(server != NULL) {
      delete server;
   }
   if(rtcpServerSocket != NULL) {
      delete rtcpServerSocket;
   }
   /*
   if(qosManager != NULL) {
      delete qosManager;
   }
   */
   if(exitCode == 0) {
      std::cout << "Terminated!" << std::endl;
   }
   exit(exitCode);
}


// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Initialize ======================================================
   bool   optForceIPv4    = false;
   bool   optUseSCTP      = false;
   bool   lossScalability = true;
   bool   disableQM       = false;
   char*  manager         = NULL;
   cardinal maxPacketSize = 1500;
   card64 timeout         = 10000000;
   card16 port            = AudioClientAppPacket::RTPAudioDefaultPort;
   String directory;


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
      else if(!(strncasecmp(argv[i],"-manager=",9)))     manager   = &argv[i][9];
      else if(!(strncasecmp(argv[i],"-timeout=",9)))     timeout   = 1000000 * (card64)atol(&argv[i][9]);
      else if(!(strncasecmp(argv[i],"-maxpktsize=",12))) maxPacketSize = (cardinal)atol(&argv[i][12]);
      else if(!(strcasecmp(argv[i],"-disable-qm")))      disableQM = true;
      else if(!(strcasecmp(argv[i],"-enable-qm")))       disableQM = false;
      else if(!(strcasecmp(argv[i],"-disable-ls")))      lossScalability = false;
      else if(!(strcasecmp(argv[i],"-enable-ls")))       lossScalability = true;
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
      /*
      qosManager = new QoSManager();
      if(qosManager == NULL) {
         std::cerr << "ERROR: Server::main() - Out of memory!" << std::endl;
         cleanUp(1);
      }
      */
   }


   // ====== Initialize =====================================================
   initAll(directory.getData(), port,
           timeout, maxPacketSize, lossScalability,
           optUseSCTP);
#ifndef FAST_BREAK
   installBreakDetector();
#endif

   InternetAddress ourAddress;
   rtcpServerSocket->getSocketAddress(ourAddress);
   ourAddress.setPrintFormat(InternetAddress::PF_Address);


   // ====== Print status ===================================================
   std::cout << "RTP Audio Server - Copyright (C) 1999-2012 Thomas Dreibholz" << std::endl;
   std::cout << "-----------------------------------------------------------" << std::endl;
   std::cout << std::endl;
   std::cout << "Version:          " << __DATE__ << ", " << __TIME__ << std::endl;
   if(optUseSCTP) {
      std::cout << "SCTP:             on" << std::endl;
   }
   else {
      std::cout << "SCTP:             off" << std::endl;
   }
   std::cout << "Server Port:      " << port << std::endl;
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",server->getOurSSRC());
   std::cout << "Server SSRC:      " << str << std::endl;
   std::cout << "Client Timeout:   " << (timeout / 1000000) << " [s]" << std::endl;
   std::cout << "Input Directory:  " << directory << std::endl;
   std::cout << "Max Packet Size:  " << maxPacketSize << std::endl;
   std::cout << "Loss Scalability: " << (lossScalability ? "on" : "off") << std::endl;
   std::cout << std::endl;


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
