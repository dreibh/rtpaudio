// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Stream Description Implementation                                ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2018 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   https://www.uni-due.de/~be0001/rtpaudio                ####
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
#include "streamdescription.h"


// Print information
// #define PRINT_DELAYOPTIMIZATION
// #define PRINT_ORIGINAL
// #define PRINT_DELAYOPTIMIZED
// #define PRINT_ALLOCATION


// ###### Constructor #######################################################
StreamDescription::StreamDescription()
{
   StreamID = 0;
   for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
      ReportedJitter[i]             = 0.0;
      ReportedLossRate[i]           = 0.0;
      CurrentLayerClassNumber[i]    = 0;
      CurrentLayerClassBandwidth[i] = 0;
      NewLayerClassNumber[i]        = 0;
      NewLayerClassBandwidth[i]     = 0;
   }
   TotalCost               = 0.0;
   TotalBandwidthUsage     = 0.0;
   TotalUtilization        = 0.0;
   TotalRuntime            = 0.0;
   TotalReservationUpdates = 0;
   CurrentCostPerSecond    = 0.0;
   NewCostPerSecond        = 0.0;
   LastUtilization         = -HUGE_VAL;
   ReservationTimeStamp    = 0;
   NextInterval            = 0;
   PartialRemappings       = 0;
   CompleteRemappings      = 0;
   Inits                   = 0;
   BufferFlushes           = 0;
   LastInitDuration        = 0;
   UnlayeredAllocation     = false;

   RoundTripTimeDestination.setPrintFormat(InternetAddress::PF_Address);
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      MeasuredTransferDelay[i] = HUGE_VAL;
   }

   NewQuality.reset();
   CurrentQuality.reset();

   RUEntries = 0;
   for(cardinal i = 0;i < MaxRUEntries;i++) {
      RUList[i].reset();
   }

   Layers         = 0;
   QoSDescription = NULL;
   Interface      = NULL;
   MaximumReached = false;
}


// ###### Destructor ########################################################
StreamDescription::~StreamDescription()
{
}


