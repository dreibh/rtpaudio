// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Bandwidth Manager                                                ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2025 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: thomas.dreibholz@gmail.com                             ####
// ####    WWW:   https://www.nntb.no/~dreibh/rtpaudio                   ####
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
#include "bandwidthmanager.h"
#include "streamdescription.h"


// Print results
// #define PRINT_REPORTS
// #define PRINT_ROUNDTRIPTIMES
// #define PRINT_MULTIPOINTS
// #define PRINT_ALLOCATION
// #define PRINT_QUALITY
// #define PRINT_COMPLETE_REMAPPING
// #define PRINT_COMPLETE_REMAPPING_SLA
// #define PRINT_PARTIAL_REMAPPING
// #define EXIT_AFTER_REMAPPING 3   // Give number of streams here!



// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, const ResourceUtilizationSimplePoint& srup)
{
   char str[256];
   snprintf((char*)&str,sizeof(str),"P%02llu:  U=%1.3f B=%8llu C=%8.0f S=%1.8f PrF=%1.8f",
              (unsigned long long)srup.Point,
              srup.Utilization,
              (unsigned long long)srup.Bandwidth,
              srup.BandwidthCost,
              srup.SortingValue,
              srup.StreamPriorityFactor);
   os << str;
   return(os);
}


// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, const ResourceUtilizationMultiPoint& srup)
{
   char str[256];
   snprintf((char*)&str,sizeof(str),"U=%1.3f B=%7llu C=%8.0f S=%1.8f PrF=%1.8f  (",
              srup.Utilization,
              (unsigned long long)srup.Bandwidth,
              srup.BandwidthCost,
              srup.SortingValue,
              srup.SessionPriorityFactor);
   os << str;
   for(cardinal i = 0;i < srup.Streams;i++) {
      snprintf((char*)&str,sizeof(str),"S%llu=%8llu",
                  (unsigned long long)i,
                  (unsigned long long)srup.Stream[i]->RUList[srup.Point[i]].Bandwidth);
      os << str;
      if((integer)i < (integer)srup.Streams - 1) {
         os << ", ";
      }
   }
   os << ")";
   return(os);
}


// ###### Static simulator time: 0 = disable simulation -> use real time ####
card64 BandwidthManager::SimulatorTime = 0;


// ###### Constructor #######################################################
BandwidthManager::BandwidthManager(ServiceLevelAgreement* sla,
                                   RoundTripTimePinger*   rttp)
   : TimedThread(50000)
{
   setTimerCorrection(0);

   MaxRUPoints                          = 32;
   UtilizationThreshold                 = 0.01;
   BandwidthThreshold                   = (card64)-1;
   SystemDelayTolerance                 = 0.0;
   UnlayeredAllocation                  = false;

   FairnessSession                      = 0.0;
   FairnessStream                       = 1.0;

   MaxRemappingInterval                 = 5000000;
   EnablePartialRemappings              = true;
   PartialRemappingUtilizationTolerance = 0.05;
   PartialRemappingPortion              = 0.1;

   AlphaLossRate                        = (7.0 / 8.0);
   AlphaJitter                          = (7.0 / 8.0);

   SLA                           = sla;
   RTTP                          = rttp;
   Log                           = NULL;
   LogStartupTimeStamp           = 0;
   Changed                       = false;
   Streams                       = 0;
   Sessions                      = 0;
   LastCompleteRemapping         = 0;
   LastCompleteRemappingDuration = 0;
   CompleteRemappings            = 0;
   PartialRemappings             = 0;

   StreamIDGenerator       = 1;
   TotalBufferFlushes      = 0;
   TotalAvailableBandwidth = 0;
   TotalBandwidth          = 0;
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      ClassAvailableBandwidthArray[i] = 0;
      ClassBandwidthArray[i]          = 0;
      SLAUpdateRecommendation[i]      = 0;
   }
}


// ###### Destructor ########################################################
BandwidthManager::~BandwidthManager()
{
   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator iterator = StreamSet.begin();
   while(iterator != StreamSet.end()) {
      removeStream(iterator->first);
      iterator = StreamSet.begin();
   }
}


// ###### Main loop #########################################################
void BandwidthManager::timerEvent()
{
   doCompleteRemapping();
}


// ###### Set log stream ####################################################
void BandwidthManager::setLogStream(std::ostream* logStream)
{
   synchronized();
   if((Log != NULL) && (logStream == NULL)) {
      *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " Shutdown" << std::endl;
   }
   Log = logStream;
   if(Log != NULL) {
      LogStartupTimeStamp = (SimulatorTime == 0) ? getMicroTime() : SimulatorTime;
      *Log << "0 Startup" << std::endl;
   }
   unsynchronized();
}


