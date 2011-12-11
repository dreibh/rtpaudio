// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Receiver                                                     ####
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


#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H


#include "tdsystem.h"
#include "thread.h"
#include "tdsocket.h"
#include "rtppacket.h"
#include "decoderinterface.h"
#include "sourcestateinfo.h"
#include "internetflow.h"


/**
  * This class implements an RTP receiver based on Thread.
  *
  * @short   RTP Receiver
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RTPReceiver : public Thread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Default constructor.
     * You have to initialize RTPReceiver by calling init(...) later!
     *
     * @see init
     */
   RTPReceiver();

   /**
     * Constructor for new RTPSender. The new sender's thread has to be started
     * by calling start()!
     *
     * @param decoder Decoder to handle packets received.
     * @param receiverSocket Socket to receive data from.
     *
     * @see Thread#start
     */
   RTPReceiver(DecoderInterface* decoder,
               Socket*           receiverSocket);

   /**
     * Destructor.
     */
   ~RTPReceiver();


   // ====== Initialize =====================================================
   /**
     * Initialize RTPSender. The new receiver's thread has to be started
     * by calling start()!
     *
     * @param decoder Decoder to handle packets received.
     * @param receiverSocket Socket to receive data from.
     *
     * @see Thread#start
     */
   void init(DecoderInterface* decoder,
             Socket*           receiverSocket);


   // ====== Status functions ===============================================
   /**
     * Get position of the encoder.
     * The access is synchronized with the receiver thread.
     *
     * @return Position in nanoseconds.
     */
   inline card64 getPosition() const;

   /**
     * Get maximum position of the encoder.
     * The access is synchronized with the receiver thread.
     *
     * @return Maximum position in nanoseconds.
     */
   inline card64 getMaxPosition() const;

   /**
     * Get number of bytes received.
     *
     * @param layer Layer number or (cardinal)-1 to get sum of all layers.
     * @return Bytes received.
     */
   inline card64 getBytesReceived(const cardinal layer) const;

   /**
     * Get number of packets received.
     *
     * @param layer Layer number or (cardinal)-1 to get sum of all layers.
     * @return Packets received.
     */
   inline card64 getPacketsReceived(const cardinal layer) const;

   /**
     * Reset number of bytes received.
     *
     * @param layer Layer number.
     */
   inline void resetBytesReceived(const cardinal layer);

   /**
     * Reset number of packets received.
     *
     * @param layer Layer number.
     */
   inline void resetPacketsReceived(const cardinal layer);

   /**
     * Get number of layers of last transmission.
     *
     * @return Number of layers.
     */
   inline cardinal getLayers() const;

   /**
     * Get InternetFlow  of last transmission in a given layer.
     *
     * @param layer Layer number.
     * @return InternetFlow.
     */
   inline InternetFlow getInternetFlow(const cardinal layer = 0) const;

   /**
     * Get SourceStateInfo for given layer.
     */
   inline SourceStateInfo getSSI(const cardinal layer = 0) const;


   /**
     * RTCPSender is a friend class to enable efficient update of
     * SSI data.
     */
   friend class RTCPSender;


   // ====== Protected data =================================================
   protected:
   cardinal        Layers;
   InternetFlow    Flow[RTPConstants::RTPMaxQualityLayers];
   SourceStateInfo SSI[RTPConstants::RTPMaxQualityLayers];
   card64          BytesReceived[RTPConstants::RTPMaxQualityLayers];
   card64          PacketsReceived[RTPConstants::RTPMaxQualityLayers];


   // ====== Private data ===================================================
   private:
   void run();


   DecoderInterface* Decoder;
   Socket*           ReceiverSocket;
};


#include "rtpreceiver.icc"


#endif
