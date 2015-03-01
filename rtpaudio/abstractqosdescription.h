// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Abstract QoS Description Inlines                                 ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2015 by Thomas Dreibholz            ####
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


#ifndef ABSTRACTQOSDESCRIPTION_H
#define ABSTRACTQOSDESCRIPTION_H


#include "tdsystem.h"
#include "frameratescalabilityinterface.h"
#include "abstractlayerdescription.h"
#include "rtppacket.h"
#include "resourceutilizationpoint.h"


/**
  * This class contains a stream's QoS requirements.
  *
  * @short   Abstract QoS Description.
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
*/
class AbstractQoSDescription : virtual public FrameRateScalabilityInterface
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   AbstractQoSDescription();

   /**
     * Destructor.
     */
   virtual ~AbstractQoSDescription();


   // ====== Initialization/update ==========================================
   /**
     * Initialize description.
     *
     * @param frameRate Frame rate.
     */
   inline void initDescription(const double frameRate);

   /**
     * Update description.
     *
     * @param pktHeaderSize Packet header size.
     * @param pktMaxSize Maximum packet size.
     */
   virtual void updateDescription(const cardinal pktHeaderSize,
                                  const cardinal pktMaxSize) = 0;


   // ====== Frame rate methods =============================================
   /**
     * Get frame rate.
     *
     * @return Frame rate.
     */
   inline double getFrameRate() const;

   /**
     * Set frame rate.
     *
     * @param frameRate Frame rate.
     * @return Frame rate set.
     */
   inline double setFrameRate(const double frameRate);

   /**
     * Get next higher frame rate.
     *
     * @return Frame rate.
     */
   inline double getNextFrameRate() const;

   /**
     * Get next lower frame rate.
     *
     * @return Frame rate.
     */
   inline double getPrevFrameRate() const;

   /**
     * Get frame rate scale factor:
     * (frameRate - MinFrameRate) / (MaxFrameRate - MinFrameRate).
     *
     * @return Frame rate scale factor (out of [0,1]).
     */
   inline double getFrameRateScaleFactor() const;


   // ====== Total bandwidth methods ========================================
   /**
     * Get minimum required total bandwidth.
     *
     * @return Minimum total bandwidth.
     */
   card64 getMinBandwidth() const;

   /**
     * Get maximum required total bandwidth.
     *
     * @return Maximum total bandwidth.
     */
   card64 getMaxBandwidth() const;


   // ====== Position =======================================================
   /**
     * Get position.
     *
     * @return Position.
     */
   inline card64 getPosition() const;

   /**
     * Set position.
     *
     * @param position Position.
     */
   inline void setPosition(const card64 position);


   // ====== Abstract layer methods =========================================
   /**
     * Get number of layers.
     *
     * @return Number of layers.
     */
   virtual cardinal getLayers() const = 0;

   /**
     * Get layer.
     *
     * @param layer Layer number.
     * @return Layer.
     */
   virtual AbstractLayerDescription* getLayer(const cardinal layer) const = 0;


   // ====== Get/set resources, calculate utilization for bandwidths ========
   /**
     * Get resources.
     *
     * @param rup ResourceUtilizationPoint reference to store resources.
     * @return Utilization.
     */
   double getResources(ResourceUtilizationPoint& rup) const;

   /**
     * Set resources.
     *
     * @param rup ResourceUtilizationPoint reference containing resources.
     * @return Utilization.
     */
   double setResources(const ResourceUtilizationPoint& rup);

   /**
     * Calculate utilization for given frame rate and layers bandwidths.
     *
     * @param frameRate Frame rate.
     * @param layers Number of layers.
     * @param bandwidth Bandwidth array with entry for each layer.
     * @return Utilization.
     */
   virtual double calculateUtilizationForLayerBandwidths(
                     const double    frameRate,
                     const cardinal  layers,
                     const card64*   bandwidth) const;


   // ====== Resource/Utilization calculation ===============================
   /**
     * Get precomputed resource utilization list. This method tries to use
     * a precomputed list instead of calculating all points like
     * calculateResourceUtilizationList().
     *
     * @param rup ResourceUtilizationPoint array capable of storing maxPoints entries.
     * @param bwThreshold Bandwidth threshold.
     * @param utThreshold Utilization threshold.
     * @param maxPoints Maximum number of ResourceUtilizationPoint to generate.
     *
     * @see calculateResourceUtilizationList
     */
   virtual cardinal getPrecomputedResourceUtilizationList(
                       ResourceUtilizationPoint* rup,
                       const card64              bwThreshold,
                       const double              utThreshold,
                       const cardinal            maxPoints) const = 0;

   /**
     * Calculate resource utilization list. To use a precomputed list, call
     * getPrecomputedResourceUtilizationList().
     *
     * @param rup ResourceUtilizationPoint array capable of storing maxPoints entries.
     * @param bwThreshold Bandwidth threshold.
     * @param utThreshold Utilization threshold.
     * @param maxPoints Maximum number of ResourceUtilizationPoint to generate.
     *
     * @see getPrecomputedResourceUtilizationList
     */
   virtual cardinal calculateResourceUtilizationList(
                       ResourceUtilizationPoint* rup,
                       const card64              bwThreshold,
                       const double              utThreshold,
                       const cardinal            maxPoints) const;

   /**
     * Calculate maximum utilization for given bandwidth. This is the
     * single-point version of calculateMaxUtilizationForBandwidthArray().
     *
     * @param totalBandwidth Total bandwidth.
     * @param rup ResourceUtilizationPoint reference to store result.
     * @return Utilization.
     *
     * @see calculateMaxUtilizationForBandwidthArray
     */
   inline double calculateMaxUtilizationForBandwidth(
                    const card64              totalBandwidth,
                    ResourceUtilizationPoint& rup) const;

   /**
     * Calculate maximum utilizations for given bandwidth array.
     *
     * @param totalBandwidthArray Total bandwidth array.
     * @param rupArray ResourceUtilizationPoint array to store results.
     * @param points Number of points in arrays.
     */
   virtual void calculateMaxUtilizationForBandwidthArray(
                   const card64*             totalBandwidthArray,
                   ResourceUtilizationPoint* rupArray,
                   const cardinal            points) const;


   // ====== Wanted quality settings ========================================
   /**
     * Get wanted utilization.
     *
     * @return Wanted utilization.
     */
   inline double getWantedUtilization() const;

   /**
     * Set wanted utilization.
     *
     * @param utilization Wanted utilization.
     */
   inline void setWantedUtilization(const double utilization);

   /**
     * Get minimum wanted bandwidth.
     *
     * @return Minimum wanted bandwidth.
     */
   card64 getMinWantedBandwidth() const;

   /**
     * Get maximum wanted bandwidth.
     *
     * @return Maximum wanted bandwidth.
     */
   card64 getMaxWantedBandwidth() const;

   /**
     * Set minimum wanted bandwidth.
     *
     * @param wanted bandwidth Minimum wanted bandwidth.
     */
   void setMinWantedBandwidth(const card64 bandwidth);

   /**
     * Set maximum wanted bandwidth.
     *
     * @param wanted bandwidth Maximum wanted bandwidth.
     */
   void setMaxWantedBandwidth(const card64 bandwidth);

   /**
     * Get stream priority.
     *
     * @return Stream priority.
     */
   inline int8 getStreamPriority() const;

   /**
     * Set stream priority.
     *
     * @param priority Stream priority.
     */
   inline void setStreamPriority(const int8 priority);

   /**
     * Get session priority.
     *
     * @return Session priority.
     */
   inline int8 getSessionPriority() const;

   /**
     * Set session priority.
     *
     * @param priority Session priority.
     */
   inline void setSessionPriority(const int8 priority);


   // ====== Protected data =================================================
   protected:
   double   WantedUtilization;
   card64   MinWantedBandwidth;
   card64   MaxWantedBandwidth;
   double   FrameRate;
   card64   Position;
   cardinal PktHeaderSize;
   cardinal PktMaxSize;
   int8     StreamPriority;
   int8     SessionPriority;


   // ====== Private data ===================================================
   private:
   void doResourceUtilizationIteration(
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
           cardinal&                 count) const;

   void calculateBandwidthInfo(const cardinal layer,
                               BandwidthInfo& bandwidthInfo) const;
};


/**
  * Output operator.
  */
std::ostream& operator<<(std::ostream& os, const AbstractQoSDescription& aqd);


#include "abstractqosdescription.icc"


#endif
