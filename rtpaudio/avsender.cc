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
#include "congestionmanagerclient.h"
#include "audioclientapppacket.h"
#include "tools.h"
#include "breakdetector.h"


#include "audioserver.h"
#include "QoS/qosmanager.h"


#include <netdb.h>
#include <netinet/in.h>
#include "session.hh"
#include "socket/socket.hh"


// Globals
QoSManager*              qosManager        = NULL;
CongestionManagerClient* cmClient          = NULL;

Socket*                  rtcpServerSocket  = NULL;
InternetAddress*         rtcpServerAddress = NULL;
RTCPReceiver*            rtcpReceiver      = NULL;
AudioServer*             server            = NULL;

::Socket          RTPSocket;
::Socket          RTCPSocket;
::Socket          FECSocket;
::Prog4D_Session* Prog4DSession = NULL;


void cleanUp(const cardinal exitCode = 0);


// ###### Initialize ########################################################
void initAll(const char*& directory,
             const char*  local,
             const char*  premote,
             const card64 timeout,
             const card16 port)
{
   // ====== Initialize Prog4D ==============================================
   int local_port  = 6000;
   int remote_port = 5000;

   sockaddr_in local_addr;
   local_addr.sin_family = AF_INET;
   if((local_port < RTP_MIN_PORT) || (local_port > RTP_MAX_PORT)) {
      cerr << "ERROR: AVSender::initAll() - Local port out of range." << endl;
      cleanUp(1);
   }
   local_addr.sin_port = htons(local_port);
   local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

   sockaddr_in remote_addr;
   remote_addr.sin_family = AF_INET;
   if((remote_port < RTP_MIN_PORT) || (remote_port > RTP_MAX_PORT)) {
       cerr << "ERROR: AVSender::initAll() - Remote port out of range." << endl;
       cleanUp(1);
   }
   remote_addr.sin_port = htons(remote_port);
   struct hostent *hp = gethostbyname(premote);
   if(hp == NULL) {
      cerr << "ERROR: AVSender::initAll() - gethostbyname() failed!" << endl;
      cleanUp(1);
   }
   remote_addr.sin_family = hp->h_addrtype;
   memcpy(&remote_addr.sin_addr.s_addr, hp->h_addr, hp->h_length);

   // ***********************************************************************
   RTPSocket.create();
   RTPSocket.assignName((struct sockaddr *) &local_addr, sizeof(local_addr));
   RTPSocket.initConnection((struct sockaddr *) &remote_addr, sizeof(remote_addr));
   local_addr.sin_port  = htons(local_port  + 1);
   remote_addr.sin_port = htons(remote_port + 1);

   RTCPSocket.create();
   RTCPSocket.assignName((struct sockaddr *) &local_addr, sizeof(local_addr));
   RTCPSocket.initConnection((struct sockaddr *) &remote_addr, sizeof(remote_addr));
   local_addr.sin_port  = htons(local_port  + 2);
   remote_addr.sin_port = htons(remote_port + 2);

   FECSocket.create();
   FECSocket.assignName((struct sockaddr *) &local_addr, sizeof(local_addr));
   FECSocket.initConnection((struct sockaddr *) &remote_addr, sizeof(remote_addr));

   Prog4DSession = new Prog4D_Session(qosManager);
   if(Prog4DSession == NULL) {
      cerr << "ERROR: AVSender::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   Prog4DSession->transmit();


   // ====== Initialize RTP Audio ===========================================
   rtcpServerSocket = new Socket(Socket::IP,Socket::UDP);
   if(rtcpServerSocket == NULL) {
      cerr << "ERROR: AVSender::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   rtcpServerAddress = new InternetAddress(local,port);
   if(rtcpServerAddress == NULL) {
      cerr << "ERROR: AVSender::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   if(rtcpServerSocket->bind(*rtcpServerAddress) == false) {
      cerr << "ERROR: AVSender::initAll() - Unable to bind socket!" << endl;
      exit(1);
   }
   server = new AudioServer(qosManager,1500,cmClient);
   if(server == NULL) {
      cerr << "ERROR: AVSender::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   server->setDefaultTimeout(timeout);
   rtcpReceiver = new RTCPReceiver(server,rtcpServerSocket);
   if(rtcpReceiver == NULL) {
      cerr << "ERROR: AVSender::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   if(server->start() == false) {
      cerr << "ERROR: AVSender::initAll() - Unable to start server thread!" << endl;
      cleanUp(1);
   }
   if(rtcpReceiver->start() == false) {
      cerr << "ERROR: AVSender::initAll() - Unable to start RTCP receiver thread!" << endl;
      cleanUp(1);
   }

   // ====== Change directory ===============================================
   if(directory != NULL) {
      if(chdir(directory) != 0) {
         cerr << "ERROR: AVSender::initAll() - Unable to change directory!" << endl;
         exit(1);
      }
   }
   else {
      directory = "./";
   }
}


// ###### Clean up ##########################################################
void cleanUp(const cardinal exitCode)
{
   if(rtcpReceiver != NULL)
      rtcpReceiver->stop();
   if(server != NULL)
      server->stop();
   if(rtcpReceiver != NULL)
      delete rtcpReceiver;
   if(server != NULL)
      delete server;
   if(rtcpServerSocket != NULL)
      delete rtcpServerSocket;
   if(rtcpServerAddress != NULL)
      delete rtcpServerAddress;
   if(cmClient != NULL)
      delete cmClient;
   if(Prog4DSession)
      delete Prog4DSession;
   if(qosManager != NULL)
      delete qosManager;

   exit(exitCode);
}


// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Check arguments =================================================
   bool   optForceIPv4 = false;
   bool   disableQM    = false;
   char*  premote      = NULL;
   char*  local        = NULL;
   char*  manager      = NULL;
   char*  directory    = NULL;
   card64 timeout      = 10000000;
   card16 port         = AudioClientAppPacket::RTPAudioDefaultPort;
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strcasecmp(argv[i],"-force-ipv4")))          optForceIPv4 = true;
      else if(!(strncasecmp(argv[i],"-local=",7)))      local        = &argv[i][7];
      else if(!(strncasecmp(argv[i],"-port=",6)))       port         = (card16)atol(&argv[i][6]);
      else if(!(strncasecmp(argv[i],"-premote=",9)))    premote      = &argv[i][9];
      else if(!(strncasecmp(argv[i],"-manager=",9)))    manager      = &argv[i][9];
      else if(!(strncasecmp(argv[i],"-timeout=",9)))    timeout   = 1000000 * (card64)atol(&argv[i][9]);
      else if(!(strcasecmp(argv[i],"-disable-qm")))     disableQM = true;
      else if(!(strncasecmp(argv[i],"-directory=",11))) directory = &argv[i][11];
      else {
         cerr << "Usage: " << argv[0] << " {-port=port} {-directory=path} {-manager=host:port} {-local=hostname} {-premote=hostname} {-timeout=secs} {-force-ipv4}" << endl;
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
   if(premote == NULL) {
      premote = "localhost";
      cerr << "NOTE: Using default \"localhost\" as Prog4D remote host parameter!" << endl;
      cerr << "      (Change with -premote=hostname, if necessary.)" << endl;
   }
   if(timeout < 5000000)
      timeout = 5000000;
   if(timeout > 1800000000)
      timeout = 1800000000;


   // ====== Initialize QoS manager =========================================
   if(disableQM == false) {
      qosManager = new QoSManager();
      if(qosManager == NULL) {
         cerr << "ERROR: AVSender::main() - Out of memory!" << endl;
         cleanUp(1);
      }
   }


   // ====== Initialize CongestionManager client ============================
   if(manager) {
      cmClient = new CongestionManagerClient(manager);
      if(!cmClient->ready()) {
         delete cmClient;
         cmClient = NULL;
         cerr << "WARNING: AVSender::main() - Initialization of CongestionManagerClient failed!"
              << endl;
      }
      else {
         cmClient->start();
      }
   }


   // ====== Initialize =====================================================
   initAll(directory,local,premote,timeout,port);
   // installBreakDetector();


   // ====== Print status ===================================================
   cout << "RTP Audio/Prog4D Sender" << endl;
   cout << "-----------------------" << endl;
   cout << endl;
   cout << "RTP Audio:" << endl;
   if(local) {
      cout << "   Server local:    " << local << endl;
   }
   InternetAddress address;
   rtcpServerSocket->getSocketAddress(address);
   cout << "   Server address:  " << address << endl;
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",server->getOurSSRC());
   cout << "   Server SSRC:     " << str << endl;
   cout << "   Client Timeout:  " << (timeout / 1000000) << " [s]" << endl;
   cout << "   Input Directory: " << directory << endl;
   if(cmClient) {
      cout << "   Congestion Mgr.: " << manager << endl;
   }
   cout << "RTP Prog4D:" << endl;
   cout << "   Prog4D remote:   " << premote << endl;
   cout << endl;


   // ====== Main loop ======================================================
   for(;;) {
      // if(breakDetected())
      //   break;
      Thread::delay(10000000,true);
   }


   // ====== Clean up =======================================================
   cout << "Terminated!" << endl;
   cleanUp(0);
}
