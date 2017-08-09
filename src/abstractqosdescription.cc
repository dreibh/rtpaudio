// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Abstract QoS Description Implementation                          ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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
#include "abstractqosdescription.h"


// Print resulting resource/utilization list after calculation
// #define PRINT_LIST


// ###### Constructor #######################################################
AbstractQoSDescription::AbstractQoSDescription()
{
   FrameRate          = HUGE_VAL;
   WantedUtilization  = 1.0;
   MinWantedBandwidth = 0;
   MaxWantedBandwidth = (card64)-1;
   StreamPriority     = 0;
   SessionPriority    = 0;
   PktMaxSize         = 1500;
   PktHeaderSize      = 28;
   Position           = (card64)-1;
}


// ###### Destructor ########################################################
AbstractQoSDescription::~AbstractQoSDescription()
{
}



// ##########################################################################
// #### Bandwidth methods                                                ####
// ##########################################################################



// ###### Get minimum required total bandwidth ##############################
card64 AbstractQoSDescription::getMinBandwidth() const
{
   card64 minBandwidth       = 0;
   const cardinal layers     = getLayers();
   const double minFrameRate = getMinFrameRate();
   for(cardinal i = 0;i < layers;i++) {
      const AbstractLayerDescription* layer = getLayer(i);
      if(!(layer->getFlags() & AbstractLayerDescription::LF_ExtensionLayer)) {
         minBandwidth += layer->frameSizeToBandwidth(
                            minFrameRate, layer->getMinFrameSize(minFrameRate));
      }
   }
   return(minBandwidth);
}


// ###### Get maximum required total bandwidth ##############################
card64 AbstractQoSDescription::getMaxBandwidth() const
{
   card64 maxBandwidth       = 0;
   const cardinal layers     = getLayers();
   const double maxFrameRate = getMaxFrameRate();
   for(cardinal i = 0;i < layers;i++) {
      const AbstractLayerDescription* layer = getLayer(i);
      maxBandwidth += layer->frameSizeToBandwidth(
                         maxFrameRate, layer->getMaxFrameSize(maxFrameRate));
   }
   return(maxBandwidth);
}



// ###########################################################################
// #### Get/set resources, calculate utilization for bandwidths           ####
// ###########################################################################



// ###### Get resources ######################################################
double AbstractQoSDescription::getResources(ResourceUtilizationPoint& rup) const
{
   rup.Bandwidth     = 0;
   rup.BandwidthCost = 0.0;
   rup.FrameRate     = FrameRate;
   rup.Layers        = getLayers();

   card64 bandwidth[rup.Layers];
   for(cardinal i = 0;i < rup.Layers;i++) {
      calculateBandwidthInfo(i,rup.LayerBandwidthInfo[i]);
      bandwidth[i] = rup.LayerBandwidthInfo[i].BytesPerSecond;
      rup.Bandwidth += bandwidth[i];
   }

   rup.Utilization = calculateUtilizationForLayerBandwidths(FrameRate,rup.Layers,(card64*)bandwidth);
   return(rup.Utilization);
}


// ###### Set resources ######################################################
double AbstractQoSDescription::setResources(const ResourceUtilizationPoint& rup)
{
   card64 bandwidth[std::max(rup.Layers,getLayers())];
   setFrameRate(rup.FrameRate);
   cardinal i;
   for(i = 0;i < std::min(rup.Layers,getLayers());i++) {
      AbstractLayerDescription* ald = getLayer(i);
      ald->setBandwidth(FrameRate,rup.LayerBandwidthInfo[i].BytesPerSecond);
      ald->setBufferDelay(rup.LayerBandwidthInfo[i].BufferDelay);
      bandwidth[i] = ald->getBandwidth();
   }
   for(   ;i < getLayers();i++) {
      AbstractLayerDescription* ald = getLayer(i);
      ald->setBandwidth(FrameRate,0);
      bandwidth[i] = ald->getBandwidth();
   }
   return(calculateUtilizationForLayerBandwidths(FrameRate,rup.Layers,(card64*)bandwidth));
}


