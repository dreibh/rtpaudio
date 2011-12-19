// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Receiver Implementation                                      ####
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
#include "rtpreceiver.h"
#include "rtcppacket.h"
#include "internetflow.h"
#include "randomizer.h"


// Debug mode: Print corrupted RTP packets
#define DEBUG


// ###### Constructor #######################################################
RTPReceiver::RTPReceiver()
   : Thread("RTPReceiver")
{
   Decoder        = NULL;
   ReceiverSocket = NULL;
}


// ###### Constructor #######################################################
RTPReceiver::RTPReceiver(DecoderInterface* decoder,
                         Socket*           receiverSocket)
   : Thread("RTPReceiver")
{
   init(decoder,receiverSocket);
}


// ###### Initialize ########################################################
void RTPReceiver::init(DecoderInterface* decoder,
                       Socket*           receiverSocket)
{
   Decoder         = decoder;
   ReceiverSocket  = receiverSocket;
   Layers          = 0;
   for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
      Flow[i].reset();
      SSI[i].reset();
      BytesReceived[i]   = 0;
      PacketsReceived[i] = 0;
   }
}


// ###### Destructor ########################################################
RTPReceiver::~RTPReceiver()
{
   stop();
}


// ###### The Process's run method: Receive and decode packets ##############
void RTPReceiver::run()
{
   if(Decoder == NULL) {
      std::cerr << "ERROR: RTPReceiver::run() - RTPReceiver is uninitialized!"
                << std::endl;
      return;
   }

   char          packetData[8192];
   InternetFlow flow;
   for(;;) {
      // ====== Read RTCP packet ============================================
      cardinal receivedPacketSize = 0;
      integer  flags;
      integer  received;
      do {
         flags = 0;
         received = ReceiverSocket->receiveFrom(
                       (char*)&packetData[receivedPacketSize],
                       sizeof(packetData) - receivedPacketSize,
                       flow,flags);
         if(received > 0) {
            receivedPacketSize += (cardinal)received;
            if( (ReceiverSocket->getProtocol() != Socket::SCTP) ||
                (flags & MSG_EOR) ) {
               break;
            }
         }
      } while(received >= 0);

      // ==== Packet loss simulation ========================================
/*
      Randomizer r;
      cardinal i = r.random32() % 20;
      if(i == 1) bytes = 0;
*/

      // ====== Process packet, if read was successful ======================
      if(receivedPacketSize > 0) {
         RTPPacket* packet = (RTPPacket*)&packetData;

         // ====== Verify RTP packet ========================================
         if(receivedPacketSize < RTPConstants::RTPDefaultHeaderSize) {
            std::cerr << "WARNING: RTPReceiver::run() - Received too small RTP header" << std::endl;
            continue;
         }
         if(packet->getVersion() != RTPConstants::RTPVersion) {
            std::cerr << "WARNING: RTPReceiver::run() - Invalid version " << packet->getVersion() << std::endl;
            continue;
         }
         const integer payloadLength = (integer)receivedPacketSize - (integer)packet->calculateHeaderSize();
         if(payloadLength < 0) {
            std::cerr << "WARNING: RTCPReceiver::run() - Invalid payload length" << std::endl;
            continue;
         }

         // ====== Check, if decoder accepts packet =========================
         synchronized();

         DecoderPacket decoderPacket;
         decoderPacket.Buffer         = packet->getPayloadData();
         decoderPacket.Length         = payloadLength;
         decoderPacket.SequenceNumber = packet->getSequenceNumber();
         decoderPacket.TimeStamp      = packet->getTimeStamp();
         decoderPacket.SSIArray       = (SourceStateInfo**)&SSI;
         decoderPacket.Marker         = packet->getMarker();
         decoderPacket.PayloadType    = packet->getPayloadType();
         decoderPacket.Layer          = (cardinal)-1;
         decoderPacket.Layers         = (cardinal)-1;

         // ====== Paket ist RTP-Paket fr den Decoder =======================
         if(Decoder->checkNextPacket(&decoderPacket) == true) {
            // Check, if packet's layer number is valid. ==
            if(decoderPacket.Layers <= RTPConstants::RTPMaxQualityLayers) {
               if(decoderPacket.Layer < decoderPacket.Layers) {
                  // Update SSI and check, if packet's sequence number is valid.
                  SSI[decoderPacket.Layer].synchronized();
                  SSI[decoderPacket.Layer].setSSRC(packet->getSSRC());
                  SeqNumValidator::ValidationResult valid =
                     SSI[decoderPacket.Layer].validate(packet->getSequenceNumber(),packet->getTimeStamp());
                  SSI[decoderPacket.Layer].unsynchronized();

                  // ====== Decoder packet ==================================
                  if(valid < SeqNumValidator::Invalid) {
                     Decoder->handleNextPacket(&decoderPacket);

                     // ====== Update variables =============================
                     Flow[decoderPacket.Layer] = flow;
                     if(Flow[decoderPacket.Layer].getTrafficClass() == 0x00) {
                        Flow[decoderPacket.Layer].setTrafficClass(ReceiverSocket->getReceivedTrafficClass());
                     }
                     Layers = decoderPacket.Layers;
                     for(cardinal i = decoderPacket.Layers;i < RTPConstants::RTPMaxQualityLayers;i++) {
                        Flow[i].setTrafficClass(0);
                     }
                     BytesReceived[decoderPacket.Layer] += receivedPacketSize;
                     PacketsReceived[decoderPacket.Layer]++;
                  }
               }
               else {
                  std::cerr << "WARNING: RTPReceiver::run() - decoderPacket.Layer >= decoderPacket.Layers: "
                       << decoderPacket.Layer << " >= " << decoderPacket.Layers
                       << "!" << std::endl;
               }
            }
         }

         // ====== Paket ist RTCP Sender Report =============================
         else {
            const RTCPSenderReport* report = (const RTCPSenderReport*)&packet;
            if((receivedPacketSize >= (ssize_t)sizeof(RTCPSenderReport)) &&
               (report->getPacketType() == RTCP_SR)) {
               for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
                  SSI[i].synchronized();
                  if(SSI[i].getSSRC() == report->getSSRC()) {
                     SSI[i].setLSR((card32)((report->getNTPTimeStamp() >> 16) & 0xffffffff));
                  }
                  SSI[i].unsynchronized();
               }
            }
         }
         unsynchronized();
      }
   }
}
