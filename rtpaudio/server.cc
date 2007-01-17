// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Server                                                     ####
// ####                                                                  ####
// #### Version 1.50  --  August 01, 2001                                ####
// ####                                                                  ####
// ####            Copyright (C) 1999-2001 by Thomas Dreibholz           ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@exp-math.uni-essen.de                           ####
// ####    WWW:   http://www.exp-math.uni-essen.de/~dreibh/rtpaudio      ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software; you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License      ####
// #### as published by the Free Software Foundation; either version 2   ####
// #### of the License, or (at your option) any later version.           ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// ##########################################################################


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


using namespace Coral;


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
void initAll(const char*              directory,
             SocketAddress**          localAddressArray,
             const cardinal           localAddresses,
             const card16             port,
             const card64             timeout,
             const cardinal           maxPacketSize,
             const bool               lossScalability,
             const bool               useSCTP)
{
   rtcpServerSocket = new Socket(Socket::IP,Socket::UDP,useSCTP ? Socket::SCTP : Socket::Default);
   if(rtcpServerSocket == NULL) {
      cerr << "ERROR: Server::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   if(useSCTP) {
      struct sctp_event_subscribe events;

      memset((char*)&events, 0 ,sizeof(events));
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events)) < 0) {
         cerr << "WARNING: AudioClient::play() - SCTP_EVENTS failed!" << endl;
      }
      sctp_initmsg init;
      init.sinit_num_ostreams   = 1;
      init.sinit_max_instreams  = 1;
      init.sinit_max_attempts   = 0;
      init.sinit_max_init_timeo = 60;
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_INITMSG,(char*)&init,sizeof(init)) < 0) {
         cerr << "WARNING: AudioServer::newClient() - Unable to set SCTP_INITMSG parameters!" << endl;
      }
   }
   for(cardinal i = 0;i < localAddresses;i++) {
      localAddressArray[i]->setPort(port);
   }
   if(rtcpServerSocket->bindx((const SocketAddress**)localAddressArray,
                              localAddresses,
                              SCTP_BINDX_ADD_ADDR) == false) {
      cerr << "ERROR: Server::initAll() - Unable to bind socket!" << endl;
      cleanUp(1);
   }
   for(cardinal i = 0;i < localAddresses;i++) {
      localAddressArray[i]->setPort(0);
   }
   server = new AudioServer(localAddressArray,localAddresses,qosManager,maxPacketSize,useSCTP);
   if(server == NULL) {
      cerr << "ERROR: Server::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   server->setDefaultTimeout(timeout);
   server->setLossScalability(lossScalability);
   rtcpReceiver = new RTCPReceiver(server,rtcpServerSocket);
   if(rtcpReceiver == NULL) {
      cerr << "ERROR: Server::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   if(server->start() == false) {
      cerr << "ERROR: Server::initAll() - Unable to start server thread!" << endl;
      cleanUp(1);
   }
   if(rtcpReceiver->start() == false) {
      cerr << "ERROR: Server::initAll() - Unable to start RTCP receiver thread!" << endl;
      cleanUp(1);
   }

   // ====== Change directory ===============================================
   if(directory != NULL) {
      if(chdir(directory) != 0) {
         cerr << "ERROR: Server::initAll() - Unable to change directory!" << endl;
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
   }  ??????????�   */
   if(exitCode == 0) {
      cout << "Terminated!" << endl;
   }
   exit(exitCode);
}


// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Initialize ======================================================
   bool   optForceIPv4               = false;
   bool   optUseSCTP                 = false;
   bool   lossScalability            = true;
   bool   disableQM                  = false;
   char*  manager                    = NULL;
   cardinal maxPacketSize            = 1500;
   card64 timeout                    = 10000000;
   card16 port                       = AudioClientAppPacket::RTPAudioDefaultPort;
   SocketAddress** localAddressArray = NULL;
   cardinal        localAddresses    = 0;
   String          directory;


   // ====== Read configuration from file ===================================
   FILE* inputFD = fopen("AudioServer.config","r");
   if(inputFD != NULL) {
      char str[256];
      char* result = fgets((char*)&str,256,inputFD);
      cardinal line = 0;
      while(!feof(inputFD)) {
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
                           cerr << "ERROR: Bad port setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Port = <number>" << endl;
                           exit(1);
                        }
                        port = (card16)portNumber;
                     }
                     else if(name == "TIMEOUT") {
                        int timeoutValue;
                        if(sscanf(value.getData(),"%d",&timeoutValue) != 1) {
                           cerr << "ERROR: Bad timeout setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Timeout = <seconds>" << endl;
                           exit(1);
                        }
                        timeout = 1000000 * (card64)timeoutValue;
                     }
                     else if(name == "MAX PACKET SIZE") {
                        int size;
                        if(sscanf(value.getData(),"%d",&size) != 1) {
                           cerr << "ERROR: Bad maximum packet size setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Max Packet Size = <Bytes>" << endl;
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
                           cerr << "ERROR: Bad QoS manager setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Disable QoS Manager = <0|1>" << endl;
                           exit(1);
                        }
                        disableQM = (off != 0) ? true : false;
                     }
                     else if(name == "QOS MANAGER") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad QoS manager setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: QoS Manager = <0|1>" << endl;
                           exit(1);
                        }
                        disableQM = (on != 0) ? false : true;
                     }
                     else if(name == "LOSS SCALABILITY") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad loss scalability setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Loss Scalability = <0|1>" << endl;
                           exit(1);
                        }
                        lossScalability = (on != 0) ? true : false;
                     }
                     else if(name == "FORCE IPV4") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad force IPv4 setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Force IPv4 = <0|1>" << endl;
                           exit(1);
                        }
                        optForceIPv4 = (on != 0) ? true : false;
                     }
                     else if(name == "SCTP") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad SCTP setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: SCTP = <0|1>" << endl;
                           exit(1);
                        }
                        optUseSCTP = (on != 0) ? true : false;
                     }
                     else {
                        cerr << "ERROR: Unknown option <"
                             << name << " = " << value << ">, "
                                "line " << line << "!" << endl;
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
      else if(!(strncasecmp(argv[i],"-local=",7))) {
         if(localAddresses < SCTP_MAXADDRESSES) {
            if(localAddressArray == NULL) {
               localAddressArray = SocketAddress::newAddressList(SCTP_MAXADDRESSES);
               if(localAddressArray == NULL) {
                  cerr << "ERROR: Out of memory!" << endl;
                  exit(1);
               }
            }
            localAddressArray[localAddresses] = SocketAddress::createSocketAddress(
                                                   SocketAddress::PF_HidePort,
                                                   &argv[i][7]);
            if(localAddressArray[localAddresses] == NULL) {
               cerr << "ERROR: Argument #" << i << " is an invalid address!" << endl;
               exit(1);
            }
            localAddresses++;
         }
         else {
            cerr << "ERROR: Too many local addresses!" << endl;
            exit(1);
         }
      }
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
         cerr << "Usage: " << argv[0] << " {-port=port} {-directory=path} {-manager=host:port} {-local=host} {-timeout=secs} {-maxpktsize=bytes} {-disable-qm|-enable-qm} {-disable-ls|-enable-ls} {-force-ipv4|-use-ipv6}" << endl;
         exit(1);
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         cerr << "NOTE: IPv6 support disabled!" << endl;
      }
   }
   if(port < 1024) {
      cerr << "ERROR: Invalid port number!" << endl;
      exit(1);
   }
   if(localAddressArray == NULL) {
      if(optUseSCTP == true) {
         if(!Socket::getLocalAddressList(
               localAddressArray,
               localAddresses,
               Socket::GLAF_HideBroadcast|Socket::GLAF_HideMulticast|Socket::GLAF_HideAnycast)) {
            cerr << "ERROR: Cannot obtain local addresses!" << endl;
            exit(1);
         }
         if(localAddresses < 1) {
            cerr << "ERROR: No valid local addresses have been found?!" << endl
                 << "       Check your network interface configuration!" << endl;
            exit(1);
         }
      }
      else {
         localAddressArray = SocketAddress::newAddressList(SCTP_MAXADDRESSES);
         if(localAddressArray == NULL) {
            cerr << "ERROR: Out of memory!" << endl;
            exit(1);
         }
         localAddressArray[0] = new InternetAddress(port);
         if(localAddressArray[0] == NULL) {
            cerr << "ERROR: Out of memory!" << endl;
            exit(1);
         }
         localAddresses = 1;
      }
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
         cerr << "ERROR: Server::main() - Out of memory!" << endl;
         cleanUp(1);
      }
      ??????
      */
   }


   // ====== Initialize =====================================================
   initAll(directory.getData(),
           localAddressArray,
           localAddresses,
           port,
           timeout, maxPacketSize, lossScalability,
           optUseSCTP);
