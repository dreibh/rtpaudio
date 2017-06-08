// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Sender Implementation                                        ####
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


#include "tdsystem.h"
#include "tdmessage.h"
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
RTPSender::RTPSender(const InternetFlow&  flow,
                     const card32         ssrc,
                     EncoderInterface*    encoder,
                     Socket*              senderSocket,
                     const card32         controlPPID,
                     const card32         dataPPID,
                     const cardinal       maxPacketSize,
                     QoSManagerInterface* qosManager)
   : TimedThread(1000000,"RTPSender")
{
   init(flow,ssrc,encoder,senderSocket,controlPPID,dataPPID,maxPacketSize,qosManager);
}


// ###### Initialize ########################################################
void RTPSender::init(const InternetFlow&  flow,
                     const card32         ssrc,
                     EncoderInterface*    encoder,
                     Socket*              senderSocket,
                     const card32         controlPPID,
                     const card32         dataPPID,
                     const cardinal       maxPacketSize,
                     QoSManagerInterface* qosManager)
{
   Encoder            = encoder;
   SenderSocket       = senderSocket;
   QoSMgr             = qosManager;
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
   ControlPPID        = controlPPID;
   DataPPID           = dataPPID;
   Randomizer random;

   for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
      // ====== Initialize values for each layer ============================
      Flow[i]           = flow;
      SequenceNumber[i] = random.random16();
#ifdef USE_TRAFFICSHAPER
      Shaper[i].setSocket(SenderSocket);
      SenderReportBuffer.setSocket(SenderSocket);
      SenderReportBuffer.setBandwidth(1000000000);
      SenderReportBuffer.setBufferDelay(1000000.0);
#endif
   }

   updateFrameRate(NULL);
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
      const cardinal maxPacketSize = std::min(MaxPacketSize, headerSize + RTPConstants::RTPMaxPayloadLimit);

      // ====== Get QoS description =========================================
      AbstractQoSDescription* aqd =
         Encoder->getQoSDescription(headerSize,maxPacketSize,offset);

      // ====== Set addresses ===============================================
      if(aqd != NULL) {
         InternetAddress localAddress;
         SenderSocket->getSocketAddress(localAddress);
         const cardinal layers = std::min(aqd->getLayers(),RTPConstants::RTPMaxQualityLayers);
         for(cardinal i = 0;i < layers;i++) {
            AbstractLayerDescription* ald = aqd->getLayer(i);
            ald->setSource(localAddress);
            ald->setDestination(Flow[i]);
         }
      }
      updateFrameRate(aqd);

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
      const cardinal layers    = std::min(aqd->getLayers(),RTPConstants::RTPMaxQualityLayers);
      const double   frameRate = aqd->getFrameRate();
      for(cardinal i = 0;i < layers;i++) {
         AbstractLayerDescription* ald = aqd->getLayer(i);
         Flow[i] = ald->getDestination();

         Bandwidth[i]   = ald->getBandwidth();
         BufferDelay[i] =
            (cardinal)ceil((double)ald->getBufferDelay() * (1000000.0 / frameRate));

#ifdef USE_TRAFFICSHAPER
         Shaper[i].setBandwidth(Bandwidth[i] + RTPConstants::RTPDefaultHeaderSize + IPv6HeaderSize + UDPHeaderSize);
         Shaper[i].setBufferDelay(BufferDelay[i] + QoSMgr->SystemDelayTolerance);
         if(Shaper[i].refreshBuffer(Flow[i].getTrafficClass(),true) == true) {
            QoSMgr->bufferFlushEvent(this,i);
         }
#endif
      }

      Encoder->updateQuality(aqd);
      updateFrameRate(aqd);

      unsynchronized();
   }
}