// ###### Add stream to manager #############################################
void BandwidthManager::addStream(ManagedStreamInterface* stream,
                                 const cardinal          sessionID,
                                 const char*             name)
{
   StreamDescription* sd = new StreamDescription;
   if(sd != NULL) {
      synchronized();

      // ====== Add stream to stream set ====================================
      sd->Interface = stream;


      // ====== Add stream to session =======================================
      std::multimap<cardinal,SessionDescription*>::iterator found =
         SessionSet.find(sessionID);
      SessionDescription* session = NULL;
      if(found != SessionSet.end()) {
         session = found->second;
      }
      if(session == NULL) {
         session = new SessionDescription;
         if(session != NULL) {
            session->SessionID          = sessionID;
            session->Priority           = -128;
            session->MinWantedBandwidth = 0;
            session->MaxWantedBandwidth = 0;
            session->Streams            = 0;
            for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
               session->AllocatedBandwidthArray[i] = 0;
            }
            session->TotalAllocatedBandwidth = 0;
            SessionSet.insert(std::pair<cardinal,SessionDescription*>(sessionID,session));
            Sessions++;
         }
      }
      sd->Session = session;

      StreamSet.insert(std::pair<ManagedStreamInterface*,StreamDescription*>(stream,sd));
      if(session != NULL) {
         if(session->Streams < ResourceUtilizationMultiPoint::MaxStreamsPerSession) {
            session->StreamSet.insert(std::pair<ManagedStreamInterface*,StreamDescription*>(stream,sd));
            session->Streams++;
         }
         else {
            std::cerr << "WARNING: BandwidthManager::addStream() - Too many streams in the same session!" << std::endl;
         }
      }
      Streams++;

      // ====== Initialize stream description ===============================
      getRoundTripTimes(sd);
      sd->init(stream,SLA,MaxRUPoints,BandwidthThreshold,
               UtilizationThreshold,SystemDelayTolerance,
               UnlayeredAllocation);
      sd->StreamID = StreamIDGenerator;
      StreamIDGenerator++;

      // ====== Initialize round trip time measurement ======================
      if(RTTP != NULL) {
         const AbstractLayerDescription* ald = sd->QoSDescription->getLayer(0);
         sd->RoundTripTimeDestination = ald->getDestination();
         sd->RoundTripTimeDestination.setPrintFormat(InternetAddress::PF_Address);
         for(cardinal i = 0;i < SLA->Classes;i++) {
            RTTP->addHost(sd->RoundTripTimeDestination,SLA->Class[i].TrafficClass);
         }
      }

      // ====== Write log entry =============================================
      if(Log) {
         char str[256];
         *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " AddStream"
              << " #=" << sd->StreamID
              << " S=" << session->SessionID
              << " N=" << name << std::endl;
         for(cardinal i = 0;i < SLA->Classes;i++) {
            snprintf((char*)&str,sizeof(str),"$%02x",SLA->Class[i].TrafficClass);
            *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " DiffServClass N=" << str
                 << " B=" << SLA->Class[i].BytesPerSecond
                 << " C=" << SLA->Class[i].CostFactor
                 << " V=" << SLA->Class[i].DelayVariability << std::endl;
         }
      }

      Changed = true;
      doCompleteRemapping();
      unsynchronized();
   }
   else {
      std::cerr << "ERROR: BandwidthManager::addStream() - Out of memory!" << std::endl;
   }
}


// ###### Remove stream from manager ########################################
void BandwidthManager::removeStream(ManagedStreamInterface* stream)
{
   synchronized();
   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator found =
      StreamSet.find(stream);
   if(found != StreamSet.end()) {
      StreamDescription*  sd      = found->second;
      SessionDescription* session = sd->Session;

      // ====== Do final total cost and bandwidth usage calculation =========
      sd->NewQuality.reset();
      updateReservation(sd);

      // ====== Write log entry =============================================
      if(Log) {
         *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " RemoveStream"
              << " #=" << sd->StreamID
              << " S=" << session->SessionID << std::endl;
      }

      // ====== Remove stream from session ==================================
      if(session != NULL) {
         session->StreamSet.erase(stream);
         session->Streams--;
         if(session->Streams <= 0) {
            std::multimap<cardinal,SessionDescription*>::iterator found =
               SessionSet.find(session->SessionID);
            if(found != SessionSet.end()) {
               SessionSet.erase(found);
            }
            delete session;
         }
         Sessions--;
      }

      // ====== Remove stream from stream set ===============================
      if(sd->QoSDescription) {
         if(RTTP != NULL) {
            const AbstractLayerDescription* ald  = sd->QoSDescription->getLayer(0);
            sd->RoundTripTimeDestination = ald->getDestination();
            for(cardinal i = 0;i < SLA->Classes;i++) {
               RTTP->removeHost(sd->RoundTripTimeDestination,SLA->Class[i].TrafficClass);
            }
         }
         delete sd->QoSDescription;
      }
      delete sd;
      StreamSet.erase(stream);

      Streams--;
      Changed = true;
   }
   unsynchronized();
}


// ###### Update stream #####################################################
void BandwidthManager::updateStream(ManagedStreamInterface* stream)
{
   std::cerr << "NOTE: BandwidthManager::updateStream() - Not implemented!"
        << std::endl;
   return;
}


// ###### Get round trip times for DiffServ classes #########################
void BandwidthManager::getRoundTripTimes(StreamDescription* sd)
{
   // ====== Get round trip times from RTT pinger ===========================
   if(RTTP != NULL) {
      // ====== Get round trip times ========================================
      cardinal rtt[SLA->Classes];
      for(cardinal i = 0;i < SLA->Classes;i++) {
         rtt[i] = RTTP->getRoundTripTime(
            sd->RoundTripTimeDestination,SLA->Class[i].TrafficClass);
         // Note: The round trip time is already smoothed!
      }

      // ====== Calculate transfer delays ===================================
      for(cardinal i = 0;i < SLA->Classes;i++) {
         sd->MeasuredTransferDelay[i] =
            std::max((double)rtt[i] - (double)(rtt[SLA->BestEffort] / 2),
                     (double)(rtt[i] / 2.0));
      }

      // ====== Write log entry =============================================
      if(Log) {
         char str[256];
         *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " DelayMeasurement"
              << " #=" << sd->StreamID
              << " S=" << sd->Session->SessionID;
         *Log     << " A=<" << sd->RoundTripTimeDestination << ">";
         for(cardinal i = 0;i < SLA->Classes;i++) {
            snprintf((char*)&str,sizeof(str),"$%02x",SLA->Class[i].TrafficClass);
            *Log << " D=<" << str
                 << "," << sd->MeasuredTransferDelay[i] << ">";
         }
         *Log << std::endl;
      }

      // ====== Print information ===========================================
#ifdef PRINT_ROUNDTRIPTIMES
      char str[256];
      for(cardinal i = 0;i < SLA->Classes;i++) {
         snprintf((char*)&str,sizeof(str),"%-4s  rtt=%8Ld  delay=%8.0f",
                     TrafficClassValues::getNameForTrafficClass(SLA->Class[i].TrafficClass),
                     (card64)rtt[i],
                     sd->MeasuredTransferDelay[i]);
         std::cout << sd->RoundTripTimeDestination << "/" << str << std::endl;
      }
#endif
   }

   // ====== Get simulation values ==========================================
   else {
      for(cardinal i = 0;i < SLA->Classes;i++) {
         sd->MeasuredTransferDelay[i] = SLA->Class[i].MaxTransferDelay;
      }
   }
}