// ###### Calculate bandwidth info ##########################################
void AbstractQoSDescription::calculateBandwidthInfo(const cardinal layer,
                                                    BandwidthInfo& bandwidthInfo) const
{
   if(layer >= getLayers()) {
      std::cerr << "WARNING: AbstractQoSDescription::calculateBandwidthInfo() - " << std::endl
                << "Invalid parameter " << layer << "!" << std::endl;
      return;
   }

   const AbstractLayerDescription* ald = getLayer(layer);
   bandwidthInfo.BufferDelay      = ald->getBufferDelay();
   bandwidthInfo.BytesPerSecond   = ald->getBandwidth();
   bandwidthInfo.PacketsPerSecond = ald->getPacketRate(FrameRate);
   bandwidthInfo.MaxTransferDelay = ald->getMaxTransferDelay();
   bandwidthInfo.MaxLossRate      = ald->getMaxLossRate();
   bandwidthInfo.MaxJitter        = ald->getMaxJitter();
}



// ###########################################################################
// #### Resource/Utilization calculation                                  ####
// ###########################################################################


// ###### Calculate utilization for layer bandwidths ########################
double AbstractQoSDescription::calculateUtilizationForLayerBandwidths(
          const double    frameRate,
          const cardinal  layers,
          const card64*   bandwidth) const
{
   // ====== Initialize =====================================================
   if(layers < getLayers()) {
      return(-1.0);
   }
   double frameSizeUtilization = 0.0;
   double frameSizeWeight      = 0.0;
   bool   ok                   = true;


   // ====== Calculate frameSizeUtilization for each layer ===========================
   for(cardinal i = 0;i < getLayers();i++) {
      const AbstractLayerDescription* ald = getLayer(i);
      const cardinal frameSize = ald->bandwidthToFrameSize(frameRate,bandwidth[i]);
      const double w = ald->getFrameSizeUtilizationWeight(frameRate);
      frameSizeWeight += w;
      if(frameSize < ald->getMinFrameSize(frameRate)) {
         if(!(ald->getFlags() & AbstractLayerDescription::LF_ExtensionLayer)) {
            ok = false;
            break;
         }
         else if(frameSize > 0) {
            std::cerr << "WARNING: AbstractQoSDescription::calculateUtilizationForLayerBandwidths() - "
                    "Senseless allocation to extension layer?!" << std::endl
                      << "Frame size " << frameSize << " for layer " << i << "." << std::endl
                      << "Minimum is " << ald->getMinFrameSize(frameRate) << "!" << std::endl;
         }
      }
      else {
         frameSizeUtilization += w *
            ald->getFrameSizeUtilizationForSize(frameRate,frameSize);
      }
   }


   // ====== Calculate global frameSizeUtilization ===================================
   double utilization = -1.0;
   if(ok == true) {
      const double frameRateWeight      = getFrameRateUtilizationWeight(frameRate);
      const double frameRateUtilization = getFrameRateUtilizationForRate(frameRate);
      frameSizeUtilization /= (double)frameSizeWeight;
      utilization = (frameRateWeight + 1.0) /
                       ( ((1.0 / frameSizeUtilization)) +
                         ((frameRateWeight / frameRateUtilization)) );
   }
   return(utilization);
}


// ###### Calculate maximum utilization for given total bandwidths ##########
void AbstractQoSDescription::calculateMaxUtilizationForBandwidthArray(
                                const card64*             totalBandwidthArray,
                                ResourceUtilizationPoint* rupArray,
                                const cardinal            points) const
{
   std::cerr << "WARNING: AbstractQoSDescription::calculateMaxUtilizationForBandwidthArray() - "
                "Needs to be overloaded..." << std::endl;

}


