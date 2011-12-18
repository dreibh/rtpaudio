// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Server Implementation                                      ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   http://www.iem.uni-due.de/~dreibh/rtpaudio             ####
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
#include "multiaudioreader.h"
#include "tdsocket.h"
#include "simpleaudioencoder.h"
#include "advancedaudioencoder.h"
#include "audioencoderrepository.h"
#include "rtpsender.h"
#include "rtcppacket.h"
#include "rtcpreceiver.h"
#include "rtcpabstractserver.h"
#include "sourcestateinfo.h"
#include "tools.h"
#include "randomizer.h"
#include "audioclientapppacket.h"
#include "audioserver.h"


// Verbose mode: Display some information on connections.
#define VERBOSE

// Debug mode: Print debug information
// #define QOSMGR_DEBUG


// ###### Constructor #######################################################
AudioServer::AudioServer(QoSManagerInterface* qosManager,
                         const cardinal       maxPacketSize,
                         const bool           useSCTP)
{
   Randomizer random;
   OurSSRC = random.random32();
   QoSMgr  = qosManager;
   UseSCTP = useSCTP;
   setMaxPacketSize(maxPacketSize);
   setLossScalability(true);
}


// ###### Destructor ########################################################
AudioServer::~AudioServer()
{
   stop();
}


// ###### Out of memory warning #############################################
void AudioServer::outOfMemoryWarning()
{
   printTimeStamp();
   std::cerr << "*** Out of memory! ***" << std::endl;
}


// ###### Send update to congestion manager #################################
void AudioServer::managementUpdate(Client* client, User* user)
{
}


// ###### Add a new client ##################################################
void* AudioServer::newClient(Client* client, const char* cname)
{
   // ====== Create new user ================================================
   User* user = new User;
   if(user == NULL) {
      outOfMemoryWarning();
      deleteClient(client,DeleteReason_Error);
      return(NULL);
   }
   user->Client           = client;
   user->StreamIdentifier = 0;
   client->UserData       = user;

   // ====== Create and connect sender socket ===============================
   user->SenderSocket.create(Socket::IP,
                             UseSCTP ? Socket::SeqPacket : Socket::Datagram,
                             UseSCTP ? Socket::SCTP      : Socket::Default);
   if(!user->SenderSocket.ready()) {
      std::cerr << "WARNING: AudioServer::newClient() - Unable to create socket!" << std::endl;
      deleteClient(client,DeleteReason_Error);
      return(NULL);
   }
   user->SenderSocket.setBlockingMode(false);
   user->Flow = user->SenderSocket.allocFlow(client->ClientAddress);
   if(user->Flow.getFlowLabel() == 0) {
      user->Flow = InternetFlow(client->ClientAddress,0,0);
   }
   user->Flow.setTrafficClass(AudioServerDefaultTrafficClass);

   // ====== Create repository and encoders =================================
   user->LastSequenceNumber = 0xffff;
   user->PosChgSeqNumber    = 0xffff;
   user->BandwidthLimit     = (card64)-1;
   user->UserLimitPause     = false;
   user->ManagerLimitPause  = false;
   user->ClientPause        = false;
   user->Repository.setAutoDelete(true);
   const bool e1 = user->Repository.addEncoder(new AdvancedAudioEncoder(&user->Reader));
   const bool e2 = user->Repository.addEncoder(new SimpleAudioEncoder(&user->Reader));
   if((e1 == false) || (e2 == false)) {
      outOfMemoryWarning();
      deleteClient(client,DeleteReason_Error);
      return(NULL);
   }

   // ====== Initialize RTPSender ===========================================
   user->Sender.init(user->Flow,OurSSRC,
                     &user->Repository,&user->SenderSocket,
                     RTPAudioControlPPID, RTPAudioDataPPID,
                     MaxPacketSize,QoSMgr);

   // ====== Add stream to QoS management ===================================
   InternetAddress ourAddress;
   user->SenderSocket.getSocketAddress(ourAddress);
   UserSetSync.synchronized();
   user->StreamIdentifier = (integer)((long)user);
   UserSet.insert(std::pair<const cardinal,User*>(user->StreamIdentifier,user));
   UserSetSync.unsynchronized();

   // ====== Start RTPSender ================================================
   if(user->Sender.start() == false) {
      deleteClient(client,DeleteReason_Error);
      return(NULL);
   }

   // ====== Print information ==============================================
#ifdef VERBOSE
   printTimeStamp();
   char str[300];
   snprintf((char*)&str,sizeof(str),"New member $%08x added.",client->SSRC);
   std::cout << str << std::endl;
   InternetAddress sourceAddress;
   user->SenderSocket.getSocketAddress(sourceAddress);
   sourceAddress.setPrintFormat(InternetAddress::PF_Address);
   std::cout << "   CNAME:               " << cname << std::endl
             << "   Source Address:      " << sourceAddress << std::endl
             << "   Destination Address: " << (InternetAddress)client->ClientAddress << std::endl;
   if(user->SenderSocket.getSendFlowLabel() != 0) {
      snprintf((char*)&str,sizeof(str),"$%05x",user->SenderSocket.getSendFlowLabel());
      std::cout << "   Flow Label:          " << str << std::endl;
   }
   snprintf((char*)&str,sizeof(str),"$%02x",user->SenderSocket.getSendTrafficClass());
   std::cout << "   Traffic Class:       " << str << std::endl
             << "   => We have " << getMembers() + 1 << " member(s) now!" << std::endl;
#endif
   return((void*)user);
}


