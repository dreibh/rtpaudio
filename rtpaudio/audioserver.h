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


#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H


#include "tdsystem.h"
#include "multiaudioreader.h"
#include "tdsocket.h"
#include "audioencoderrepository.h"
#include "rtpsender.h"
#include "rtcppacket.h"
#include "rtcpreceiver.h"
#include "rtcpabstractserver.h"
#include "multiaudioreader.h"
#include "bandwidthmanager.h"

#include "audioclientapppacket.h"

#include <map>


/**
  * This class is an audio server based on RTCPAbstractServer
  *
  * @short   Audio Server
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class AudioServer : public RTCPAbstractServer
{
   // ====== Definitions ====================================================
   private:
   struct User {
      RTCPAbstractServer::Client* Client;
      RTPSender                   Sender;
      Socket                      SenderSocket;
      InternetFlow                Flow;
      AudioEncoderRepository      Repository;
      MultiAudioReader            Reader;
      String                      MediaName;
      integer                     StreamIdentifier;
      card64                      BandwidthLimit;
      card16                      LastSequenceNumber;
      card16                      PosChgSeqNumber;
      bool                        UserLimitPause;
      bool                        ManagerLimitPause;
      bool                        ClientPause;
   };


   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor for new AudioServer.
     *
     * @param maxPacketSize Maximum packet size.
     * @param qosManager QoS manager.
     * @param useSCTP true to use SCTP instead of UDP; false otherwise.
     */
   AudioServer(BandwidthManager* qosManager    = NULL,
               const cardinal    maxPacketSize = 1500,
               const bool        useSCTP       = false);

   /**
     * Destructor.
     */
   ~AudioServer();


   // ====== Status functions ===============================================
   /**
     * Get client SSRC.
     *
     * @return Client SSRC.
     */
   card32 getOurSSRC() const;

   /**
     * Get loss scalibility setting.
     *
     * @return true, if loss scalability is on; false otherwise.
     */
   inline bool getLossScalability() const;

   /**
     * Set loss scalibility setting.
     *
     * @param on true, if to set loss scalability on; false otherwise.
     */
   inline void setLossScalability(const bool on);


   // ====== Packet size ====================================================
   /**
     * Get maximum packet size.
     *
     * @return Maximum packet size.
     */
   inline cardinal getMaxPacketSize() const;

   /**
     * Set maximum packet size.
     *
     * @param size Maximum packet size.
     * @return Maximum packet size set.
     */
   inline cardinal setMaxPacketSize(const cardinal size);


   // ====== RTCPAbstractServer implementation ==============================
   /**
     * outOfMemoryWarning() implementation of RTCPAbstractServer.
     */
   void outOfMemoryWarning();

   /**
     * newClient() implementation of RTCPAbstractServer.
     *
     * @see RTCPAbstractServer#newClient
     */
   void* newClient(RTCPAbstractServer::Client* client, const char* cname);

   /**
     * deleteClient() implementation of RTCPAbstractServer.
     *
     * @see RTCPAbstractServer#deleteClient
     */
   void deleteClient(RTCPAbstractServer::Client* client, const DeleteReason reason);

   /**
     * checkClient() implementation of RTCPAbstractServer.
     *
     * @see RTCPAbstractServer#checkClient
     */
   bool checkClient(const RTCPAbstractServer::Client* client);

   /**
     * appMessage() implementation of RTCPAbstractServer.
     *
     * @see RTCPAbstractServer#appMessage
     */
   void appMessage(const RTCPAbstractServer::Client* client,
                   const char*                       name,
                   const void*                       data,
                   const cardinal                    dataLength);

   /**
     * sdesMessage() implementation of RTCPAbstractServer.
     *
     * @see RTCPAbstractServer#sdesMessage
     */
   void sdesMessage(const RTCPAbstractServer::Client* client,
                    const card8                       type,
                    const char*                       data,
                    const cardinal                    length);

   /**
     * receiverReport() implementation of RTCPAbstractServer.
     *
     * @see RTCPAbstractServer#receiverReport
     */
   void receiverReport(const RTCPAbstractServer::Client* client,
                       const RTCPReceptionReportBlock*   report,
                       const cardinal                    layer);


   // ====== Execute user command ===========================================
   /**
     * Execute commands given in AudioClientAppPacket.
     *
     * @param client Client.
     * @param user User.
     * @param app AudioClientApp message.
     */
   void userCommand(const RTCPAbstractServer::Client* client,
                    User*                             user,
                    const AudioClientAppPacket*       app);

   /**
     * Update QoS/congestion management.
     *
     * @param client Client to do congestion for.
     * @param user User data.
     */
   void managementUpdate(const RTCPAbstractServer::Client* client, User* user);


   // ====== Private data ===================================================
   private:
   BandwidthManager*                   QoSMgr;
   std::multimap<const cardinal,User*> UserSet;
   Synchronizable                      UserSetSync;
   cardinal                            MaxPacketSize;
   card32                              OurSSRC;
   bool                                LossScalability;
   bool                                UseSCTP;
};


#include "audioserver.icc"


#endif
