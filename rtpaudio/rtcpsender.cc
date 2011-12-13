// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Sender Implementation                                       ####
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


#include "tdsystem.h"
#include "rtcpsender.h"


// ###### Constructor #######################################################
RTCPSender::RTCPSender()
   : TimedThread(1000000,"RTCPSender")
{
   SenderSocket = NULL;
   Receiver     = NULL;
}


// ###### Constructor #######################################################
RTCPSender::RTCPSender(const card32 ssrc,
                       Socket*      senderSocket,
                       RTPReceiver* receiver,
                       const card64 bandwidth)
   : TimedThread(1000000,"RTCPSender")
{
   init(ssrc,senderSocket,receiver,bandwidth);
}


// ###### Initialize ########################################################
void RTCPSender::init(const card32 ssrc,
                      Socket*      senderSocket,
                      RTPReceiver* receiver,
                      const card64 bandwidth)
{
   SSRC            = ssrc;
   Receiver        = receiver;
   SenderSocket    = senderSocket;

   Initial         = true;
   WeSent          = false;
   Senders         = 1;
   Members         = 1;
   RTCPBandwidth   = (double)bandwidth;
   AverageRTCPSize = 200;

   setFastStart(false);
   setTimerCorrection(0);
}


// ###### Destructor ########################################################
RTCPSender::~RTCPSender()
{
   stop();

   // ====== Delete SDES item list ==========================================
   while(SDESItemSet.begin() != SDESItemSet.end()) {
       std::multimap<const card8,RTCPSourceDescriptionItem*>::iterator sdesIterator =
          SDESItemSet.begin();
       RTCPSourceDescriptionItem* item = sdesIterator->second;
       SDESItemSet.erase(sdesIterator);
       delete item;
   }
}


// ###### Periodic transmission of reports and SDES messages ################
void RTCPSender::timerEvent()
{
   if(SenderSocket == NULL) {
      std::cerr << "ERROR: RTCPSender::timerEvent() - RTCPSender is uninitialized!"
                << std::endl;
      return;
   }

   // ====== Renew flow label and send report ===============================
   SenderSocket->renewFlow(10);
   setInterval((card64)(computeTransmissionInterval() * 1000000.0));
   sendReport();
   sendSDES();
}