#ifndef FAST_BREAK
   installBreakDetector();
#endif

   InternetAddress ourAddress;
   rtcpServerSocket->getSocketAddress(ourAddress);
   ourAddress.setPrintFormat(InternetAddress::PF_Address);


   // ====== Print status ===================================================
   cout << "RTP Audio Server - Copyright (C) 1999-2002 Thomas Dreibholz" << endl;
   cout << "-----------------------------------------------------------" << endl;
   cout << endl;
   cout << "Version:          " << __DATE__ << ", " << __TIME__ << endl;
   if(optUseSCTP) {
      cout << "SCTP:             on" << endl;
   }
   else {
      cout << "SCTP:             off" << endl;
   }
   localAddressArray[0]->setPrintFormat(SocketAddress::PF_Address|SocketAddress::PF_HidePort);
   cout << "Local Addresses:  " << *(localAddressArray[0]) << endl;
   for(cardinal i = 1;i < localAddresses;i++) {
      localAddressArray[i]->setPrintFormat(SocketAddress::PF_Address|SocketAddress::PF_HidePort);
      cout << "                  " << *(localAddressArray[i]) << endl;
   }
   cout << "Server Port:      " << port << endl;
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",server->getOurSSRC());
   cout << "Server SSRC:      " << str << endl;
   cout << "Client Timeout:   " << (timeout / 1000000) << " [s]" << endl;
   cout << "Input Directory:  " << directory << endl;
   cout << "Max Packet Size:  " << maxPacketSize << endl;
   cout << "Loss Scalability: " << (lossScalability ? "on" : "off") << endl;
   cout << endl;


   // ====== Main loop ======================================================
   for(;;) {
#ifndef FAST_BREAK
      if(breakDetected())
         break;
#endif
      Thread::delay(10000000,true);
   }

   // ====== Clean up =======================================================
   SocketAddress::deleteAddressList(localAddressArray);
   cleanUp(0);
}