// ###### Delete a client ###################################################
void AudioServer::deleteClient(Client* client, const DeleteReason reason)
{
   User* user = (User*)client->UserData;

   // ====== Remove user ====================================================
   if(user != NULL) {
      if(user->StreamIdentifier != 0) {
         UserSetSync.synchronized();
         std::multimap<const cardinal,User*>::iterator found =
            UserSet.find(user->StreamIdentifier);
         UserSet.erase(found);
         UserSetSync.unsynchronized();
      }
      if(QoSMgr != NULL) {
         QoSMgr->removeStream(&user->Sender);
      }
      user->Sender.stop();
      if(user->Flow.getFlowLabel() != 0) {
         user->SenderSocket.freeFlow(user->Flow);
      }
      delete user;
   }

   client->UserData = NULL;

   // ====== Print information ==============================================
#ifdef VERBOSE
   char str[128];
   printTimeStamp();
   snprintf((char*)&str,sizeof(str),"$%08x",client->SSRC);

   std::cout << str << " removed";
   switch(reason) {
      case DeleteReason_Timeout:
         std::cout << " due to timeout!" << std::endl;
       break;
      case DeleteReason_Shutdown:
         std::cout << " due to server shutdown!" << std::endl;
        break;
      case DeleteReason_Error:
         std::cout << " due to transmission error!" << std::endl;
        break;
      default:
         std::cout << "." << std::endl;
        break;
   }
   if(reason != DeleteReason_Shutdown) {
      std::cout << "   => We have " << getMembers() - 1 << " member(s) now!" << std::endl;
   }
#endif
}


// ###### Handle user command ###############################################
void AudioServer::userCommand(Client*               client,
                              User*                 user,
                              AudioClientAppPacket* app)
{
   // ====== Check, if command has already been executed ====================
   const integer diff = (integer)app->SequenceNumber -
                           (integer)user->LastSequenceNumber;
   if(!((diff > 0) || (diff < -30000))) {
      return;
   }

   user->LastSequenceNumber = app->SequenceNumber;
   user->Sender.synchronized();

   // ====== Set quality, position, media name and bandwidth limit ==========
   if(user->Repository.selectEncoderForTypeID(app->Encoding) == true) {
      user->Repository.setSamplingRate(app->SamplingRate);
      user->Repository.setBits(app->Bits);
      user->Repository.setChannels(app->Channels);

      // ====== Select new media and set frame number =======================
      if(user->Reader.getErrorCode() == ME_NoMedia) {
#ifdef VERBOSE
         char str[128];
         printTimeStamp();
         snprintf((char*)&str,sizeof(str),"$%08x",client->SSRC);
         std::cout << str << " loading media <" << (const char*)&app->MediaName
                   << ">." << std::endl;
#endif
         if(user->Reader.openMedia((const char*)&app->MediaName)) {
            user->Reader.setPosition(app->RestartPosition);
            user->PosChgSeqNumber = app->PosChgSeqNumber;
            user->MediaName       = String((const char*)&app->MediaName);
            user->Sender.leaveCorrectionLoop();
            if(QoSMgr != NULL) {
               QoSMgr->addStream(&user->Sender,0,(const char*)&app->MediaName);
            }
         }
      }

      else if((user->MediaName != "") && (user->MediaName != (char*)&app->MediaName)) {
#ifdef VERBOSE
         char str[128];
         printTimeStamp();
         snprintf((char*)&str,sizeof(str),"$%08x",client->SSRC);
         std::cout << str << " changing media to <" << (const char*)&app->MediaName
              << ">." << std::endl;
#endif
         if(QoSMgr != NULL) {
            QoSMgr->removeStream(&user->Sender);
         }
         user->Reader.closeMedia();
         if(user->Reader.openMedia((const char*)&app->MediaName)) {
            user->Reader.setPosition(app->RestartPosition);
            user->MediaName = String((const char*)&app->MediaName);
            user->Sender.leaveCorrectionLoop();
            if(QoSMgr != NULL) {
               QoSMgr->addStream(&user->Sender,0,(const char*)&app->MediaName);
            }
         }
      }

      // ====== Change position, if it has been updated =====================
      const integer diff = (integer)app->PosChgSeqNumber -
                              (integer)user->PosChgSeqNumber;
      if((diff > 0) || (diff < -30000)) {
         user->PosChgSeqNumber = app->PosChgSeqNumber;
         if(app->StartPosition != (card64)-1) {
            user->Reader.setPosition(app->StartPosition);
            user->Sender.leaveCorrectionLoop();
         }
      }

      // ====== Update client's bandwidth limit =============================
      user->BandwidthLimit = (card64)app->BandwidthLimit;
   }
   else {
      char str[32];
      snprintf((char*)&str,sizeof(str),"$%04x",app->Encoding);
      std::cerr << "WARNING: AudioServer::userCommand() - Unsupported Encoding #"
           << str << " requested!" << std::endl;
   }


   // ====== Execute command ================================================
   switch(app->Status) {
      case AudioClientAppPacket::ACAS_Pause:
        user->ClientPause = true;
        user->Sender.setPause(true);
       break;
      case AudioClientAppPacket::ACAS_Play:
         {
            // NOTE: getQoSDescription() will set frame rate!
            const AbstractQoSDescription* qosDescription =
               user->Sender.getQoSDescription(0);

            if(qosDescription != NULL) {
               if(user->BandwidthLimit >= qosDescription->getMinWantedBandwidth()) {
                  if(user->UserLimitPause == true) {
                     user->UserLimitPause = false;
                     if(user->ManagerLimitPause == false) {
#ifdef VERBOSE
                        printTimeStamp();
                        char str[128];
                        snprintf((char*)&str,sizeof(str),"$%08x resumed due to increased user bandwidth!",client->SSRC);
                        std::cout << str << std::endl;
#endif
                        user->Sender.setPause(false);
                     }
                  }
               }
               else {
                  if(user->UserLimitPause == false) {
                     user->UserLimitPause = true;
                     if(user->ManagerLimitPause == false) {
#ifdef VERBOSE
                        printTimeStamp();
                        char str[128];
                        snprintf((char*)&str,sizeof(str),"$%08x paused due to user bandwidth limit!",client->SSRC);
                        std::cout << str << std::endl;
#endif
                        user->Sender.setPause(true);
                     }
                  }
               }
            }

            if((user->ClientPause == true) &&
               (user->UserLimitPause == false) &&
               (user->ManagerLimitPause == false)) {
               user->ClientPause = false;
               user->Sender.setPause(false);
            }
         }
        break;
   }

   user->Sender.unsynchronized();
}