// ###### Update frame rate #################################################
void RTPSender::updateFrameRate(const AbstractQoSDescription* aqd)
{
   double frameRate;
   if(aqd) {
      frameRate = aqd->getFrameRate();
   }
   else {
      frameRate = Encoder->getFrameRate();
   }
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
      std::cerr << "ERROR: RTPSender::timerEvent() - RTPSender is uninitialized!"
                << std::endl;
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

      // ====== Create RTCP Sender Report ===================================
      RTCPSenderReport report(SSRC,0);
      const card64 now = getMicroTime();
      report.setNTPTimeStamp(now);
      report.setRTPTimeStamp(
         (card32)rint((double)(now - TimeStamp) /
                      RTPConstants::RTPMicroSecondsPerTimeStamp) & 0xffffffff);
      report.setOctetsSent(PayloadBytesSent);
      report.setPacketsSent(PayloadPacketsSent);

      // ====== Send RTCP Sender Report =====================================
#ifdef USE_TRAFFICSHAPER
      if(SenderReportBuffer.send(&report,sizeof(RTCPSenderReport),(cardinal)-1,(SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED|MSG_NOSIGNAL : MSG_NOSIGNAL) != sizeof(RTCPSenderReport)) {
#else
#ifndef WITH_NEAT
      SocketMessage<sizeof(sctp_sndrcvinfo)> message;
#else
      SocketMessage<0> message;
#endif
      message.setBuffer(&report,sizeof(RTCPSenderReport));
      message.setAddress(Flow[0],SenderSocket->getFamily());
#ifndef WITH_NEAT
      if(SenderSocket->getProtocol() == IPPROTO_SCTP) {
         sctp_sndrcvinfo* info = (sctp_sndrcvinfo*)message.addHeader(
                                    sizeof(sctp_sndrcvinfo),IPPROTO_SCTP,SCTP_SNDRCV);
         info->sinfo_assoc_id   = 0;
         info->sinfo_stream     = 0;
         info->sinfo_flags      = SCTP_UNORDERED;
         info->sinfo_timetolive = 100;   // 100ms
         info->sinfo_ppid       = htonl(ControlPPID);
      }
#endif
      if(SenderSocket->sendMsg(&message.Header,MSG_NOSIGNAL,Flow[0].getTrafficClass()))  {
#endif
         const integer error = SenderSocket->getLastError();
         if((TransmissionError == false) && (error != EAGAIN) && (error != EINTR)) {
#ifdef DEBUG
            std::cerr << "RTPSender::timerEvent() - Transmission of RTCP SR failed!" << std::endl;
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
      std::min(MaxPacketSize, headerSizeTransport + headerSizeRTP + RTPConstants::RTPMaxPayloadLimit);
   const cardinal maxPayloadSize =
      maxPacketSize - (headerSizeTransport + headerSizeRTP);


   if(!Pause) {
      card64     nextInterval = (card64)-1;
      bool       newRUList    = false;
      const bool newInterval  = Encoder->checkInterval(nextInterval,newRUList);

      // ====== Check for new interval ======================================
      if(QoSMgr != NULL) {
         QoSMgr->intervalChangeEvent(this,newInterval,nextInterval,newRUList);
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
         if(bytesData > maxPayloadSize) {
            std::cerr << "WARNING: RTPSender::timerEvent() - Encoder exceeds packet size limit!" << std::endl;
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
            ssize_t sent;
#ifdef USE_TRAFFICSHAPER
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
               if(QoSMgr != NULL) {
                  QoSMgr->bufferFlushEvent(this,encoderPacket.Layer);
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
#ifndef WITH_NEAT
            SocketMessage<sizeof(sctp_sndrcvinfo)> message;
#else
            SocketMessage<0> message;
#endif
            message.setBuffer(&packet, packet.calculateHeaderSize() + bytesData);
            message.setAddress(Flow[encoderPacket.Layer], SenderSocket->getFamily());
#ifndef WITH_NEAT
            if(SenderSocket->getProtocol() == IPPROTO_SCTP) {
               sctp_sndrcvinfo* info = (sctp_sndrcvinfo*)message.addHeader(
                                          sizeof(sctp_sndrcvinfo),IPPROTO_SCTP,SCTP_SNDRCV);
               info->sinfo_assoc_id   = 0;
               info->sinfo_stream     = (unsigned short)encoderPacket.Layer;
               info->sinfo_flags      = SCTP_UNORDERED;
               info->sinfo_timetolive = 100;   // 100ms
               info->sinfo_ppid       = htonl(DataPPID);
            }
#endif
            sent = SenderSocket->sendMsg(&message.Header,MSG_NOSIGNAL,Flow[encoderPacket.Layer].getTrafficClass());
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
                  std::cerr << "WARNING: RTPSender::timerEvent() - "
                            << "Unable to send " << packet.calculateHeaderSize() + bytesData
                            << " bytes to " << Flow[encoderPacket.Layer] << std::endl
                            << "Transmission error #"
                            << error << ": " << strerror(error) << std::endl;
                  TransmissionError = true;
                  break;
               }
            }


            // ====== Check for traffic shaper transmission errors ==========
#ifdef USE_TRAFFICSHAPER
            const integer error = SenderSocket->getLastError();
            if((error > 0) && (error != -EINTR) && (TransmissionError == false)) {
               if(error != -ECONNREFUSED) {
                  std::cerr << "WARNING: RTPSender::timerEvent() - Transmission error #"
                            << error << ": " << strerror(error) << std::endl;
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
         // ====== Check for pending timer event ============================
         if(pendingTimerEvent(0)) {
            // There is a new timer event -> Skip rest of this frame!
#ifdef DEBUG
            std::cerr << "NOTE: RTPSender::timerEvent() - Pending timer -> "
                      << "Skipping rest of frame!" << std::endl;
#endif
            break;
         }
*/
      }
   }

   unsynchronized();
}