// ###### Initialize StreamDescription ######################################
void StreamDescription::init(ManagedStreamInterface*      stream,
                             const ServiceLevelAgreement* sla,
                             const cardinal               maxPoints,
                             const card64                 bwThreshold,
                             const double                 utThreshold,
                             const double                 systemDelayTolerance,
                             const bool                   unlayeredAllocation)
{
   // ====== Initialize =====================================================
   const card64 s = getMicroTime();
   Interface = stream;
   if(QoSDescription != NULL) {
      delete QoSDescription;
   }
   QoSDescription      = stream->getQoSDescription(0);
   Layers              = QoSDescription->getLayers();
   MaximumReached      = false;
   UnlayeredAllocation = unlayeredAllocation;

   // ====== Get precomputed resource/utilization list ======================
   RUEntries = QoSDescription->getPrecomputedResourceUtilizationList(
                  (ResourceUtilizationPoint*)&RUList,
                  bwThreshold, utThreshold,
                  std::min(maxPoints,MaxRUEntries));

#ifdef PRINT_ORIGINAL
   char str1[256];
   snprintf((char*)&str1,sizeof(str1),"Original resource/utilization list for stream #%Ld:",
            (card64)StreamID);
   std::cout << str1 << std::endl;
   for(cardinal i = 0;i < RUEntries;i++) {
      snprintf((char*)&str1,sizeof(str1),"   P%02d:  ",i);
      std::cout << str1 << RUList[i] << std::endl;
   }
#endif
#ifdef PRINT_DELAYOPTIMIZATION
   char str2[256];
   snprintf((char*)&str2,sizeof(str2),"Delay optimization for stream #%Ld:",
            (card64)StreamID);
   std::cout << str2 << std::endl;
#endif


   // ====== Update resource/utilization list using buffer delay =========
   for(cardinal i = 0;i < RUEntries;i++) {
      // ====== Preparations for unlayered allocation ====================
      if(UnlayeredAllocation) {
         double maxLossRate      = HUGE_VAL;
         double maxJitter        = HUGE_VAL;
         double maxTransferDelay = HUGE_VAL;

         for(cardinal i = 0;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
            maxLossRate      = std::min(RUList[i].LayerBandwidthInfo[i].MaxLossRate,maxLossRate);
            maxTransferDelay = std::min(RUList[i].LayerBandwidthInfo[i].MaxTransferDelay,maxTransferDelay);
            maxJitter        = std::min(RUList[i].LayerBandwidthInfo[i].MaxJitter,maxJitter);
         }
         for(cardinal i = 0;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
            RUList[i].LayerBandwidthInfo[i].MaxLossRate      = maxLossRate;
            RUList[i].LayerBandwidthInfo[i].MaxTransferDelay = maxTransferDelay;
            RUList[i].LayerBandwidthInfo[i].MaxJitter        = maxJitter;
         }
      }


      // ====== Calculate possible layer -> class mappings ===============
      if(calculatePossibleLayerClassMappings(RUList[i],sla,QoSDescription) == true) {
         card64 totalBandwidth = 0;
         double totalCost      = 0;

         for(cardinal j = 0;j < Layers;j++) {
            const AbstractLayerDescription* ald = QoSDescription->getLayer(j);
            const double maxTransferDelay       = ald->getMaxTransferDelay();
            double   cost        = HUGE_VAL;
            cardinal bufferDelay = (cardinal)-1;
            card64 bandwidth     = (card64)-1;

            // Bandwidth cache: Avoid calculation of the same delay setting.
            // This will save about half of the computing time.
            const cardinal bwCacheSize = 48;
            card64 bwCache[bwCacheSize];
            for(cardinal k = 0;k < bwCacheSize;k++) {
               bwCache[k] = (card64)-1;
            }
#ifdef PRINT_DELAYOPTIMIZATION
               char str2[256];
               snprintf((char*)&str2,sizeof(str2),"   P%02d:  ",i);
               std::cout << str2 << RUList[i] << std::endl;
#endif
            for(cardinal k = 0;k < RUList[i].Mapping[j].Possibilities;k++) {

               // ====== Calculate available delay ==========================
               const cardinal classID = RUList[i].Mapping[j].Possibility[k].Class;
               const double availableDelay =
                  std::max(0.0,
                           maxTransferDelay -
                           systemDelayTolerance -
                           ((1.0 + sla->Class[classID].DelayVariability) *
                              MeasuredTransferDelay[classID]));


               // ====== Calculate new buffer delay =========================
               // Note: It is very important that the delay does not exceed
               //       the given maximum buffer delay. TDTF's approximation
               //       will be invalid for too high values!
               const cardinal newBufferDelay =
                  std::min(ald->getMaxBufferDelay(RUList[i].FrameRate),
                           std::max((cardinal)1,
                                    1 + (cardinal)floor(availableDelay /
                                                        (1000000.0 / RUList[i].FrameRate))));
#ifdef PRINT_DELAYOPTIMIZATION
               snprintf((char*)&str2,sizeof(str2),
                        "      %-4s -> user=%1.0f system=%1.0f measured=%1.0f var=%1.0f => available=%1.0f  bd=%d",
                        TrafficClassValues::getNameForTrafficClass(sla->Class[classID].TrafficClass),
                        maxTransferDelay,
                        systemDelayTolerance,
                        MeasuredTransferDelay[classID],
                        sla->Class[classID].DelayVariability * MeasuredTransferDelay[classID],
                        availableDelay,
                        newBufferDelay);
               std::cout << str2 << std::endl;
#endif


               // ====== Calculate new bandwidth and cost ===================
               card64 newBandwidth;
               if((newBufferDelay < bwCacheSize) && (bwCache[newBufferDelay] != (card64)-1)) {
                  newBandwidth = bwCache[newBufferDelay];
               }
               else {
                  const card64 newPayloadBandwidth =
                     ald->payloadBandwidthToBandwidth(
                        RUList[i].LayerBandwidthInfo[j].BytesPerSecond,
                        RUList[i].FrameRate,
                        RUList[i].LayerBandwidthInfo[j].BufferDelay,
                        newBufferDelay);
                  const cardinal newPayloadFrameSize =
                     ald->bandwidthToFrameSize(RUList[i].FrameRate,newPayloadBandwidth);
                  const cardinal newFrameSize =
                     ald->payloadToRaw(RUList[i].FrameRate,newPayloadFrameSize,newBufferDelay);
                  newBandwidth =
                     ald->frameSizeToBandwidth(RUList[i].FrameRate,newFrameSize);
                  if(newBufferDelay < bwCacheSize) {
                     bwCache[newBufferDelay] = newBandwidth;
                  }
               }
               const double newCost = (double)newBandwidth * sla->Class[classID].CostFactor;


               // ====== Check, if new settings are cheapest ================
               RUList[i].Mapping[j].Possibility[k].Cost        = newCost;
               RUList[i].Mapping[j].Possibility[k].Bandwidth   = newBandwidth;
               RUList[i].Mapping[j].Possibility[k].BufferDelay = newBufferDelay;
               if(cost > newCost) {
                  cost        = newCost;
                  bandwidth   = newBandwidth;
                  bufferDelay = newBufferDelay;
               }
            }


            // ====== Sort mapping by cost ==================================
            for(cardinal k = 0;k < RUList[i].Mapping[j].Possibilities;k++) {
               for(cardinal l = k + 1;l < RUList[i].Mapping[j].Possibilities;l++) {
                  if(RUList[i].Mapping[j].Possibility[k].Cost >
                     RUList[i].Mapping[j].Possibility[l].Cost) {
                     const LayerClassMappingPossibility tmp =
                        RUList[i].Mapping[j].Possibility[l];
                     RUList[i].Mapping[j].Possibility[l] = RUList[i].Mapping[j].Possibility[k];
                     RUList[i].Mapping[j].Possibility[k] = tmp;
                  }
               }
            }


            // ====== Set cheapest settings =================================
            RUList[i].LayerBandwidthInfo[j].BytesPerSecond = bandwidth;
            RUList[i].LayerBandwidthInfo[j].BufferDelay    = bufferDelay;
            totalBandwidth += bandwidth;
            totalCost      += cost;
         }

         RUList[i].Bandwidth     = totalBandwidth;
         RUList[i].BandwidthCost = totalCost;
      }


      // ====== Preparations for unlayered allocation ====================
      if(UnlayeredAllocation) {
         // ====== Get per-class cost ====================================
         LayerClassMappingPossibility possibility[std::min(Layers,RTPConstants::RTPMaxQualityLayers)][TrafficClassValues::MaxValues];
         double cost[sla->Classes];
         for(cardinal j = 0;j < sla->Classes;j++) {
            cost[j] = 0.0;
         }
         for(cardinal j = 0;j < std::min(Layers,RTPConstants::RTPMaxQualityLayers);j++) {
            bool usable[sla->Classes];
            for(cardinal c = 0;c < sla->Classes;c++) {
               usable[c] = false;
               possibility[j][c].Cost = 12345.0;
            }
            for(cardinal k = 0;k < RUList[i].Mapping[j].Possibilities;k++) {
               const cardinal classID = RUList[i].Mapping[j].Possibility[k].Class;
               if(cost[classID] < HUGE_VAL) {
                  usable[classID]         = true;
                  cost[classID]          += RUList[i].Mapping[j].Possibility[k].Cost;
                  possibility[j][classID] = RUList[i].Mapping[j].Possibility[k];
               }
            }
            for(cardinal c = 0;c < sla->Classes;c++) {
               if(usable[c] == false) {
                  cost[c] = HUGE_VAL;
               }
            }

         }

#ifdef PRINT_DELAYOPTIMIZED
         std::cout << "Per-class cost list:" << std::endl;
         for(cardinal c = 0;c < sla->Classes;c++) {
            char str[256];
            snprintf((char*)&str,sizeof(str),"   %4s = %12.0f",
                     TrafficClassValues::getNameForTrafficClass(sla->Class[c].TrafficClass),
                     cost[c]);
            std::cout << str << std::endl;
         }
#endif

         // ====== Sort list by cost ========================================
         for(cardinal j = 0;j < sla->Classes;j++) {
            cardinal minIndex = j;
            double   minCost  = cost[j];
            for(cardinal k = j + 1;k < sla->Classes;k++) {
               if(cost[k] < minCost) {
                  minIndex = k;
                  minCost  = cost[k];
               }
            }
            if(cost[minIndex] < HUGE_VAL) {
               for(cardinal l = 0;l < std::min(Layers,RTPConstants::RTPMaxQualityLayers);l++) {
                  RUList[i].Mapping[l].Possibility[j] = possibility[l][minIndex];
                  RUList[i].Mapping[l].Possibilities  = j + 1;
               }
               cost[minIndex] = HUGE_VAL;
            }
            else {
               break;
            }
         }
         RUList[i].Bandwidth     = 0;
         RUList[i].BandwidthCost = 0.0;
         for(cardinal l = 0;l < std::min(Layers,RTPConstants::RTPMaxQualityLayers);l++) {
            RUList[i].Bandwidth     += RUList[i].Mapping[l].Possibility[0].Bandwidth;
            RUList[i].BandwidthCost += RUList[i].Mapping[l].Possibility[0].Cost;
         }
      }
   }


   // ====== Remove "bad" entries from list =================================
   RUEntries = ResourceUtilizationPoint::optimizeResourceUtilizationList(
      (ResourceUtilizationPoint*)&RUList,RUEntries);

#ifdef PRINT_DELAYOPTIMIZED
      char str2[256];
      snprintf((char*)&str2,sizeof(str2),"Delay-optimized resource/utilization list for stream #%Ld:",
               (card64)StreamID);
      std::cout << str2 << std::endl;
      for(cardinal i = 0;i < RUEntries;i++) {
         snprintf((char*)&str2,sizeof(str2),"   P%02d:  ",i);
         std::cout << str2 << RUList[i] << std::endl;
         for(cardinal j = 0;j < Layers;j++) {
            std::cout << "            Layer #" << j << ":  ";
            if(RUList[i].Mapping[j].Possibilities > 0) {
               for(cardinal k = 0;k < RUList[i].Mapping[j].Possibilities;k++) {
                  snprintf((char*)&str2,sizeof(str2),"%4s = %d D=%02d B=%7Ld C=%8Ld  ",
                           TrafficClassValues::getNameForTrafficClass(sla->Class[RUList[i].Mapping[j].Possibility[k].Class].TrafficClass),
                           RUList[i].Mapping[j].Possibility[k].Class,
                           RUList[i].Mapping[j].Possibility[k].BufferDelay,
                           RUList[i].Mapping[j].Possibility[k].Bandwidth,
                           (card64)ceil(RUList[i].Mapping[j].Possibility[k].Cost));
                  std::cout << str2;
               }
            }
            else {
               std::cout << "None!";
            }
            std::cout << std::endl;
         }
      }
#endif

   const card64 e = getMicroTime();
   LastInitDuration = e - s;
   Inits++;
}


