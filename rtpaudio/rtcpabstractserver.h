// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Abstract Server Implementation                              ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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


#ifndef RTCPABSTRACTSERVER_H
#define RTCPABSTRACTSERVER_H


#include "tdsystem.h"
#include "timedthread.h"
#include "rtcppacket.h"
#include "internetflow.h"
#include "sourcestateinfo.h"

#include <map>


/**
  * This class is an abstract RTCP server.
  *
  * @short   RTCP abstract server
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RTCPAbstractServer : public TimedThread
{
   // ====== Definitions ====================================================
   public:
   /**
     * Client structure with information on the client.
     */
   struct Client {
      card32       SSRC;
      InternetFlow ClientAddress;
      card64       TimeStamp;
      card64       Timeout;
      void*        UserData;
   };


   // ====== Constructor/Destructor =========================================
   /**
     * Constructor.
     */
   RTCPAbstractServer();

   /**
     * Destructor.
     */
   ~RTCPAbstractServer();


   // ====== Virtual functions ==============================================
   /**
     * Called when a new client sends its SDES CNAME message.
     * The class inheriting RTCPAbstractServer may use the client->UserData
     * field to store additional data to serve the client. The result of
     * the call will be saved into this field (client->UserData = newClient(client))!
     * The call is synchronized by RTCPAbstractServer.
     *
     * @param client Client.
     * @param cname CNAME string.
     * @return A value, which RTCPAbstractServer will save to client->UserData.
     */
   virtual void* newClient(Client* client, const char* cname) = 0;


   /**
     * A set of reasons for deleteClient() call.
     */
   enum DeleteReason {
      DeleteReason_UserBye  = 0,
      DeleteReason_Timeout  = 1,
      DeleteReason_Shutdown = 2,
      DeleteReason_Error    = 3,
   };

   /**
     * Called when a client sends RTCP BYE or the timeout is reached.
     * The call is synchronized by RTCPAbstractServer.
     *
     * @param client Client.
     * @param reason Reason for deleteClient() call.
     * @param hasTimeout true, if timeout is reached; false, if RTCP BYE received.
     * @param shutdown true, if server shutdown is in progress.
     */
   virtual void deleteClient(Client*            client,
                             const DeleteReason reason) = 0;

   /**
     * This method is called about once per second to check, if the client
     * is okay (e.g. no transmission error has occurred etc.)
     * The call is synchronized by RTCPAbstractServer.
     *
     * @return true, if client is okay; false to delete client in case of an error.
     */
   virtual bool checkClient(Client* client) = 0;

   /**
     * Called when a client sends RTCP APP message.
     * The call is synchronized by RTCPAbstractServer.
     *
     * @param client Client.
     * @param name RTCP APP name.
     * @param data RTCP APP data.
     * @param dataLength RTCP APP data length.
     */
   virtual void appMessage(Client*        client,
                           const char*    name,
                           void*          data,
                           const cardinal dataLength) = 0;

   /**
     * Called when a client sends RTCP SDES message; it is called for every
     * SDES item in the message.
     * The call is synchronized by RTCPAbstractServer.
     *
     * @param client Client.
     * @param type RTCP SDES type.
     * @param data RTCP SDES data.
     * @param length RTCP SDES length.
     */
   virtual void sdesMessage(Client*        client,
                            const card8    type,
                            char*          data,
                            const cardinal length) = 0;

   /**
     * Called when a client sends a receiver report; it is called for every
     * receiver report block in the message.
     * The call is synchronized by RTCPAbstractServer.
     *
     * @param client Client.
     * @param report RTCPReceptionReportBlock.
     * @param layer Layer number.
     */
   virtual void receiverReport(Client*                   client,
                               RTCPReceptionReportBlock* report,
                               const cardinal            layer) = 0;


   /**
     * This method is called, if an out of memory error occurs. It prints
     * a simple error message. It should be overloaded by a more useful
     * method within the concrete server.
     * The call is synchronized by RTCPAbstractServer.
     */
   virtual void outOfMemoryWarning();


   /**
     * Get number of members serverd by the server.
     *
     * @return Number of members.
     */
   inline cardinal getMembers();


   // ====== Settings =======================================================
   /**
     * Get the default timeout in microseconds, after which a client is
     * assumed to be dead and removed.
     *
     * @return Default timeout in microseconds.
     */
   inline card64 getDefaultTimeout() const;

   /**
     * Set the default timeout in microseconds, after which a client is
     * assumed to be dead and removed. The new value will be used for all
     * new clients. Timeouts of old clients are not changed!
     *
     * @param timeout Default timeout in microseconds.
     */
   inline void setDefaultTimeout(const card64 timeout);


   // ====== stop() reimplementation ========================================
   /**
     * Reimplementation of Thread::stop() to remove all clients before
     * stopping.
     *
     * @see Thread#stop
     */
   void* stop();


   // ====== RTCP packet handlers ===========================================
   private:
   /**
     * RTCPReceiver is friend class to enable usage of receivedXXX() methods.
     */
   friend class RTCPReceiver;

   void receivedSenderReport(const InternetFlow        flow,
                             const card32              source,
                             RTCPReceptionReportBlock* report,
                             const cardinal            layer);
   void receivedReceiverReport(const InternetFlow        flow,
                               const card32              source,
                               RTCPReceptionReportBlock* report,
                               const cardinal            layer);
   void receivedSourceDescription(const InternetFlow flow,
                                  const card32       source,
                                  const card8        type,
                                  char*              data,
                                  const card8        length);
   void receivedApp(const InternetFlow flow,
                    const card32       source,
                    const char*        name,
                    void*              data,
                    const card32       dataLength);
   void receivedBye(const InternetFlow flow,
                    const card32       source,
                    const DeleteReason reason);

   Client* findClient(const card32 source, const InternetFlow flow);


   // ====== Private data ===================================================
   private:
   void timerEvent();


   card64                                DefaultTimeout;
   std::multimap<const cardinal,Client*> ClientSet;
};


#include "rtcpabstractserver.icc"


#endif
