// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Sender Implementation                                        ####
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
#include "rtpsender.h"
#include "rtcpsender.h"
#include "randomizer.h"

#include <signal.h>


// Debug mode: Print some debug information
#define DEBUG


// ###### Constructor #######################################################
RTPSender::RTPSender()
   : TimedThread(1000000,"RTPSender")
{
   Encoder      = NULL;
   SenderSocket = NULL;
}


// ###### Constructor #######################################################
RTPSender::RTPSender(const card32      ssrc,
                     EncoderInterface* encoder,
                     Socket*           senderSocket,
                     const cardinal    maxPacketSize,
                     BandwidthManager* bwManager)
   : TimedThread(1000000,"RTPSender")
{
   init(ssrc,encoder,senderSocket,maxPacketSize,bwManager);
}


// ###### Initialize ########################################################
void RTPSender::init(const card32      ssrc,
                     EncoderInterface* encoder,
                     Socket*           senderSocket,
                     const cardinal    maxPacketSize,
                     BandwidthManager* bwManager)
{
   Encoder            = encoder;
   SenderSocket       = senderSocket;
   BandwidthMgr       = bwManager;
   MaxPacketSize      = maxPacketSize;
   BytesSent          = 0;
   PacketsSent        = 0;
   PayloadPacketsSent = 0;
   PayloadBytesSent   = 0;
   FramesPerSecond    = 0;
   RenewCounter       = 0;
   Pause              = false;
   TransmissionError  = false;
   TimeStamp          = getMicroTime();
   SSRC               = ssrc;
   Randomizer random;

   for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
      // ====== Initialize values for each layer ============================
      SenderSocket->getPeerAddress(Flow[i]);
      SequenceNumber[i] = random.random16();
#ifdef USE_TRAFFICSHAPER
      Shaper[i].setSocket(SenderSocket);
      SenderReportBuffer.setSocket(SenderSocket);
      SenderReportBuffer.setBandwidth(1000000000);
      SenderReportBuffer.setBufferDelay(1000000.0);
#endif
   }

   const AbstractQoSDescription* aqd = getQoSDescription(0);
   if(aqd != NULL) {
      update(aqd);
      delete aqd;
   }


   else {
   setInterval((card64)(1000000.0 / 35));  //???? HACK!!
   puts("********************* =?????? FR-HACK!!");
   }

}


// ###### Destructor ########################################################
RTPSender::~RTPSender()
{
   stop();
}


// ###### Get QoS description ###############################################
AbstractQoSDescription* RTPSender::getQoSDescription(const card64 offset)
{
   if(Encoder != NULL) {
      synchronized();

      // ====== Get packet and header sizes =================================
      InternetAddress peerAddress;
      SenderSocket->getPeerAddress(peerAddress);
      const cardinal headerSize    = ((peerAddress.isIPv6()) ? IPv6HeaderSize : IPv4HeaderSize) +
                                        UDPHeaderSize +
                                        RTPConstants::RTPDefaultHeaderSize;
      const cardinal maxPacketSize = min(MaxPacketSize, headerSize + RTPConstants::RTPMaxPayloadLimit);

      // ====== Get QoS description =========================================
      AbstractQoSDescription* aqd =
         Encoder->getQoSDescription(headerSize,maxPacketSize,offset);

      // ====== Set addresses ===============================================
      if(aqd != NULL) {
         InternetAddress localAddress;
         SenderSocket->getSocketAddress(localAddress);
         const cardinal layers = min(aqd->getLayers(),RTPConstants::RTPMaxQualityLayers);
         for(cardinal i = 0;i < layers;i++) {
            AbstractLayerDescription* ald = aqd->getLayer(i);
            ald->setSource(localAddress);
            ald->setDestination(Flow[i]);
         }
         update(aqd);
      }

      unsynchronized();
      return(aqd);
   }
   return(NULL);
}


// ###### Update QoS description ############################################
void RTPSender::updateQuality(const AbstractQoSDescription* aqd)
{
   if(Encoder != NULL) {
      synchronized();

      // ====== Update traffic constraints ==================================
      const cardinal layers    = min(aqd->getLayers(),RTPConstants::RTPMaxQualityLayers);
      const double   frameRate = aqd->getFrameRate();
      for(cardinal i = 0;i < layers;i++) {
         AbstractLayerDescription* ald = aqd->getLayer(i);
         Flow[i] = ald->getDestination();

         Bandwidth[i]    = ald->getBandwidth();
         BufferDelay[i]  =
            (cardinal)ceil((double)ald->getBufferDelay() * (1000000.0 / frameRate));

#ifdef USE_TRAFFICSHAPER
         Shaper[i].setBandwidth(Bandwidth[i] + RTPConstants::RTPDefaultHeaderSize + IPv6HeaderSize + UDPHeaderSize);
         Shaper[i].setBufferDelay(BufferDelay[i] + BandwidthMgr->SystemDelayTolerance);
         if(Shaper[i].refreshBuffer(Flow[i].getTrafficClass(),true) == true) {
            BandwidthMgr->bufferFlushEvent(this,i);
         }
#endif
      }

      Encoder->updateQuality(aqd);
      update(aqd);

      unsynchronized();
   }
}