// ###### Stream's intervall has changed ####################################
void BandwidthManager::intervalChangeEvent(ManagedStreamInterface* stream,
                                           const bool              isNew,
                                           const card64            when,
                                           const bool              newRUList)
{
   // ====== Find StreamDescription =========================================
   synchronized();
   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator found =
      StreamSet.find(stream);
   if(found != StreamSet.end()) {
      StreamDescription* streamDescription = found->second;

      // ====== Update QoS description ======================================
      streamDescription->NextInterval = when;
      if((isNew) || (newRUList)) {
         // ====== Initialize stream description ============================
         streamDescription->init(stream,SLA,MaxRUPoints,BandwidthThreshold,
                                 UtilizationThreshold,SystemDelayTolerance,
                                 UnlayeredAllocation);

         // ====== Write log entry =============================================
         if(Log) {
            *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " IntervalChangeEvent"
                 << " #=" << streamDescription->StreamID
                 << " S=" << streamDescription->Session->SessionID
                 << " I=" << isNew
                 << " R=" << newRUList << std::endl;
         }
      }
      if(isNew) {
         // ====== Do complete remapping, if partial one failed or disabled =
         if((!EnablePartialRemappings) || (doPartialRemapping(streamDescription) == false)) {
#ifdef PRINT_PARTIAL_REMAPPING
            std::cout << "   Complete remapping forced." << std::endl;
#endif
            Changed = true;
            doCompleteRemapping();
         }
      }

      // ====== Update reservation ==========================================
      updateReservation(streamDescription);
   }
   unsynchronized();
}


// ###### Update reservation ################################################
void BandwidthManager::updateReservation(StreamDescription* streamDescription)
{
   // ====== Check, if quality has to be updated ============================
   if((streamDescription->Interface != NULL)      &&
      (streamDescription->QoSDescription != NULL) &&
      (streamDescription->CurrentQuality != streamDescription->NewQuality)) {

      // ====== Calculate cost of last interval =============================
      const card64 now = (SimulatorTime == 0) ? getMicroTime() : SimulatorTime;
      double realCostPerSecond = 0.0;
      for(cardinal j = 0;j < streamDescription->Layers;j++) {
         realCostPerSecond += SLA->Class[streamDescription->CurrentLayerClassNumber[j]].CostFactor *
                                 (double)streamDescription->CurrentLayerClassBandwidth[j];
      }
      if(streamDescription->ReservationTimeStamp != 0) {
         const double interval  = (double)now - streamDescription->ReservationTimeStamp;
         const double intervalS = interval / 1000000.0;
         streamDescription->TotalBandwidthUsage += (intervalS * (double)streamDescription->CurrentQuality.Bandwidth);
         streamDescription->TotalCost           += (intervalS * realCostPerSecond);
         if(streamDescription->CurrentQuality.Utilization >= 0.0) {
            streamDescription->TotalUtilization += (interval * streamDescription->CurrentQuality.Utilization);
         }
         streamDescription->TotalRuntime        += interval;
         streamDescription->TotalReservationUpdates++;
      }
      streamDescription->ReservationTimeStamp = now;

      // ====== Copy new reservation to current one =========================
      for(cardinal j = 0;j < RTPConstants::RTPMaxQualityLayers;j++) {
         streamDescription->CurrentLayerClassNumber[j]    =
            streamDescription->NewLayerClassNumber[j];
         streamDescription->CurrentLayerClassBandwidth[j] =
            streamDescription->NewLayerClassBandwidth[j];
      }

      // ====== Set traffic classes =========================================
      for(cardinal j = 0;j < streamDescription->Layers;j++) {
         AbstractLayerDescription* ald = streamDescription->QoSDescription->getLayer(j);
         InternetFlow dest = ald->getDestination();
         dest.setTrafficClass(SLA->Class[streamDescription->NewLayerClassNumber[j]].TrafficClass);
         ald->setDestination(dest);
      }

      streamDescription->CurrentQuality       = streamDescription->NewQuality;
      streamDescription->CurrentCostPerSecond = streamDescription->NewCostPerSecond;

      // ====== Set new quality =============================================
      // std::cout << "UPD: " << streamDescription->NewQuality << std::endl;
      streamDescription->Interface->updateQuality(streamDescription->QoSDescription);

      // ====== Write log entry =============================================
      if(Log) {
         char str[256];
         *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " ReservationUpdate"
              << " #=" << streamDescription->StreamID
              << " S=" << streamDescription->Session->SessionID
              << " U=" << streamDescription->CurrentQuality.Utilization
              << " C=" << streamDescription->CurrentQuality.BandwidthCost
              << " B=" << streamDescription->CurrentQuality.Bandwidth;
         for(cardinal j = 0;j < streamDescription->Layers;j++) {
            snprintf((char*)&str,sizeof(str),"$%02x",
                     SLA->Class[streamDescription->NewLayerClassNumber[j]].TrafficClass);
            *Log << " L" << j << "=<"
                 << streamDescription->CurrentQuality.LayerBandwidthInfo[j].BytesPerSecond << ","
                 << streamDescription->CurrentQuality.LayerBandwidthInfo[j].BufferDelay << ","
                 << str
                 << ">";
         }
         *Log << std::endl;
      }
   }
}


