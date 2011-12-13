// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Sender                                                       ####
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


#ifndef RTPSENDER_H
#define RTPSENDER_H


#include "tdsystem.h"
#include "timedthread.h"
#include "tdsocket.h"
#include "rtppacket.h"
#include "encoderinterface.h"
#include "trafficshaper.h"
#include "abstractqosdescription.h"
#include "qosmanagerinterface.h"


/**
  * This class implements an RTP sender based on TimedThread.
  *
  * @short   RTP Sender
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RTPSender : virtual public ManagedStreamInterface,
                  public TimedThread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Default constructor.
     * You have to initialize RTPSender by calling init(...) later!
     *
     * @see init
     */
   RTPSender();

   /**
     * Constructor for new RTPSender. The new sender's thread has to be started
     * by calling start()!
     *
     * @param flow Flow to remote side.
     * @param ssrc Sender's SSRC (see RFC 1889).
     * @param encoder Encoder to get packets to send from.
     * @param senderSocket Socket to write packets to.
     * @param controlPPID PPID for SCTP control transport.
     * @param dataPPID PPID for SCTP data transport.
     * @param maxPacketSize Maximum packet size.
     * @param qosManager QoS manager.
     *
     * @see Thread#start
     */
   RTPSender(const InternetFlow&  flow,
             const card32         ssrc,
             EncoderInterface*    encoder,
             Socket*              senderSocket,
             const card32         controlPPID,
             const card32         dataPPID,
             const cardinal       maxPacketSize = 1500,
             QoSManagerInterface* qosManager     = NULL);

   /**
     * Destructor.
     */
   ~RTPSender();


   // ====== Initialize =====================================================
   /**
     * Initialize new RTPSender. The new sender's thread has to be started
     * by calling start()!
     *
     * @param flow Flow to remote side.
     * @param ssrc Sender's SSRC (see RFC 1889).
     * @param encoder Encoder to get packets to send from.
     * @param senderSocket Socket to write packets to.
     * @param controlPPID PPID for SCTP control transport.
     * @param dataPPID PPID for SCTP data transport.
     * @param maxPacketSize Maximum packet size.
     * @param qosManager QoS manager.
     *
     * @see Thread#start
     */
   void init(const InternetFlow&  flow,
             const card32         ssrc,
             EncoderInterface*    encoder,
             Socket*              senderSocket,
             const card32         controlPPID,
             const card32         dataPPID,
             const cardinal       maxPacketSize = 1500,
             QoSManagerInterface* qosManager    = NULL);



   // ====== Quality control ================================================
   /**
     * Implementation of ManagedStreamInterface's getQoSDescription().
     *
     * @see ManagedStreamInterface#getQoSDescription
     */
   AbstractQoSDescription* getQoSDescription(const card64 offset);

   /**
     * Implementation of ManagedStreamInterface's updateQuality().
     *
     * @see ManagedStreamInterface#updateQuality
     */
   void updateQuality(const AbstractQoSDescription* aqd);

   /**
     * Implementation of ManagedStreamInterface's lock().
     *
     * @see ManagedStreamInterface#lock
     */
   void lock();

   /**
     * Implementation of ManagedStreamInterface's unlock().
     *
     * @see ManagedStreamInterface#unlock
     */
   void unlock();


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


   // ====== Transmission control ===========================================
   /**
     * Check, if transmission is paused.
     *
     * @return true, if paused; false otherwise.
     */
   inline bool paused() const;


   /**
     * Check, if transmission error has been detected (e.g. destination rejects
     * packets, no route etc.).
     * Note: The transmission error attribute will be resetted to false by
     * calling this method.
     */
   inline bool transmissionErrorDetected();


   /**
     * Set pause on or off.
     *
     * @param on true to set pause on; false to set pause off.
     */
   inline void setPause(const bool on);


   /**
     * Get number of bytes sent.
     *
     * @return Bytes sent.
     */
   inline card64 getBytesSent() const;

   /**
     * Get number of packets sent.
     *
     * @return Packets sent.
     */
   inline card64 getPacketsSent() const;

   /**
     * Reset number of bytes sent.
     */
   inline void resetBytesSent();

   /**
     * Reset number of packets sent.
     */
   inline void resetPacketsSent();


   // ====== Private data ===================================================
   private:
   void timerEvent();
   void updateFrameRate(const AbstractQoSDescription* aqd);


   private:
   EncoderInterface*    Encoder;
   Socket*              SenderSocket;

   cardinal             FramesPerSecond;
   cardinal             RenewCounter;
   cardinal             MaxPacketSize;
   card32               SSRC;
   card64               BytesSent;
   card64               PacketsSent;
   card64               TimeStamp;
   card32               ControlPPID;
   card32               DataPPID;

   card32               PayloadBytesSent;
   card32               PayloadPacketsSent;

   bool                 Pause;
   bool                 TransmissionError;

   InternetFlow         Flow[RTPConstants::RTPMaxQualityLayers];
   card16               SequenceNumber[RTPConstants::RTPMaxQualityLayers];

   QoSManagerInterface* QoSMgr;
   cardinal             Bandwidth[RTPConstants::RTPMaxQualityLayers];
   double               BufferDelay[RTPConstants::RTPMaxQualityLayers];

#ifdef USE_TRAFFICSHAPER
   TrafficShaper        SenderReportBuffer;
   TrafficShaper        Shaper[RTPConstants::RTPMaxQualityLayers];
#endif
};


#include "rtpsender.icc"


#endif