// ###### Calculate possible mappings from layers to DiffServ classes #######
bool StreamDescription::calculatePossibleLayerClassMappings(
                           ResourceUtilizationPoint&     rup,
                           const ServiceLevelAgreement*  sla,
                           const AbstractQoSDescription* aqd)
{
   cardinal i;
   for(i = 0;i < Layers;i++) {
      // ====== Get possible layer -> class mappings ========================
      const AbstractLayerDescription* ald = aqd->getLayer(i);
      cardinal mapping[TrafficClassValues::MaxValues];
      rup.Mapping[i].Possibilities =
         sla->getPossibleClassesForBandwidthInfo(ald,(cardinal*)&mapping);


      // ====== Add mapping information to resource/utilization point =======
      if((rup.Mapping[i].Possibilities == 0) &&
         (aqd->getMinBandwidth() > 0)) {
         return(false);
      }
      else {
         for(cardinal k = 0;k < rup.Mapping[i].Possibilities;k++) {
            rup.Mapping[i].Possibility[k].Class = mapping[k];
         }
      }
   }
   for(   ;i < RTPConstants::RTPMaxQualityLayers;i++) {
      rup.Mapping[i].Possibilities = 0;
   }
   return(true);
}


// ###### Try to allocate given bandwidths to stream's layers ###############
bool StreamDescription::tryAllocation(
                           const ServiceLevelAgreement* sla,
                           card64&                      totalAvailableBandwidth,
                           card64*                      classAvailableBandwidthArray,
                           ResourceUtilizationPoint&    rup,
                           const card64                 bandwidthLimit)
{
   // ====== Check for impossibility ========================================
   if(std::min(bandwidthLimit,Session->MaxWantedBandwidth) == 0) {
#ifdef PRINT_ALLOCATION
      std::cout << "      Limit is 0 -> Allocation impossible." << std::endl;
#endif
      return(false);
   }

   // ====== Backup old values ==============================================
   card64 oldTotalAvailableBandwidth = totalAvailableBandwidth;
   card64 oldSessionTotalBandwidth = Session->TotalAllocatedBandwidth;
   card64 oldClassAvailableBandwidthArray[TrafficClassValues::MaxValues];
   card64 oldSessionBandwidthArray[TrafficClassValues::MaxValues];
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      oldClassAvailableBandwidthArray[i] = classAvailableBandwidthArray[i];
      oldSessionBandwidthArray[i]        = Session->AllocatedBandwidthArray[i];
   }

   // ====== Free old allocation ============================================
   for(cardinal i = 0;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
      totalAvailableBandwidth += NewLayerClassBandwidth[i];
      classAvailableBandwidthArray[NewLayerClassNumber[i]]     += NewLayerClassBandwidth[i];
      Session->TotalAllocatedBandwidth                         -= NewLayerClassBandwidth[i];
      Session->AllocatedBandwidthArray[NewLayerClassNumber[i]] -= NewLayerClassBandwidth[i];
   }


   // ====== Try new allocation =============================================
   card64   totalBandwidth = 0;
   card64   bandwidth[RTPConstants::RTPMaxQualityLayers];
   cardinal bufferDelay[RTPConstants::RTPMaxQualityLayers];
   cardinal classNumber[RTPConstants::RTPMaxQualityLayers];

    // ====== Layered allocation ============================================
   if(!UnlayeredAllocation) {
      for(cardinal i = 0;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
         bool mappedLayer = false;
         for(cardinal j = 0;j < rup.Mapping[i].Possibilities;j++) {

            // ====== Get class and bandwidth ===============================
            classNumber[i] = rup.Mapping[i].Possibility[j].Class;
            bandwidth[i]   = rup.Mapping[i].Possibility[j].Bandwidth;
            bufferDelay[i] = rup.Mapping[i].Possibility[j].BufferDelay;
#ifdef PRINT_ALLOCATION
            char str[256];
            snprintf((char*)&str,sizeof(str),"%7Ld in %4s, delay %2d => ",
                     bandwidth[i],
                     TrafficClassValues::getNameForTrafficClass(sla->Class[classNumber[i]].TrafficClass),
                     bufferDelay[i]);
            std::cout << "      Layer #" << i << ":  Trying to allocate " << str;
#endif

            // ====== Try allocation ========================================
            if(classAvailableBandwidthArray[classNumber[i]] >= bandwidth[i]) {
               if(Session->TotalAllocatedBandwidth + bandwidth[i] <=
                     std::min(bandwidthLimit,Session->MaxWantedBandwidth)) {
                  classAvailableBandwidthArray[classNumber[i]]     -= bandwidth[i];
                  Session->AllocatedBandwidthArray[classNumber[i]] += bandwidth[i];
#ifdef PRINT_ALLOCATION
                  std::cout << "success!" << std::endl;
#endif
                  totalAvailableBandwidth          -= bandwidth[i];
                  totalBandwidth                   += bandwidth[i];
                  Session->TotalAllocatedBandwidth += bandwidth[i];
                  mappedLayer                       = true;
                  break;
               }
#ifdef PRINT_ALLOCATION
               else {
                  std::cout << "failed due to limit "
                       << min(bandwidthLimit,Session->MaxWantedBandwidth)
                       << "." << std::endl;
               }
#endif
            }
#ifdef PRINT_ALLOCATION
            else {
               std::cout << "failed." << std::endl;
            }
#endif
         }
         if(mappedLayer == false) {
            goto rollback;
         }
      }
   }

   // ====== Unlayered allocation ===========================================
   else {
      bool mappedLayer = false;
      for(cardinal j = 0;j < rup.Mapping[0].Possibilities;j++) {
         bandwidth[0] = 0;
         for(cardinal i = 0;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
            bandwidth[0] += rup.Mapping[i].Possibility[j].Bandwidth;
         }

         // ====== Get class and bandwidth ===============================
         classNumber[0] = rup.Mapping[0].Possibility[j].Class;
         bufferDelay[0] = rup.Mapping[0].Possibility[j].BufferDelay;
#ifdef PRINT_ALLOCATION
         char str[256];
         snprintf((char*)&str,sizeof(str),"%7Ld in %4s, delay %2d => ",
                  bandwidth[0],
                  TrafficClassValues::getNameForTrafficClass(sla->Class[classNumber[0]].TrafficClass),
                  bufferDelay[0]);
         std::cout << "      Trying to unlayered-allocate " << str;
#endif

         // ====== Try allocation ========================================
         if(classAvailableBandwidthArray[classNumber[0]] >= bandwidth[0]) {
            if(Session->TotalAllocatedBandwidth + bandwidth[0] <=
                  std::min(bandwidthLimit,Session->MaxWantedBandwidth)) {
               classAvailableBandwidthArray[classNumber[0]]     -= bandwidth[0];
               Session->AllocatedBandwidthArray[classNumber[0]] += bandwidth[0];
#ifdef PRINT_ALLOCATION
               std::cout << "success!" << std::endl;
#endif
               totalAvailableBandwidth          -= bandwidth[0];
               totalBandwidth                   += bandwidth[0];
               Session->TotalAllocatedBandwidth += bandwidth[0];
               for(cardinal i = 1;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
                  classNumber[i] = classNumber[0];
                  bandwidth[i]   = rup.Mapping[i].Possibility[j].Bandwidth;
                  bufferDelay[i] = rup.Mapping[i].Possibility[j].BufferDelay;
               }
               bandwidth[0] = rup.Mapping[0].Possibility[j].Bandwidth;
               mappedLayer = true;
               break;
            }
#ifdef PRINT_ALLOCATION
            else {
               std::cout << "failed due to limit "
                         << min(bandwidthLimit,Session->MaxWantedBandwidth)
                         << "." << std::endl;
            }
#endif
         }
#ifdef PRINT_ALLOCATION
         else {
            std::cout << "failed." << std::endl;
         }
#endif
      }
      if(mappedLayer == false) {
         goto rollback;
      }
   }


   // ====== Success ========================================================
   rup.Bandwidth = totalBandwidth;
   for(cardinal i = 0;i < std::min(Layers,RTPConstants::RTPMaxQualityLayers);i++) {
      NewLayerClassNumber[i]    = classNumber[i];
      NewLayerClassBandwidth[i] = bandwidth[i];
      rup.LayerBandwidthInfo[i].BufferDelay    = bufferDelay[i];
      rup.LayerBandwidthInfo[i].BytesPerSecond = bandwidth[i];
   }
   return(true);


   // ====== Allocation failed ==============================================
rollback:
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      classAvailableBandwidthArray[i]     = oldClassAvailableBandwidthArray[i];
      Session->AllocatedBandwidthArray[i] = oldSessionBandwidthArray[i];
   }
   totalAvailableBandwidth          = oldTotalAvailableBandwidth;
   Session->TotalAllocatedBandwidth = oldSessionTotalBandwidth;


   return(false);
}