// ###### Stream's report has been received #################################
void BandwidthManager::reportEvent(ManagedStreamInterface*         stream,
                                   const RTCPReceptionReportBlock* report,
                                   const cardinal                  layer)
{
   // ====== Update StreamDescription =======================================
   synchronized();
   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator found =
      StreamSet.find(stream);
   if(found != StreamSet.end()) {
      StreamDescription* streamDescription = found->second;
      if(layer < streamDescription->Layers) {
         smoothedUpdate(streamDescription->ReportedLossRate[layer],
                        report->getFractionLost(),
                        AlphaLossRate);
         smoothedUpdate(streamDescription->ReportedJitter[layer],
                        report->getJitter(),
                        AlphaJitter);
         getRoundTripTimes(streamDescription);

         // ====== Write log entry =============================================
         if(Log) {
            *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " ReportEvent"
                 << " #=" << streamDescription->StreamID
                 << " S=" << streamDescription->Session->SessionID
                 << " L=" << streamDescription->ReportedLossRate[layer]
                 << " J=" << streamDescription->ReportedJitter[layer] << std::endl;
         }
#ifdef PRINT_REPORT
         char str[256];
         snprintf((char*)&str,sizeof(str),"Report: L%d  Loss=%1.2f Jitter=%1.2f -> Loss=%1.2f Jitter=%1.2f",
                    layer,
                    report->getFractionLost(),
                    report->getJitter(),
                    streamDescription->ReportedLossRate[layer],
                    streamDescription->ReportedJitter[layer]);
         std::cout << str << std::endl;
#endif
      }
   }
   unsynchronized();
}


// ###### Buffer flush event ################################################
void BandwidthManager::bufferFlushEvent(ManagedStreamInterface* stream,
                                        const cardinal          layer)
{
   synchronized();
   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator found =
      StreamSet.find(stream);
   if(found != StreamSet.end()) {
      StreamDescription* streamDescription = found->second;
      streamDescription->BufferFlushes++;


      // ====== Write log entry =============================================
      if(Log) {
         *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " BufferFlushEvent"
              << " #=" << streamDescription->StreamID
              << " S=" << streamDescription->Session->SessionID
              << " L=" << layer << std::endl;
      }
   }
   TotalBufferFlushes++;
   unsynchronized();
}


// ###### Calculate multipoints of a session ################################
cardinal BandwidthManager::calculateSessionMultiPoints(
                              SessionDescription*            session,
                              const cardinal                 offset,
                              const cardinal                 lastPoint,
                              ResourceUtilizationMultiPoint* rumpList)
{
#ifdef PRINT_MULTIPOINTS
   char str[256];
   sprintf((char*)&str,"Calculating multipoints for session #%d with %d streams\n",
           session->SessionID,session->Streams);
   std::cout << str;
#endif

   // ====== Get resource/utilization points of each stream =================
   ResourceUtilizationSimplePoint streamRUPList[session->Streams][MaxRUPoints];
   double                         sortingList[session->Streams * MaxRUPoints];
   cardinal                       points[session->Streams];
   cardinal                       streamID = 0;
   cardinal                       sortings = 0;

   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator stream = session->StreamSet.begin();
   session->Priority                = -128;
   session->MinWantedBandwidth      = 0;
   session->MaxWantedBandwidth      = (card64)-1;
   session->MaximumReached          = false;
   session->TotalAllocatedBandwidth = 0;
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      session->AllocatedBandwidthArray[i] = 0;
   }
   while(stream != session->StreamSet.end()) {
      if(streamID >= session->Streams) {
         std::cerr << "INTERNAL ERROR: BandwidthManager::calculateSessionMultiPoints() - More streams than expected?!" << std::endl;
         abort();
      }
      points[streamID] = 0;
      StreamDescription* streamDescription = stream->second;
      if(streamDescription->QoSDescription != NULL) {
         // ====== Reset allocated bandwidth to 0 ===========================
         streamDescription->NewQuality.reset();
         streamDescription->NewQuality.Utilization = -HUGE_VAL;
         streamDescription->MaximumReached = false;
         for(cardinal j = 0;j < RTPConstants::RTPMaxQualityLayers;j++) {
            streamDescription->NewLayerClassNumber[j]     = 0;
            streamDescription->NewLayerClassBandwidth[j]  = 0;
         }
         streamDescription->NewCostPerSecond = 0.0;
         streamDescription->LastUtilization   = -HUGE_VAL;


         // ====== Get session limits =======================================
         const int8   sPriority      = streamDescription->QoSDescription->getSessionPriority();
         const card64 bwMin          = streamDescription->QoSDescription->getMinWantedBandwidth();
         const card64 bwMax          = streamDescription->QoSDescription->getMaxWantedBandwidth();
         session->Priority           = std::max(sPriority,session->Priority);
         session->MinWantedBandwidth = std::max(bwMin,session->MinWantedBandwidth);
         session->MaxWantedBandwidth = std::min(bwMax,session->MaxWantedBandwidth);


         // ====== Check, if this stream has an RU list =====================
         if(streamDescription->RUEntries > 0) {
            for(cardinal j = 0;j < streamDescription->RUEntries;j++) {
               if(points[streamID] >= MaxRUPoints) {
                  std::cerr << "INTERNAL ERROR: BandwidthManager::calculateSessionMultiPoints() - List overflow?!" << std::endl;
                  abort();
               }

               // ====== Ensure utilization limit ===========================
               if((j > 0) && (streamDescription->RUList[j].Utilization > streamDescription->QoSDescription->getWantedUtilization())) {
                  break;
               }

               // ====== Add resource/utilization point to list =============
               streamRUPList[streamID][points[streamID]].Bandwidth     = streamDescription->RUList[j].Bandwidth;
               streamRUPList[streamID][points[streamID]].BandwidthCost = streamDescription->RUList[j].BandwidthCost;
               streamRUPList[streamID][points[streamID]].Utilization   = streamDescription->RUList[j].Utilization;
               streamRUPList[streamID][points[streamID]].StreamPriorityFactor = getPriorityFactor(streamDescription->QoSDescription->getStreamPriority());
               streamRUPList[streamID][points[streamID]].Stream  = streamDescription;
               streamRUPList[streamID][points[streamID]].Point   = j;
               streamRUPList[streamID][points[streamID]].SortingValue =
                  getStreamSortingValue(streamRUPList[streamID][points[streamID]]);

               sortingList[sortings] =
                  streamRUPList[streamID][points[streamID]].SortingValue;

               points[streamID]++;
               sortings++;
            }
         }
         else {
            // This stream has no bandwidth requirements. Therefore,
            // 0 bytes/s bandwidth make 100% utilization!
            streamDescription->NewQuality.Utilization = 1.0;
            streamRUPList[streamID][points[streamID]].Point  = (cardinal)-1;
            streamRUPList[streamID][points[streamID]].Stream = streamDescription;
         }
      }
      streamID++;
      stream++;
   }
   if(sortings < 1) {
      return(0);
   }