// ###### Update frame rate #################################################
void RTPSender::update(const AbstractQoSDescription* aqd)
{
   const double frameRate = aqd->getFrameRate();
   if(frameRate <= 1.0) {
      setInterval(1000000);
      FramesPerSecond = 1;
   }
   else {
      setInterval((card64)(1000000.0 / frameRate));
      FramesPerSecond = (cardinal)ceil(1000000.0 / frameRate);
   }
}


// ###### Lock sender #######################################################
void RTPSender::lock()
{
   synchronized();
}


// ###### Unlock sender #####################################################
void RTPSender::unlock()
{
   unsynchronized();
}


// ###### Sender loop #######################################################
void RTPSender::timerEvent()
{
   if(Encoder == NULL) {
      cerr << "ERROR: RTPSender::timerEvent() - RTPSender is uninitialized!"
           << endl;
      return;
   }
   synchronized();


   // ==== Renew flow label reservation and send sender report every second =
   RenewCounter++;
   if(RenewCounter >= FramesPerSecond) {
      // ====== Renew flow label ============================================
      SenderSocket->renewFlow(10);
      for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
         SenderSocket->renewFlow(Flow[i],10);
      }
      RenewCounter = 0;

      // ====== Send RTCP Sender Report =====================================
      RTCPSenderReport report(SSRC,0);
      const card64 now = getMicroTime();
      report.setNTPTimeStamp(now);
      report.setRTPTimeStamp(
         (card32)rint((double)(now - TimeStamp) /
                      RTPConstants::RTPMicroSecondsPerTimeStamp) & 0xffffffff);
      report.setOctetsSent(PayloadBytesSent);
      report.setPacketsSent(PayloadPacketsSent);
#ifdef USE_TRAFFICSHAPER
      if(SenderReportBuffer.send(&report,sizeof(RTCPSenderReport),(cardinal)-1,(SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED : 0) != sizeof(RTCPSenderReport)) {
#else
      if(SenderSocket->send(&report,sizeof(RTCPSenderReport),(SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED : 0) != sizeof(RTCPSenderReport)) {
#endif
         const integer error = SenderSocket->getLastError();
         if((TransmissionError == false) && (error != EAGAIN) && (error != EINTR)) {
#ifdef DEBUG
            cerr << "RTPSender::timerEvent() - Transmission of RTCP SR failed!" << endl;
#endif
            TransmissionError = true;
         }
      }
   }


   // ====== Transmit next frame ============================================
   RTPPacket packet;
   InternetAddress peerAddress;
   SenderSocket->getPeerAddress(peerAddress);
   const cardinal headerSizeTransport =
      ((peerAddress.isIPv6()) ? IPv6HeaderSize : IPv4HeaderSize) +
      UDPHeaderSize;
   const cardinal headerSizeRTP = packet.calculateHeaderSize();
   const cardinal maxPacketSize =
      min(MaxPacketSize, headerSizeTransport + headerSizeRTP + RTPConstants::RTPMaxPayloadLimit);
   const cardinal maxPayloadSize =
      maxPacketSize - (headerSizeTransport + headerSizeRTP);


   if(!Pause) {
      card64     nextInterval = (card64)-1;
      bool       newRUList    = false;
      const bool newInterval  = Encoder->checkInterval(nextInterval,newRUList);

      // ====== Check for new interval ======================================
      if(BandwidthMgr != NULL) {
         BandwidthMgr->intervalChangeEvent(this,newInterval,nextInterval,newRUList);
      }
   }


   // ====== Prepare next frame for sending =================================
   if((!Pause) && (Encoder->prepareNextFrame(headerSizeTransport + headerSizeRTP,
                                             maxPacketSize) == true)) {

      // ====== Get packet and transmit packet loop =========================
      cardinal bytesData = 0;
      for(;;) {
         // ====== Get next packet from encoder =============================
         EncoderPacket encoderPacket;
         encoderPacket.Buffer      = packet.getPayloadData();
         encoderPacket.MaxLength   = maxPayloadSize;
         encoderPacket.Layer       = 0;
         encoderPacket.Marker      = false;
         encoderPacket.PayloadType = 0x00;
         encoderPacket.ErrorCode   = ME_NoError;
         bytesData = Encoder->getNextPacket(&encoderPacket);
cerr << "";
         if(bytesData > maxPayloadSize) {
            cerr << "WARNING: RTPSender::timerEvent() - Encoder exceeds packet size limit!" << endl;
            bytesData = 0;
         }

         // ====== Check, if frame has been transmitted completely ==========
         if(bytesData > 0) {
            // ====== Initialize RTP packet header ==========================
            packet.setMarker(encoderPacket.Marker);
            packet.setPayloadType(encoderPacket.PayloadType);
            packet.setSequenceNumber(SequenceNumber[encoderPacket.Layer]);
            packet.setSSRC(SSRC);
            packet.setTimeStamp((card32)rint((double)(getMicroTime() - TimeStamp) /
                                RTPConstants::RTPMicroSecondsPerTimeStamp) & 0xffffffff);

            // ====== Send packet via traffic shaper =========================
#ifdef USE_TRAFFICSHAPER
            ssize_t sent;
            if(encoderPacket.ErrorCode >= ME_UnrecoverableError) {
                sent = SenderReportBuffer.sendTo(
                          &packet,
                          packet.calculateHeaderSize() + bytesData,
                          SequenceNumber[encoderPacket.Layer],
                          (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED : 0,
                          Flow[encoderPacket.Layer],
                          Flow[encoderPacket.Layer].getTrafficClass());
            }
            else {
                sent = Shaper[encoderPacket.Layer].sendTo(
                          &packet,
                          packet.calculateHeaderSize() + bytesData,
                          SequenceNumber[encoderPacket.Layer],
                          (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED : 0,
                          Flow[encoderPacket.Layer],
                          Flow[encoderPacket.Layer].getTrafficClass());
            }
            if(sent < 0) {
               SequenceNumber[encoderPacket.Layer] = Shaper[encoderPacket.Layer].getLastSeqNum();
               if(BandwidthMgr != NULL) {
                  BandwidthMgr->bufferFlushEvent(this,encoderPacket.Layer);
               }
               packet.setSequenceNumber(SequenceNumber[encoderPacket.Layer]);
               sent = Shaper[encoderPacket.Layer].sendTo(
                         &packet,
                         packet.calculateHeaderSize() + bytesData,
                         SequenceNumber[encoderPacket.Layer],
                         (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED : 0,
                         Flow[encoderPacket.Layer],
                         Flow[encoderPacket.Layer].getTrafficClass());
            }

            // ====== Send packet without traffic shaper ====================
#else
printf("send: %d\n",bytesData);
            const ssize_t sent = SenderSocket->sendTo(
                                    &packet,
                                    packet.calculateHeaderSize() + bytesData,
                                    (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED : 0,
                                    Flow[encoderPacket.Layer],
                                    Flow[encoderPacket.Layer].getTrafficClass());
#endif

            // ====== Update counters and sequence number ===================
            if(sent > 0) {
               PayloadBytesSent += (card32)sent;
               PayloadPacketsSent++;
               BytesSent += sent + headerSizeTransport;
               PacketsSent++;
               SequenceNumber[encoderPacket.Layer]++;
            }
            else {
               const integer error = SenderSocket->getLastError();
               if((error != 0) && (TransmissionError == false) && (error != EAGAIN) && (error != EINTR)) {
                  cerr << "WARNING: RTPSender::timerEvent() - "
                       << "Unable to send " << packet.calculateHeaderSize() + bytesData
                       << " bytes to " << Flow[encoderPacket.Layer] << endl
                       << "Transmission error #"
                       << error << ": " << strerror(error) << endl;
                  TransmissionError = true;
                  break;
               }
            }


            // ====== Check for traffic shaper transmission errors ==========
#ifdef USE_TRAFFICSHAPER
            const integer error = SenderSocket->getLastError();
            if((error > 0) && (error != -EINTR) && (TransmissionError == false)) {
               if(error != -ECONNREFUSED) {
                  cerr << "WARNING: RTPSender::timerEvent() - Transmission error #"
                       << error << ": " << strerror(error) << endl;
               }
               TransmissionError = true;
               break;
            }
#endif
         }
         else {
            break;
         }

/*
??????????????????????????
         // ====== Check for pending timer event ============================
         if(pendingTimerEvent(0)) {
            // There is a new timer event -> Skip rest of this frame!
#ifdef DEBUG
            cerr << "NOTE: RTPSender::timerEvent() - Pending timer -> "
                 << "Skipping rest of frame!" << endl;
#endif
            break;
         }
*/
      }
   }

   unsynchronized();
}
