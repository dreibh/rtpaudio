// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Receiver Implementation                                      ####
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

   RTPPacket     packet;
   integer       bytes;
   InternetFlow  flow;

   for(;;) {
      // ====== Read an RTP packet ==========================================
      bytes = ReceiverSocket->receiveFrom(&packet,sizeof(RTPPacket),flow);

      // ====== Verify RTP packet ===========================================
      if(packet.getVersion() != RTPConstants::RTPVersion) {
#ifdef DEBUG
         std::cerr << "RTP packet: Invalid version " << packet.getVersion() << std::endl;
#endif
         continue;
      }


      // ==== Packet loss simulation ========================================
/*
      Randomizer r;
      cardinal i = r.random32() % 20;
      if(i == 1) bytes = 0;
*/


      // ====== Process packet, if read was successful ======================
      if(bytes > 0) {
         synchronized();

         // ====== Check, if decoder accepts packet =========================
         DecoderPacket decoderPacket;
         decoderPacket.Buffer         = packet.getPayloadData();
         decoderPacket.Length         = bytes - packet.calculateHeaderSize();
         decoderPacket.SequenceNumber = packet.getSequenceNumber();
         decoderPacket.TimeStamp      = packet.getTimeStamp();
         decoderPacket.SSIArray       = (SourceStateInfo**)&SSI;
         decoderPacket.Marker         = packet.getMarker();
         decoderPacket.PayloadType    = packet.getPayloadType();
         decoderPacket.Layer          = (cardinal)-1;
         decoderPacket.Layers         = (cardinal)-1;

         // ====== Paket ist RTP-Paket fr den Decoder ======================
         if(Decoder->checkNextPacket(&decoderPacket) == true) {
            // Check, if packet's layer number is valid. ==
            if(decoderPacket.Layers <= RTPConstants::RTPMaxQualityLayers) {
               if(decoderPacket.Layer < decoderPacket.Layers) {
                  // Update SSI and check, if packet's sequence number is valid.
                  SSI[decoderPacket.Layer].synchronized();
                  SSI[decoderPacket.Layer].setSSRC(packet.getSSRC());
                  SeqNumValidator::ValidationResult valid =
                     SSI[decoderPacket.Layer].validate(packet.getSequenceNumber(),packet.getTimeStamp());
                  SSI[decoderPacket.Layer].unsynchronized();

                  // ====== Decoder packet ===================================
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
                     BytesReceived[decoderPacket.Layer] += bytes;
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
            RTCPSenderReport* report = (RTCPSenderReport*)&packet;
            if((bytes >= (ssize_t)sizeof(RTCPSenderReport)) &&
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