#ifdef PRINT_MULTIPOINTS
   sprintf((char*)&str,"   Minimum Bandwidth = %Ld\n"
                       "   Maximum Bandwidth = %Ld\n"
                       "   Session Priority  = %d\n",
                       session->MinWantedBandwidth,
                       session->MaxWantedBandwidth,
                       session->Priority);
   std::cout << str;
#endif


   // ====== Get list of all sortings =======================================
   quickSort<double>((double*)&sortingList,0,sortings - 1);
   sortings = removeDuplicates<double>((double*)&sortingList,sortings);
#ifdef PRINT_MULTIPOINTS
   std::cout << "   Sorting steps: ";
   for(cardinal i = 0;i < sortings;i++) {
      std::cout << sortingList[i] << " ";
   }
   std::cout << std::endl;
   for(cardinal i = 0;i < session->Streams;i++) {
      const StreamDescription* streamDescription = streamRUPList[i][0].Stream;
      std::cout << "   Session #" << session->SessionID << ", stream #" << streamDescription->StreamID << ":" << std::endl;
      if(points[i] > 0) {

         // ====== Print stream's RU points =================================
         for(cardinal j = 0;j < points[i];j++) {
            std::cout << "      " << streamRUPList[i][j] << std::endl;

            // ====== Print possible layer -> class mappings ================
            for(cardinal l = 0;l < streamDescription->Layers;l++) {
               std::cout << "            Layer #" << l << ":  ";
               if(streamDescription->RUList[streamRUPList[i][j].Point].Mapping[l].Possibilities > 0) {
                  for(cardinal k = 0;k < streamDescription->RUList[streamRUPList[i][j].Point].Mapping[l].Possibilities;k++) {
                     snprintf((char*)&str,sizeof(str),"%4s D=%02d B=%7Ld C=%7Ld  ",
                              TrafficClassValues::getNameForTrafficClass(SLA->Class[streamDescription->RUList[streamRUPList[i][j].Point].Mapping[l].Possibility[k].Class].TrafficClass),
                              streamDescription->RUList[streamRUPList[i][j].Point].Mapping[l].Possibility[k].BufferDelay,
                              streamDescription->RUList[streamRUPList[i][j].Point].Mapping[l].Possibility[k].Bandwidth,
                              (card64)ceil(streamDescription->RUList[streamRUPList[i][j].Point].Mapping[l].Possibility[k].Cost));
                     std::cout << str;
                  }
                  std::cout << std::endl;
               }
               else {
                  std::cout << "None!" << std::endl;
               }
            }
         }
      }
      else {
         std::cout << "      Stream has no bandwidth requirements." << std::endl;
      }
   }
#endif


   // ====== Join lists: Generate "sorting"-fair bandwidth mapping list =====
   cardinal start[session->Streams];
   cardinal count = offset;
   for(cardinal i = 0;i < session->Streams;i++) {
      start[i] = 0;
   }
#ifdef PRINT_MULTIPOINTS
   std::cout << "   Multipoints of session #" << session->SessionID << ":" << std::endl;
#endif
   const double sessionPriorityFactor = getPriorityFactor(session->Priority);
   for(cardinal j = 0;j < sortings;j++) {


      // ====== Go to next sorting ==========================================
      bool changed = false;
      if(j > 0) {
         for(cardinal i = 0;i < session->Streams;i++) {
            if((points[i] > 0)            &&
               (start[i] < points[i] - 1) &&
               (streamRUPList[i][start[i]].SortingValue <= sortingList[j])) {
               start[i]++;
               changed = true;
            }
         }
      }
      else {
         // Always add multipoint with minimum utilization!
         changed = true;
      }


      // ====== Point has to be added, if there was a change ================
      if(changed) {
         // ====== Add resource/utilization multipoint to global list =======
         if(session->Streams >= ResourceUtilizationMultiPoint::MaxStreamsPerSession) {
            std::cerr << "INTERNAL ERROR: BandwidthManager::calculateSessionMultiPoints() - MaxStreamsPerSession setting too low?!" << std::endl;
            abort();
         }
         card64 totalBandwidth   = 0;
         double totalCost        = 0.0;
         double totalUtilization = 0.0;
         cardinal k = 0;
         for(cardinal i = 0;i < session->Streams;i++) {
            if(points[i] > 0) {
               totalBandwidth   += streamRUPList[i][start[i]].Bandwidth;
               totalCost        += streamRUPList[i][start[i]].BandwidthCost;
               totalUtilization += streamRUPList[i][start[i]].Utilization;
               rumpList[count].Stream[k] = streamRUPList[i][start[i]].Stream;
               rumpList[count].Point[k]  = streamRUPList[i][start[i]].Point;
               k++;
            }
         }
         rumpList[count].Session       = session;
         rumpList[count].Streams       = k;
         rumpList[count].Bandwidth     = totalBandwidth;
         rumpList[count].BandwidthCost = totalCost;
         rumpList[count].Utilization   = totalUtilization / (double)k;
         rumpList[count].SessionPriorityFactor = sessionPriorityFactor;
         rumpList[count].SortingValue          = getSessionSortingValue(rumpList[count]);
         rumpList[count].AlreadyAllocated      = false;

#ifdef PRINT_MULTIPOINTS
         snprintf((char*)&str,sizeof(str),"      #%02d:  ",count - offset);
         std::cout << str << rumpList[count] << std::endl;
#endif
         count++;
      }
   }

   return(count - offset);
}