// ###### Recursive ResourceUtilizationPoint calculation method #############
void AbstractQoSDescription::doResourceUtilizationIteration(
          ResourceUtilizationPoint* rup,
          const card64              bwThreshold,
          const double              utThreshold,
          double*                   utilizationCache,
          card64*                   bandwidthCache,
          const cardinal            maxPoints,
          const cardinal            maxCachePoints,
          const cardinal            start,
          const cardinal            end,
          const card64              startBandwidth,
          const card64              endBandwidth,
          const cardinal            level,
          const cardinal            maxLevel,
          cardinal&                 count) const
{
   // ====== Calculate middle cache position and bandwidth ==================
   const cardinal m            = (start + end) / 2;
   const card64   newBandwidth = (startBandwidth + endBandwidth) / 2;


   // ====== If level is lowest: Calculate new ResourceUtilizationPoint =====
   if(level == maxLevel) {
      utilizationCache[m] = calculateMaxUtilizationForBandwidth(newBandwidth,rup[count]);
      bandwidthCache[m]   = rup[count].Bandwidth;


      // ====== Add point, if utilization and bandwidth is useful ===========
      if(utilizationCache[m] >= 0.0) {
         // ====== Get nearest lower point's utilization ====================
         double uLeft = utilizationCache[m];
         card64 bLeft = bandwidthCache[m];
         for(integer i = (integer)m - 1;i >= 0;i--) {
            if(utilizationCache[i] != -HUGE_VAL) {
               uLeft = utilizationCache[i];
               bLeft = bandwidthCache[i];
               break;
            }
         }

         // ====== Get nearest higher point's utilization ===================
         double uRight = utilizationCache[m];
         card64 bRight = bandwidthCache[m];
         for(cardinal i = m + 1;i < maxCachePoints;i++) {
            if(utilizationCache[i] != -HUGE_VAL) {
               uRight = utilizationCache[i];
               bRight = bandwidthCache[i];
               break;
            }
         }

         // Add point, if utilization or bandwidth difference is greater than
         // threshold.
         if( ((utilizationCache[m] - uLeft >= utThreshold) &&
              (uRight - utilizationCache[m] >= utThreshold)) ||
             (((double)bandwidthCache[m] - (double)bLeft >= (double)bwThreshold) &&
              ((double)bRight - (double)bandwidthCache[m] >= (double)bwThreshold)) ) {
            count++;
         }
         else {
            utilizationCache[m] = -HUGE_VAL;
         }
      }
      else {
         utilizationCache[m] = -HUGE_VAL;
      }
   }


   // ====== Go into next recursion level ===================================
   else {
      if(count < maxPoints) {
         if(start + 1 < m) {
            doResourceUtilizationIteration(rup,
                                           bwThreshold, utThreshold,
                                           utilizationCache, bandwidthCache,
                                           maxPoints,
                                           maxCachePoints,
                                           start, m,
                                           startBandwidth, newBandwidth,
                                           level + 1, maxLevel, count);
         }
      }
      if(count < maxPoints) {
         if(m + 1 < end) {
            doResourceUtilizationIteration(rup,
                                           bwThreshold, utThreshold,
                                           utilizationCache, bandwidthCache,
                                           maxPoints,
                                           maxCachePoints,
                                           m, end,
                                           newBandwidth, endBandwidth,
                                           level + 1, maxLevel, count);
         }
      }
   }
}