// ###### Handle app message ################################################
void AudioServer::appMessage(Client*        client,
                             const char*    name,
                             void*          data,
                             const cardinal dataLength)
{
   User* user = (User*)client->UserData;
   AudioClientAppPacket* app = (AudioClientAppPacket*)data;
   app->translate();
   if(app->FormatID == AudioClientAppPacket::AudioClientFormatID) {
      userCommand(client,user,app);
      managementUpdate(client,user);
   }
}


// ###### Handle SDES message ###############################################
void AudioServer::sdesMessage(Client*        client,
                              const card8    type,
                              char*          data,
                              const cardinal length)
{
   User* user = (User*)client->UserData;
   if(type == RTCP_SDES_PRIV) {
      AudioClientSDESPrivPacket* priv = (AudioClientSDESPrivPacket*)data;
      if( (priv->PrefixLength == 7) &&
          (priv->Prefix[0] == 'C') &&
          (priv->Prefix[1] == 'o') &&
          (priv->Prefix[2] == 'o') &&
          (priv->Prefix[3] == 'k') &&
          (priv->Prefix[4] == 'i') &&
          (priv->Prefix[5] == 'e') &&
          (priv->Prefix[6] == '0') ) {
         AudioClientAppPacket* app  = &priv->Status;
         app->translate();
         if(app->FormatID == AudioClientAppPacket::AudioClientFormatID) {
            userCommand(client,user,app);
            managementUpdate(client,user);
         }
      }
      else {
         std::cerr << "NOTE: AudioServer::sdesMessage() - Unknown prefix" << std::endl;
      }
   }
/*
   else {
      std::cerr << "NOTE: AudioServer::sdesMessage() - Unsupported SDES type: "
                << (cardinal)type << std::endl;
   }
*/
}


// ###### Check, if client is okay ##########################################
bool AudioServer::checkClient(Client* client)
{
   User* user = (User*)client->UserData;

   return(user->Sender.transmissionErrorDetected() == false);
}


// ###### Handle receiver report ############################################
void AudioServer::receiverReport(Client*                   client,
                                 RTCPReceptionReportBlock* report,
                                 const cardinal            layer)
{
   User* user = (User*)client->UserData;

   if(LossScalability == true) {
      if(QoSMgr != NULL) {
#ifdef QOSMGR_DEBUG
         cout << "Loss rate in layer #" << layer << ": " << report->getFractionLost() << endl;
#endif
         QoSMgr->reportEvent(&user->Sender,report,layer);
      }
   }
}