// ###### Allocate bandwidth ################################################
void BandwidthManager::doAllocationTrials(
                           ResourceUtilizationMultiPoint* rumpList,
                           const cardinal                 points,
                           const card64                   bandwidthLimit)
{
   // ====== Interate list of all multipoints ===============================
   for(cardinal i = 0;i < points;i++) {
#ifdef PRINT_ALLOCATION
      char str[256];
      snprintf((char*)&str,sizeof(str),"S%02Ld:  ",
               (card64)rumpList[i].Session->SessionID);
      std::cout << "   => " << str << rumpList[i] << std::endl;
#endif
      SessionDescription* session = rumpList[i].Session;


      // ===== Check, if multipoint can be allocated ========================
      if(session->MaximumReached) {
#ifdef PRINT_ALLOCATION
         std::cout << "      Not possible - no more allocations to this session." << std::endl;
#endif
      }

      // ===== Check, if multipoint is already allocated (session minimum) ==
      else if(rumpList[i].AlreadyAllocated) {
#ifdef PRINT_ALLOCATION
         std::cout << "      Already in minimum allocation." << std::endl;
#endif
      }

      // ====== Try to allocate multipoint ==================================
      else {
         if(!tryAllocation(rumpList[i],bandwidthLimit)) {
            session->MaximumReached = true;
         }
         else {
            rumpList[i].AlreadyAllocated = true;
         }
      }
   }
}


// ====== Try allocation for a multipoint ===================================
bool BandwidthManager::tryAllocation(
                           ResourceUtilizationMultiPoint& rump,
                           const card64                   bandwidthLimit)
{
   // ====== Try to allocate all points of the multipoint =============
   bool success = true;
   for(cardinal j = 0;j < rump.Streams;j++) {
      StreamDescription* streamDescription = rump.Stream[j];
      if(streamDescription->MaximumReached == false) {
         const cardinal pointNumber    = rump.Point[j];
         ResourceUtilizationPoint& rup = streamDescription->RUList[pointNumber];
#ifdef PRINT_ALLOCATION
         std::cout << "      => " << rump.Stream[j]->StreamDescription << rup << std::endl;
#endif

         // ====== Try allocation of a point ==========================
         const bool ok = streamDescription->tryAllocation(
                            SLA,
                            TotalAvailableBandwidth,
                            ClassAvailableBandwidthArray,
                            rup,
                            bandwidthLimit);
         if(ok == false) {
            streamDescription->MaximumReached = true;
            success                           = false;
#ifdef PRINT_ALLOCATION
            std::cout << "            Bandwidth not sufficient." << std::endl;
#endif
         }
         else {
            streamDescription->NewQuality       = rup;
            streamDescription->NewCostPerSecond = rup.BandwidthCost;
         }
      }
   }
   return(success);
}


