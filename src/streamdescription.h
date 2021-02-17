// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Stream Description                                               ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2021 by Thomas Dreibholz            ####
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


#ifndef STREAMDESCRIPTION_H
#define STREAMDESCRIPTION_H


#include "tdsystem.h"
#include "servicelevelagreement.h"
#include "trafficclassvalues.h"
#include "managedstreaminterface.h"
#include "abstractqosdescription.h"
#include "sessiondescription.h"


/**
   * Maximum number of entries in the list.
   */
static const cardinal MaxRUEntries = 256;


/**
  * This class contains a description of a stream. It is used
  * for the bandwidth manager's remapping algorithm.
  *
  * @short   Stream Description
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class StreamDescription
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   StreamDescription();

   /**
     * Destructor.
     */
   ~StreamDescription();


   // ====== Initialization =================================================
   /**
     * Initialize.
     *
     * @param stream ManagedStreamInterface.
     * @param sla Service level agreement.
     * @param maxPoints Maximum number of resource/utilization points for each stream.
     * @param bwThreshold Bandwidth threshold.
     * @param utThreshold Utilization threshold.
     * @param systemDelayTolerance The system's delay tolerance for the buffering optimization.
     * @param unlayeredAllocation True, to use the same DiffServ class for *all* layers; false otherwise.
     */
   void init(ManagedStreamInterface*      stream,
             const ServiceLevelAgreement* sla,
             const cardinal               maxPoints,
             const card64                 bwThreshold,
             const double                 utThreshold,
             const double                 systemDelayTolerance,
             const bool                   unlayeredAllocation);


   // ====== Layer -> DiffServ class mapping and bandwidth allocation =======
   /**
     * Try to allocate given layer bandwidths to a stream. If allocation
     * is successful, the availability references are decremented by the
     * bandwidth allocation.
     * The allocated bandwidths and buffer delays will be stored into the
     * resource/utilization point.
     *
     * @param sla Service level agreement.
     * @param totalAvailableBandwidth Reference to total available bandwidth.
     * @param classAvailableBandwidthArray Available bandwidths for each DiffServ class.
     * @param rup Resource/utilization point to do allocation for.
     * @param session Session of the stream.
     * @param bandwidthLimit Upper bandwidth limit (e.g. the session's maximum bandwidth).
     */
   bool tryAllocation(const ServiceLevelAgreement* sla,
                      card64&                      totalAvailableBandwidth,
                      card64*                      classAvailableBandwidthArray,
                      ResourceUtilizationPoint&    rup,
                      const card64                 bandwidthLimit = (card64)-1);


   // ====== Stream and session information =================================
   /**
     * Stream's manager interface.
     */
   ManagedStreamInterface* Interface;

   /**
     * Stream's AbstractQoSDescription.
     */
   AbstractQoSDescription* QoSDescription;

   /**
     * Session description.
     */
   SessionDescription* Session;

   /**
     * Stream ID.
     */
   card64 StreamID;

   /**
     * Number of layers.
     */
   cardinal Layers;


   // ====== Resource/utilization list ======================================
   /**
     * Number of entries in resource/utilization list.
     */
   cardinal RUEntries;

   /**
     * Resource/utilization list.
     */
   ResourceUtilizationPoint RUList[MaxRUEntries];


   // ====== Newly allocated bandwidths and classes =========================
   /**
     * New remapping's layer's allocated DiffServ class number.
     */
   cardinal NewLayerClassNumber[RTPConstants::RTPMaxQualityLayers];

   /**
     * New remapping's layer's allocated bandwidth.
     */
   card64 NewLayerClassBandwidth[RTPConstants::RTPMaxQualityLayers];

   /**
     * New remapping's bandwidth cost per second.
     */
   double NewCostPerSecond;

   /**
     * New remapping's quality.
     */
   ResourceUtilizationPoint NewQuality;

   /**
     * Last *complete* remapping's utilization.
     */
   double LastUtilization;


   // ====== Current allocated bandwidths and classes =======================
   /**
     * Layer's allocated DiffServ class number currently used.
     */
   cardinal CurrentLayerClassNumber[RTPConstants::RTPMaxQualityLayers];

   /**
     * Current layer's allocated bandwidth currently used.
     */
   card64 CurrentLayerClassBandwidth[RTPConstants::RTPMaxQualityLayers];

   /**
     * Current bandwidth cost per second.
     */
   double CurrentCostPerSecond;

   /**
     * Current quality.
     */
   ResourceUtilizationPoint CurrentQuality;

   /**
     * Reservation time stamp.
     */
   card64 ReservationTimeStamp;


   // ====== Statistics =====================================================
   /**
     * Total cost of this stream.
     */
   double TotalCost;

   /**
     * Total bandwidth usage of this stream.
     */
   double TotalBandwidthUsage;

   /**
     * Total utilization of this stream.
     */
   double TotalUtilization;

   /**
     * Total runtime of this stream.
     */
   double TotalRuntime;

   /**
     * Total reservation updates.
     */
   cardinal TotalReservationUpdates;

   /**
     * Number of successful partial remappings.
     */
   cardinal PartialRemappings;

   /**
     * Number of complete remappings forced by this stream.
     */
   cardinal CompleteRemappings;

   /**
     * Number of init() calls.
     *
     * @see init
     */
   cardinal Inits;

   /**
     * Number of buffer flushes.
     */
   cardinal BufferFlushes;

   /**
     * Duration of last init() call.
     */
   card64 LastInitDuration;



   // ====== Network quality ================================================
   /**
     * Smoothed received loss rate for each layer (via RTCP reports).
     */
   double ReportedLossRate[RTPConstants::RTPMaxQualityLayers];

   /**
     * Smoothed reported jitter for each layer (via RTCP reports).
     */
   double ReportedJitter[RTPConstants::RTPMaxQualityLayers];

   /**
     * Smoothed measured transfer delay from ICMP echo replies for each DiffServ class.
     */
   double MeasuredTransferDelay[TrafficClassValues::MaxValues];


   // ====== Miscellaneous data =============================================
   /**
     * Round trip time measurement destination.
     */
   InternetAddress RoundTripTimeDestination;

   /**
     * Time to next interval in microseconds.
     */
   card64 NextInterval;

   /**
     * True, if all following higher bandwidth allocations will fail (no more
     * bandwidth available to achieve higher quality -> no more allocation trials
     * necessary); false otherwise.
     */
   bool MaximumReached;

   /**
     * Unlayered allocation: Use the same DiffServ class for *all* layers.
     */
   bool UnlayeredAllocation;


   // ====== Private methods ================================================
   private:
   bool calculatePossibleLayerClassMappings(
           ResourceUtilizationPoint&     rup,
           const ServiceLevelAgreement*  sla,
           const AbstractQoSDescription* aqd);
};


#endif