// ###### Send RTCP BYE message #############################################
integer RTCPSender::sendBye()
{
   if(SenderSocket != NULL) {
      char     packet[sizeof(RTCPBye) + 4];
      RTCPBye* bye = (RTCPBye*)&packet;

      bye->init(1);
      bye->setSource(0,SSRC);
      bye->setLength(sizeof(packet));

      return(SenderSocket->send((void*)&packet,sizeof(packet),
                (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED|MSG_NOSIGNAL : MSG_NOSIGNAL));
   }
   return(0);
}


// ###### Send RTCP APP message #############################################
integer RTCPSender::sendApp(const char*    name,
                            const void*    data,
                            const cardinal dataLength)
{
   if(SenderSocket != NULL) {
      char     packet[sizeof(RTCPApp) + dataLength];
      RTCPApp* app = (RTCPApp*)&packet;

      app->init(0);
      app->setSource(SSRC);
      app->setName(name);
      app->setLength(sizeof(packet));
      memcpy(app->getData(),data,dataLength);

      return(SenderSocket->send((void *)&packet,sizeof(packet),
               (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED|MSG_NOSIGNAL : MSG_NOSIGNAL));
   }
   return(0);
}


// ###### Add SDES item to item list ########################################
bool RTCPSender::addSDESItem(const card8 type,
                             const void* data,
                             const card8 length)
{
   synchronized();

   // ====== Initialize new packet ==========================================
   card16 len = length;
   if((len == 0) && (data != NULL)) len = strlen((char*)data);
   char* ptr = new char[sizeof(RTCPSourceDescriptionItem) + len];
   if(ptr == NULL) {
      unsynchronized();
      return(false);
   }
   RTCPSourceDescriptionItem* packet = (RTCPSourceDescriptionItem*)ptr;
   packet->Type   = type;
   packet->Length = len;
   if(data != NULL) {
      memcpy(&packet->Data,data,len);
   }

   // ====== Remove old packet ==============================================
   removeSDESItem(type);

   // ====== Add new packet =================================================
   SDESItemSet.insert(std::pair<const card8,RTCPSourceDescriptionItem*>(type,packet));

   unsynchronized();
   return(true);
}


// ###### Remove SDES item from item list ###################################
void RTCPSender::removeSDESItem(const card8 type)
{
   synchronized();
   std::multimap<const card8,RTCPSourceDescriptionItem*>::iterator sdesIterator =
      SDESItemSet.find(type);
   if(sdesIterator != SDESItemSet.end()) {
       RTCPSourceDescriptionItem* item = sdesIterator->second;
       SDESItemSet.erase(sdesIterator);
       delete [] (char*)item;
   }
   unsynchronized();
}


// ###### Send SDES messages ################################################
integer RTCPSender::sendSDES()
{
   if(SenderSocket != NULL) {
      synchronized();
      if(SDESItemSet.size() > 0) {
         // ====== Initialize packet ========================================
         char ptr[256 * (SDESItemSet.size() + 1)];
         RTCPSourceDescription* sdes = (RTCPSourceDescription*)ptr;
         sdes->init(1);
         sdes->Chunk[0].SRC = SSRC;

         // Copy all SDES item into one packet
         cardinal bytes = sizeof(RTCPSourceDescription);
         char*    adr   = (char*)&sdes->Chunk[0].Item[0];

         std::multimap<const card8,RTCPSourceDescriptionItem*>::iterator sdesIterator =
            SDESItemSet.begin();
         while(sdesIterator != SDESItemSet.end()) {
            RTCPSourceDescriptionItem* item = sdesIterator->second;
            memcpy(adr,item,sizeof(RTCPSourceDescriptionItem) + item->Length);
            adr = (char*)((long)adr + (long)sizeof(RTCPSourceDescriptionItem) +
                          (long)item->Length);
            bytes += sizeof(RTCPSourceDescriptionItem) + item->Length;
            sdesIterator++;
         }

         // ====== Mark the end of the SDES chunk ===========================
         if(bytes % 4) {
            adr[0] = RTCP_SDES_END;
            adr[1] = RTCP_SDES_END;
            adr[2] = RTCP_SDES_END;
            bytes += 4 - (bytes % 4);
         }
         sdes->setLength(bytes);

         // ====== Send packet ==============================================
         unsynchronized();
         const int result = SenderSocket->send((void*)sdes,bytes,
                               (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED|MSG_NOSIGNAL : MSG_NOSIGNAL);
         return(result);
      }
      unsynchronized();
   }
   return(0);
}


// ###### Send Receiver Report ##############################################
integer RTCPSender::sendReport()
{
   if(Receiver != NULL) {
      Receiver->synchronized();

      // ====== Get report data from SourceStateInfos =======================
      const cardinal layers = std::min(Receiver->getLayers(),RTPConstants::RTPMaxQualityLayers);
      cardinal length       = sizeof(RTCPReceiverReport);
      length += layers * sizeof(RTCPReceptionReportBlock);

      char* buffer[length];
      RTCPReceiverReport* report = (RTCPReceiverReport*)&buffer;
      report->init(SSRC,layers);
      report->setLength(length);

      for(cardinal i = 0;i < layers;i++) {
         report->rr[i].setSSRC(Receiver->SSI[i].getSSRC());
         report->rr[i].setFractionLost(Receiver->SSI[i].calculateFractionLost());
         report->rr[i].setPacketsLost((card32)Receiver->SSI[i].getPacketsLost());
         report->rr[i].setLastSeqNum((card32)Receiver->SSI[i].getLastSeqNum());
         report->rr[i].setJitter((card32)Receiver->SSI[i].getJitter());
         report->rr[i].setLSR(Receiver->SSI[i].getLSR());
         report->rr[i].setDLSR(Receiver->SSI[i].calculateDLSR());
      }

      Receiver->unsynchronized();

/*
      std::cout << "RTCPReceiverReport" << std::endl;
      std::cout << "   RTCP Common Header:" << std::endl;
      std::cout << "      Version     = " << report->getVersion()    << std::endl;
      std::cout << "      Padding     = " << report->getPadding()    << std::endl;
      std::cout << "      Count       = " << report->getCount()      << std::endl;
      std::cout << "      Packet Type = " << (cardinal)report->getPacketType() << std::endl;
      std::cout << "      Length      = " << report->getLength()     << std::endl;
      std::cout << "   RTCP Report:" << std::endl;
      std::cout << "      SSRC = " << report->getSSRC() << std::endl;
      for(cardinal i = 0;i < layers;i++) {
         std::cout << "   RTCP Receiver Report #" << i << ":" << std::endl;
         std::cout << "      SSRC            = " << report->rr[i].getSSRC()         << std::endl;
         std::cout << "      Fraction Lost   = " << report->rr[i].getFractionLost() << std::endl;
         std::cout << "      Packets Lost    = " << report->rr[i].getPacketsLost()  << std::endl;
         std::cout << "      Last Seq Number = " << report->rr[i].getLastSeqNum()   << std::endl;
         std::cout << "      Interar. Jitter = " << report->rr[i].getJitter()       << std::endl;
         std::cout << "      LSR             = " << report->rr[i].getLSR()          << std::endl;
         std::cout << "      DLSR            = " << report->rr[i].getDLSR()         << std::endl;
      }
      std::cout << std::endl;
*/

      return(SenderSocket->send((void*)report,length,
               (SenderSocket->getProtocol() == IPPROTO_SCTP) ? SCTP_UNORDERED|MSG_NOSIGNAL : MSG_NOSIGNAL));
   }
   return(0);
}


// ###### Compute transmission interval in seconds (from RFC 1889) ##########
double RTCPSender::computeTransmissionInterval()
{
   /*
    * Minimum average time between RTCP packets from this site (in
    * seconds).  This time prevents the reports from `clumping' when
    * sessions are small and the law of large numbers isn't helping
    * to smooth out the traffic.  It also keeps the report interval
    * from becoming ridiculously small during transient outages like
    * a network partition.
    */
   double const RTCP_MIN_TIME = 5.0;

   /*
    * Fraction of the RTCP bandwidth to be shared among active
    * Senders.  (This fraction was chosen so that in a typical
    * session with one or two active Senders, the computed report
    * time would be roughly equal to the minimum report time so that
    * we don't unnecessarily slow down receiver reports.) The
    * receiver fraction must be 1 - the sender fraction.
    */
   double const RTCP_SENDER_BW_FRACTION = 0.25;
   double const RTCP_RECVER_BW_FRACTION = (1 - RTCP_SENDER_BW_FRACTION);

   /*
    * To compensate for "unconditional reconsideration" converging to a
    * value below the intended average.
    */
   double const COMPENSATION = 2.71828 - 1.5;
   double rtcp_min_time = RTCP_MIN_TIME;

   /*
    * Very first call at application start-up uses half the min
    * delay for quicker notification while still allowing some time
    * before reporting for randomization and to learn about other
    * sources so the report interval will converge to the correct
    * interval more quickly.
    */
   if(Initial) {
      rtcp_min_time = rtcp_min_time / 2;
   }

   /*
    * If there were active Senders, give them at least a minimum
    * share of the RTCP bandwidth.  Otherwise all participants share
    * the RTCP bandwidth equally.
    */
   integer n = Members;            /* no. of Members for computation */

   if(Senders > 0 && Senders < Members * RTCP_SENDER_BW_FRACTION) {
      if(WeSent) {
          RTCPBandwidth = RTCPBandwidth * RTCP_SENDER_BW_FRACTION;
          n = Senders;
      }
      else {
         RTCPBandwidth = RTCPBandwidth * RTCP_RECVER_BW_FRACTION;
	 n = n - Senders;
      }
   }

   /*
    * The effective number of sites times the average packet size is
    * the total number of octets sent when each site sends a report.
    * Dividing this by the effective bandwidth gives the time
    * interval over which those packets must be sent in order to
    * meet the bandwidth target, with a minimum enforced.  In that
    * time interval we send one report so this time is also our
    * average time between reports.
    */
   double t = AverageRTCPSize * n / RTCPBandwidth;            /* interval */
   if(t < rtcp_min_time) t = rtcp_min_time;

   /*
    * To avoid traffic bursts from unintended synchronization with
    * other sites, we then pick our actual next report interval as a
    * random number uniformly distributed between 0.5*t and 1.5*t.
    */
   t = t * (Random.random() + 0.5);
   t = t / COMPENSATION;

   return t;
}