// ###### Do complete remapping #############################################
void BandwidthManager::doCompleteRemapping()
{
   // ====== Check, if remapping is necessary ===============================
   const card64 now = (SimulatorTime == 0) ? getMicroTime() : SimulatorTime;
   synchronized();
   if( ((Changed == false) && (now - LastCompleteRemapping < MaxRemappingInterval)) ||
       (StreamSet.begin() == StreamSet.end())) {
      unsynchronized();
      return;
   }
   Changed               = false;
   LastCompleteRemapping = now;
   CompleteRemappings++;
#ifdef PRINT_COMPLETE_REMAPPING
   std::cout << "*** Complete remapping ***" << std::endl;
#endif


   // ====== Allocate memory for resource/utilization list ==================
   const card64 startTimeStamp = getMicroTime();
   ResourceUtilizationMultiPoint* rupList = new ResourceUtilizationMultiPoint[Streams * MaxRUPoints];
   if(rupList == NULL) {
      std::cerr << "ERROR: BandwidthManager::doCompleteRemapping() - Out of memory!"
           << std::endl;
      unsynchronized();
      return;
   }


   // ====== Initialize available bandwidths ================================
   TotalAvailableBandwidth = 0;
   TotalBandwidth          = 0;
   card64 reservedBandwidth[TrafficClassValues::MaxValues];
   for(cardinal i = 0;i < SLA->Classes;i++) {
      ClassBandwidthArray[i] = SLA->Class[i].BytesPerSecond;
      TotalBandwidth         += ClassBandwidthArray[i];

      ClassAvailableBandwidthArray[i] =
         (card64)ceil((1.0 - PartialRemappingPortion) * (double)SLA->Class[i].BytesPerSecond);
      reservedBandwidth[i] = SLA->Class[i].BytesPerSecond - ClassAvailableBandwidthArray[i];
      TotalAvailableBandwidth += ClassAvailableBandwidthArray[i];
   }
#ifdef PRINT_COMPLETE_REMAPPING_SLA
   char s[256];
   for(cardinal i = 0;i < SLA->Classes;i++) {
      snprintf((char*)&s,sizeof(s),"%-4s   usable=%20Ld   reserved=%20Ld",
               TrafficClassValues::getNameForTrafficClass(SLA->Class[i].TrafficClass),
               ClassAvailableBandwidthArray[i],
               reservedBandwidth[i]);
      std::cout << s << std::endl;
   }
#endif

   // ====== Get latest round trip times for DiffServ classes ===============
   std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator stream = StreamSet.begin();
   while(stream != StreamSet.end()) {
      StreamDescription* streamDescription = stream->second;
      getRoundTripTimes(streamDescription);
      stream++;
   }


   // ====== Get resource/utilization multipoint list of each session =======
   cardinal points = 0;
   std::multimap<cardinal,SessionDescription*>::iterator iterator = SessionSet.begin();
   while(iterator != SessionSet.end()) {

      // ====== Get resouce/utilization multipoint list =====================
      SessionDescription* session = iterator->second;
      const cardinal newPoints =
         calculateSessionMultiPoints(session,points,Streams * MaxRUPoints,rupList);

      // ====== Each session gets it's minimum wanted bandwidth =============
      if(newPoints != 0) {
#ifdef PRINT_ALLOCATION
         std::cout << "Allocate minimum for session #"
              << session->SessionID << ":" << std::endl;
#endif
         doAllocationTrials(&rupList[points],newPoints,session->MinWantedBandwidth);

         session->MaximumReached = false;
         std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator stream = session->StreamSet.begin();
         while(stream != session->StreamSet.end()) {
            StreamDescription* streamDescription = stream->second;
            streamDescription->MaximumReached = false;
            stream++;
         }
      }

      points += newPoints;
      iterator++;
   }


   // ====== Update sorting of resource/utilization points ==================
   if(points > 0) {
      quickSort<ResourceUtilizationMultiPoint>(rupList,0,points-1);
   }

#ifdef PRINT_MULTIPOINTS
   char str[256];
   std::cout << "Global multipoint list:" << std::endl;
   for(cardinal i = 0;i < points;i++) {
      snprintf((char*)&str,sizeof(str),"S%02Ld:  ",
               (card64)rupList[i].Session->SessionID);
      std::cout << "   " << str << rupList[i] << std::endl;
   }
#endif


   // ===== Allocate remaining bandwidth ====================================
#ifdef PRINT_ALLOCATION
   std::cout << "Allocate remaining bandwidth:" << std::endl;
#endif
   doAllocationTrials(rupList,points);

#ifdef PRINT_QUALITY
#ifndef PRINT_MULTIPOINTS
   char str[256];
#endif
   std::cout << "Resulting qualities:" << std::endl;
   iterator = SessionSet.begin();
   while(iterator != SessionSet.end()) {
      SessionDescription* session = iterator->second;
      std::cout << "   Session #" << session->SessionID << ":" << std::endl;

      multimap<ManagedStreamInterface*,StreamDescription*>::iterator stream = session->StreamSet.begin();
      while(stream != session->StreamSet.end()) {

         // ====== Print quality ============================================
         StreamDescription* streamDescription = stream->second;
         std::cout << "      #" << streamDescription->StreamID << ":" << std::endl;
         snprintf((char*)&str,sizeof(str),"         Bandwidth=%Ld Utilization=%1.3f Cost=%1.0f",
                  streamDescription->NewQuality.Bandwidth,
                  streamDescription->NewQuality.Utilization,
                  streamDescription->NewQuality.BandwidthCost);
         std::cout << str << std::endl;
         for(cardinal i = 0;i < streamDescription->NewQuality.Layers;i++) {
            snprintf((char*)&str,sizeof(str),"         Layer #%d: %8Ld/%02d %-4s",
                     i,
                     streamDescription->NewQuality.LayerBandwidthInfo[i].BytesPerSecond,
                     streamDescription->NewQuality.LayerBandwidthInfo[i].BufferDelay,
                     TrafficClassValues::getNameForTrafficClass(
                        SLA->Class[streamDescription->NewLayerClassNumber[i]].TrafficClass));
            std::cout << str << std::endl;
         }

         stream++;
      }

      iterator++;
   }
#endif


   // ====== Set quality in AbstractQoSDescription ==========================
   card64 totalUsedPerClass[TrafficClassValues::MaxValues];
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      SLAUpdateRecommendation[i] = 0;
      totalUsedPerClass[i]       = 0;
   }
   stream = StreamSet.begin();
#ifdef EXIT_AFTER_REMAPPING
   cardinal counter = 0;
