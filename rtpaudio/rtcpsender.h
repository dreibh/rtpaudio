// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Sender                                                      ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2014 by Thomas Dreibholz            ####
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


#ifndef RTCPSENDER_H
#define RTCPSENDER_H


#include "tdsystem.h"
#include "tdsocket.h"
#include "timedthread.h"
#include "rtcppacket.h"
#include "rtpreceiver.h"
#include "randomizer.h"

#include <map>


/**
  * This class implements an RTCP sender based on TimedThread.
  *
  * @short   RTCP Sender
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RTCPSender : public TimedThread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Default constructor.
     * You have to initialize RTPSender by calling init(...) later!
     *
     * @see init
     */
   RTCPSender();

   /**
     * Constructor for new RTCPSender. The new sender's thread has to be started
     * by calling start()!
     *
     * @param flow Flow to remote side.
     * @param ssrc SSRC.
     * @param senderSocket Socket to write data to.
     * @param receiver RTPReceiver for reports to send.
     * @param bandwidth RTCP Bandwidth (see RFC 1889).
     * @param controlPPID PPID for SCTP transport.
     */
   RTCPSender(const InternetFlow& flow,
              const card32        ssrc,
              Socket*             senderSocket,
              RTPReceiver*        receiver,
              const card64        bandwidth,
              const card32        controlPPID);

   /**
     * Destructor.
     */
   ~RTCPSender();


   // ====== Initialize =====================================================
   /**
     * Initialize new RTCPSender. The new sender's thread has to be started
     * by calling start()!
     *
     * @param flow Flow to remote side.
     * @param ssrc SSRC.
     * @param senderSocket Socket to write data to.
     * @param receiver RTPReceiver for reports to send.
     * @param bandwidth RTCP Bandwidth (see RFC 1889).
     * @param controlPPID PPID for SCTP transport.
     */
   void init(const InternetFlow& flow,
             const card32        ssrc,
             Socket*             senderSocket,
             RTPReceiver*        receiver,
             const card64        bandwidth,
             const card32        controlPPID);


   // ====== RTCP packet sending functions ==================================
   /**
     * Send RTCP APP message.
     *
     * @param name RTCP APP name.
     * @param data RTCP APP data.
     * @param dataLength RTCP APP data length.
     * @return Bytes sent.
     */
   integer sendApp(const char* name, const void* data, const cardinal dataLength);

   /**
     * Send RTCP BYE message.
     *
     * @return Bytes sent.
     */
   integer sendBye();

   /**
     * Send RTCP receiver report from the SourceStateInfo given in the
     * constructor.
     *
     * @return Bytes sent.
     */
   integer sendReport();

   /**
     * Send RTCP SDES message from the list given by addSDESItem().
     *
     * @return Bytes sent.
     *
     * @see addSDESItem
     */
   integer sendSDES();

   /**
     * Add SDES item to SDES item list.
     * If a SDES item with the same type already exists in the list, the new
     * item replaces the old item.
     *
     * @param type SDES item type.
     * @param data SDES item data.
     * @param length SDES item data length.
     * @return true, if item has been added; false, if not.
     *
     * @see sendSDES
     */
   bool addSDESItem(const card8 type, const void* data, const card8 length = 0);

   /**
     * Remove SDES item from SDES item list.
     *
     * @param type SDES item type to be removed.
     *
     * @see addSDESItem
     * @see sendSDES
     */
   void removeSDESItem(const card8 type);


   // ====== Private data ===================================================
   private:
   void timerEvent();
   double computeTransmissionInterval();

   InternetFlow                                          Flow;
   SocketAddress*                                        ReceiverAddress;
   Socket*                                               SenderSocket;
   RTPReceiver*                                          Receiver;
   card32                                                SSRC;
   std::multimap<const card8,RTCPSourceDescriptionItem*> SDESItemSet;
   Randomizer                                            Random;
   card32                                                ControlPPID;

   bool    Initial;         // True, if application has not yet sent an RTCP packet
   bool    WeSent;          // True, if data sent since 2nd previous RTCP report
   integer Senders;         // Most current estimate for nr. of session senders
   integer Members;         // Most current estimate for nr. of session members
   double  RTCPBandwidth;   // Bandwidth for RTCP in octets/second
   double  AverageRTCPSize; // Average compound RTCP packet size
};


#endif