// ###### Calculate ResourceUtilization list ################################
cardinal AbstractQoSDescription::calculateResourceUtilizationList(
            ResourceUtilizationPoint* rup,
            const card64              bwThreshold,
            const double              utThreshold,
            const cardinal            maxPoints) const
{
   // ====== Get minimum and maximum RU points ==============================
   const card64 minBandwidth = getMinBandwidth();
   const card64 maxBandwidth = getMaxBandwidth();
   // printf("min=%Ld  max=%Ld\n",minBandwidth,maxBandwidth);

   if(calculateMaxUtilizationForBandwidth(minBandwidth,rup[0]) < 0.0) {
      std::cerr << "WARNING: AbstractQoSDescription::calculateResourceUtilizationList() - "
                   "Empty resource/utilization list for minimum bandwidth?!" << std::endl;
      return(0);
   }
   if(calculateMaxUtilizationForBandwidth(maxBandwidth,rup[1]) < 0.0) {
      std::cerr << "WARNING: AbstractQoSDescription::calculateResourceUtilizationList() - "
                   "Empty resource/utilization list for maximum bandwidth?!" << std::endl;
      return(0);
   }


   // ====== Initialize utilization and bandwidth caches ====================
   const cardinal additionalDepth = 2;
   const cardinal iterations =
      additionalDepth + (cardinal)ceil(log((double)maxPoints) / log(2.0));
   const cardinal maxCachePoints =  (1 << (iterations + 1));

   double utilizationCache[maxCachePoints];
   card64 bandwidthCache[maxCachePoints];
   for(cardinal i = 2;i < maxCachePoints;i++) {
      utilizationCache[i] = -HUGE_VAL;
      bandwidthCache[i]   = (card64)-1;
   }
   utilizationCache[0]                  = rup[0].Utilization;
   utilizationCache[maxCachePoints - 1] = rup[1].Utilization;
   bandwidthCache[0]                    = rup[0].Bandwidth;
   bandwidthCache[maxCachePoints - 1]   = rup[1].Bandwidth;
   cardinal count = 2;


   // ====== Execute recursive algorithm ====================================
   for(cardinal i = 0;count < maxPoints;i++) {
      doResourceUtilizationIteration(rup,
         bwThreshold, utThreshold,
         (double*)&utilizationCache, (card64*)&bandwidthCache,
         maxPoints,
         maxCachePoints,
         0, maxCachePoints - 1,
         minBandwidth, maxBandwidth,
         0, i, count);

      if(i >= iterations) {
         break;
      }
   }


   // ====== Sort list by utilization =======================================
   ResourceUtilizationPoint::sortResourceUtilizationList(rup,0,count - 1);


   // ====== Verify result ==================================================
#ifdef PRINT_LIST
   std::cout << "Calculated resource/Utilization list:" << std::endl;
      for(cardinal i = 0;i < count;i++) {
      std::cout << rup[i] << std::endl;
   }
#endif
   card64 bandwidth = 0;
   for(cardinal i = 0;i < count;i++) {
      if((rup[i].Utilization < 0.0) || (rup[i].Utilization > 1.0)) {
         std::cerr << "INTERNAL ERROR: AbstractQoSDescription::calculateResourceUtilizationList() - "
                      "Resulting utilization out of range [0,1]!" << std::endl;
         ::abort();
      }
      if(rup[i].Bandwidth < bandwidth) {
         std::cerr << "INTERNAL ERROR: AbstractQoSDescription::calculateResourceUtilizationList() - "
                      "Senseless bandwidth settings in resource/utilization list!" << std::endl;
         ::abort();
      }
      if((i < count - 1) &&
         (rup[i].Utilization == rup[i + 1].Utilization) &&
         (rup[i].Bandwidth   == rup[i + 1].Bandwidth) &&
         (rup[i].FrameRate   == rup[i + 1].FrameRate)) {
         std::cerr << "INTERNAL ERROR: AbstractQoSDescription::calculateResourceUtilizationList() - "
                      "Duplicate points in list?!" << std::endl;
         ::abort();
      }
   }
   if(rup[count - 1].Utilization < 1.0) {
      std::cerr << "INTERNAL ERROR: AbstractQoSDescription::calculateResourceUtilizationList() - "
                   "Maximum utilization < 1.0?!" << std::endl;
      ::abort();
   }

   return(count);
}


// ##########################################################################
// #### Output operator                                                  ####
// ##########################################################################



// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, const AbstractQoSDescription& aqd)
{
   // ====== Get resources ==================================================
   const double frameRate = aqd.getFrameRate() ;
   ResourceUtilizationPoint rup;
   aqd.getResources(rup);


   // ====== Print AbstractQoSDescription ===================================
   os << "Bandwidth        = " << aqd.getMinBandwidth() << " <= "
                               << rup.Bandwidth << " <= "
                               << aqd.getMaxBandwidth()   << std::endl;
   os << "TotalUtilization = " << rup.Utilization * 100.0 << " [%]" << std::endl;
   os << "Layers           = " << aqd.getLayers() << std::endl;


   // ====== Print FrameRateScalability =====================================
   os << "Frame Rate:" << std::endl;
   os << "   FrameRate        = " << aqd.getMinFrameRate() << " <= "
                               << frameRate             << " <= "
                               << aqd.getMaxFrameRate() << std::endl;
   os << "   Scalability Cls. = " << aqd.getFrameRateScalabilityClass() << std::endl;
   os << "   Properties       = ";
   if(aqd.isFrameRateScalable()) os << "<Scalable> "; else os << "<Non-scalable> ";
   os << std::endl;
   os << "   Scale Factor     = " << aqd.getFrameRateScaleFactor() * 100.0
                               << " [%]" << std::endl;
   os << "   PrevFrameRate    = " << aqd.getPrevFrameRate() << std::endl;
   os << "   NextFrameRate    = " << aqd.getNextFrameRate() << std::endl;


   // ====== Print AbstractLayerDescriptions ================================
   for(cardinal i = 0;i < aqd.getLayers();i++) {
      os << "Layer #" << i << " Frame Size:" << std::endl;
      const AbstractLayerDescription* ald = aqd.getLayer(i);
      cardinal frameSize = ald->bandwidthToFrameSize(frameRate,ald->getBandwidth());
      if(frameSize > ald->getMaxFrameSize(frameRate)) {
         frameSize = ald->getMaxFrameSize(frameRate);
      }
      os << "   FrameSize        = " << ald->getMinFrameSize(frameRate)  << " <= "
                                     << frameSize << " <= "
                                     << ald->getMaxFrameSize(frameRate)  << "   (Payload: "
                                     << ald->getMinPayloadFrameSizeForDelay(frameRate,ald->getBufferDelay()) << " <= "
                                     << ald->rawToPayload(frameRate,frameSize,ald->getBufferDelay())    << " <= "
                                     << ald->getMaxPayloadFrameSizeForDelay(frameRate,ald->getBufferDelay()) << ")"
                                     << std::endl;
      os << "   BytesPerSecond   = " << ald->getBandwidth()           << std::endl;
      os << "   PacketsPerSecond = " << ald->getPacketRate(frameRate) << " ("
                                     << ald->getPacketCountForSize(frameRate,frameSize)
                                     << " per frame)" << std::endl;
      os << "   Souce            = " << ald->getSource() << std::endl;
      os << "   Destination      = " << ald->getDestination() << std::endl;
      os << "   Scalability Cls. = " << ald->getFrameSizeScalabilityClass() << std::endl;
      os << "   Properties       = ";
      if(ald->isFrameSizeScalable()) os << "<Scalable> "; else os << "<Non-scalable> ";
      if(ald->isVariableBitrate())   os << "<VBR> ";      else os << "<CBR> ";
      os << std::endl;
      os << "   Scale Factor     = " << ald->getFrameSizeScaleFactorForSize(frameRate,frameSize) * 100.0
                                     << " [%]" << std::endl;
      os << "   PrevFrameSize    = " << ald->getPrevFrameSizeForSize(frameRate,frameSize) << " (Payload: "
                                     << ald->rawToPayload(frameRate,(cardinal)ald->getPrevFrameSizeForSize(frameRate,frameSize),ald->getBufferDelay()) << ")" << std::endl;
      os << "   NextFrameSize    = " << ald->getNextFrameSizeForSize(frameRate,frameSize) << " (Payload: "
                                     << ald->rawToPayload(frameRate,(cardinal)ald->getNextFrameSizeForSize(frameRate,frameSize),ald->getBufferDelay()) << ")" << std::endl;
      os << "   PeakFrameSize    = " << ald->getPeakFrameSizeForSize(frameRate,frameSize) << " (Payload: "
                                     << ald->rawToPayload(frameRate,ald->getPeakFrameSizeForSize(frameRate,frameSize),ald->getBufferDelay())
                                     << ")" << std::endl;

      os << "   BufferDelay      = " << ald->getBufferDelay()   << " [Frame(s)]" << std::endl;
      os << "   MaxTransferDelay = " << ald->getMaxTransferDelay()    << " [ms]" << std::endl;
      os << "   MaxLossRate      = " << ald->getMaxLossRate() * 100.0 << " [%]"  << std::endl;
      os << "   MaxJitter        = " << ald->getMaxJitter()           << " [ms]" << std::endl;
   }
   return(os);
}