#endif
   while(stream != StreamSet.end()) {
      StreamDescription* streamDescription = stream->second;
      streamDescription->MaximumReached          = false;
      streamDescription->Session->MaximumReached = false;
      if(streamDescription->QoSDescription != NULL) {

         // ====== Set bandwidths  ==========================================
         // std::cout << "SET: " << streamDescription->NewQuality << std::endl;
         streamDescription->LastUtilization =
            streamDescription->QoSDescription->setResources(streamDescription->NewQuality);

         // ====== Calculate "bad" allocations ==============================
         for(cardinal j = 0;j < streamDescription->Layers;j++) {
            SLAUpdateRecommendation[streamDescription->NewQuality.Mapping[j].Possibility[0].Class] += (int64)streamDescription->NewQuality.Mapping[j].Possibility[0].Bandwidth;
            totalUsedPerClass[streamDescription->NewLayerClassNumber[j]] += streamDescription->NewLayerClassBandwidth[j];
         }

         // Note: We do *not* set traffic classes here. This can only be
         //       done in intervalChangeEvent(), since AbstractQoSDescription
         //       may be updated there (and TC's reseted to 0x00)!
      }

#ifdef EXIT_AFTER_REMAPPING
      counter++;
#endif
      stream++;
   }


   // ====== Add reserved bandwidths for partial remapping to available bws =
   TotalAvailableBandwidth = 0;
   double totalCost = 0.0;
   card64 totalUsed = 0;
   for(cardinal i = 0;i < SLA->Classes;i++) {
      ClassAvailableBandwidthArray[i] += reservedBandwidth[i];
      TotalAvailableBandwidth         += ClassAvailableBandwidthArray[i];
      totalUsed                       += totalUsedPerClass[i];
      totalCost                       += (double)totalUsedPerClass[i] * SLA->Class[i].CostFactor;
      SLAUpdateRecommendation[i]      -= (int64)(ClassBandwidthArray[i] - ClassAvailableBandwidthArray[i]);
   }
   if(TotalBandwidth - TotalAvailableBandwidth != totalUsed) {
      std::cerr << "INTERNAL ERROR: BandwidthManager::doCompleteRemapping() - "
              "Total bandwidth verification failed!" << std::endl;
      std::cerr << "Really used:     " << totalUsed << std::endl;
      std::cerr << "Calculated used: " << TotalBandwidth - TotalAvailableBandwidth << std::endl;
      abort();
   }


   // ====== Write log entry ================================================
   if(Log) {
      char line[1024];
      const card64 timeStamp = ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp;
      snprintf((char*)&line,sizeof(line),
               "%llu AllocationStatistics B=%llu C=%f",
               (unsigned long long)timeStamp, (unsigned long long)totalUsed, totalCost);
      char str[256];
      *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " AllocationStatistics"
           << " B=" << totalUsed
           << " C=" << totalCost;
      for(cardinal i = 0;i < SLA->Classes;i++) {
         snprintf((char*)&str,sizeof(str),"$%02x",SLA->Class[i].TrafficClass);
         *Log << " D=<" << str
              << "," << totalUsedPerClass[i] << ">";
      }
      *Log << std::endl;
   }


   // ====== Free memory of resource/utilization list =======================
   delete [] rupList;


   // ====== Write statistics ===============================================
   const card64 endTimeStamp = getMicroTime();
   LastCompleteRemappingDuration = endTimeStamp - startTimeStamp;


   // ====== Unsynchronize ==================================================
   unsynchronized();
#ifdef EXIT_AFTER_REMAPPING
   if(counter >= EXIT_AFTER_REMAPPING) {
      std::cerr << EXIT_AFTER_REMAPPING << " streams available. Stop now!" << std::endl;
      exit(1);
   }
#endif
}


// ###### Do partial remapping ##############################################
bool BandwidthManager::doPartialRemapping(StreamDescription* streamDescription)
{
   bool success = false;
   for(integer i = (integer)streamDescription->RUEntries - 1;i >= 0;i--) {
      if(fabs(streamDescription->RUList[i].Utilization -
            streamDescription->LastUtilization) <= PartialRemappingUtilizationTolerance) {
         ResourceUtilizationPoint allocationPoint = streamDescription->RUList[i];
#ifdef PRINT_PARTIAL_REMAPPING
         std::cout << "*** Partial remapping for stream #" << streamDescription->StreamID << " ***" << std::endl;
         std::cout << "   Trying " << allocationPoint << std::endl;
#endif

         getRoundTripTimes(streamDescription);
         const bool ok = streamDescription->tryAllocation(
                            SLA,
                            TotalAvailableBandwidth,
                            ClassAvailableBandwidthArray,
                            allocationPoint);
         if(ok == true) {
            // NOTE: We do *not* update LastUtilization here, since this value
            // gives utilization of last *complete* remapping!
            streamDescription->NewQuality       = allocationPoint;
            streamDescription->NewCostPerSecond = allocationPoint.BandwidthCost;
            streamDescription->QoSDescription->setResources(streamDescription->NewQuality);
#ifdef PRINT_PARTIAL_REMAPPING
            std::cout << "      => Success: Utilization is "
                 << streamDescription->NewQuality.Utilization
                 << " (Last complete remapping's utilization was " << streamDescription->LastUtilization << ")." << std::endl;
#endif
            streamDescription->PartialRemappings++;
            PartialRemappings++;
            success = true;

            // ====== Write log entry =======================================
            if(Log) {
               card64 totalUsedPerClass[TrafficClassValues::MaxValues];
               for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
                  totalUsedPerClass[i] = 0;
               }
               std::multimap<ManagedStreamInterface*,StreamDescription*>::iterator stream = StreamSet.begin();
               while(stream != StreamSet.end()) {
                  StreamDescription* streamDescription = stream->second;
                  if(streamDescription->QoSDescription != NULL) {
                     for(cardinal j = 0;j < streamDescription->Layers;j++) {
                        totalUsedPerClass[streamDescription->NewLayerClassNumber[j]] += streamDescription->NewLayerClassBandwidth[j];
                     }
                  }
                  stream++;
               }
               double totalCost = 0.0;
               card64 totalUsed = 0;
               for(cardinal i = 0;i < SLA->Classes;i++) {
                  totalUsed += totalUsedPerClass[i];
                  totalCost += (double)totalUsedPerClass[i] * SLA->Class[i].CostFactor;
               }
               char str[256];
               *Log << ((SimulatorTime == 0) ? getMicroTime() : SimulatorTime) - LogStartupTimeStamp << " AllocationStatistics"
                    << " B=" << totalUsed
                    << " C=" << totalCost;
               for(cardinal i = 0;i < SLA->Classes;i++) {
                  snprintf((char*)&str,sizeof(str),"$%02x",SLA->Class[i].TrafficClass);
                  *Log << " D=<" << str
                       << "," << totalUsedPerClass[i] << ">";
               }
               *Log << std::endl;
            }

            break;
         }
         else {
#ifdef PRINT_PARTIAL_REMAPPING
            std::cout << "      => Not enough bandwidth available!" << std::endl;
#endif
         }
      }
   }
   return(success);
}
