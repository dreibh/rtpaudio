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
// ####           Copyright (C) 1999-2023 by Thomas Dreibholz            ####
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


#ifndef BANDWIDTHMANAGER_H
#define BANDWIDTHMANAGER_H


#include "tdsystem.h"
#include "qosmanagerinterface.h"
#include "abstractqosdescription.h"
#include "timedthread.h"
#include "servicelevelagreement.h"
#include "streamdescription.h"
#include "sessiondescription.h"
#include "roundtriptimepinger.h"
#include "rtcppacket.h"


#include <map>
#include <algorithm>


/**
  * This is a resource/utilization point structure to be used
  * within the bandwidth manager.
  *
  * @short   Resource Utilization Simple Point
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
*/
struct ResourceUtilizationSimplePoint
{
   // ====== Stream and point identification ================================
   /**
     * StreamDescription of this point's stream.
     */
   StreamDescription* Stream;

   /**
     * Point number (StreamDescription's RUList entry number).
     */
   cardinal Point;

   /**
     * Stream's priority factor.
     */
   double StreamPriorityFactor;


   // ====== Point data =====================================================
   /**
     * Bandwidth.
     */
   card64 Bandwidth;

   /**
     * Bandwidth cost.
     */
   double BandwidthCost;

   /**
     * Utilization.
     */
   double Utilization;

   /**
     * Sorting value.
     */
   double SortingValue;
};

/**
  * Output operator.
  */
std::ostream& operator<<(std::ostream& os, const ResourceUtilizationSimplePoint& srup);



/**
  * This is a resource/utilization multipoint structure to be used
  * within the bandwidth manager.
  *
  * @short   Resource Utilization Simple Point
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
*/
struct ResourceUtilizationMultiPoint
{
   // ====== Comparision operators ==========================================
   /**
     * Operator "<".
     */
   inline int operator<(const ResourceUtilizationMultiPoint& srup) const;

   /**
     * Operator "<".
     */
   inline int operator>(const ResourceUtilizationMultiPoint& srup) const;


   // ====== Session identification =========================================
   /**
     * SessionDescription of this multipoint's session.
     */
   SessionDescription* Session;

   /**
     * Session's priority factor.
     */
   double SessionPriorityFactor;


   // ====== Streams and points identification ==============================
   /**
     * Maximum number of streams per session.
     */
   static const cardinal MaxStreamsPerSession = 128;

   /**
     * Number of streams in this session.
     */
   cardinal Streams;

   /**
     * Array of StreamDescriptions for this multipoint's streams.
     */
   StreamDescription* Stream[MaxStreamsPerSession];

   /**
     * Array of point numbers for this multipoint's points.
     */
   cardinal Point[MaxStreamsPerSession];


   // ====== Point data =====================================================
   /**
     * Bandwidth.
     */
   card64 Bandwidth;

   /**
     * Bandwidth cost.
     */
   double BandwidthCost;

   /**
     * Utilization.
     */
   double Utilization;

   /**
     * Sorting value.
     */
   double SortingValue;


   // ====== Miscellaneous ==================================================
   /**
     * True, if this point has already been allocated during session's
     * minimum bandwidth allocation.
     */
   bool AlreadyAllocated;
};


/**
  * Output operator.
  */
std::ostream& operator<<(std::ostream& os, const ResourceUtilizationMultiPoint& srup);



/**
  * This is the bandwidth manager.
  *
  * @short   Bandwidth Manager
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
*/
class BandwidthManager : virtual public QoSManagerInterface,
                         public TimedThread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param sla ServiceLevelAgreement object.
     * @param rttp RoundTripTimePinger object.
     */
   BandwidthManager(ServiceLevelAgreement* sla,
                    RoundTripTimePinger*   rttp);

   /**
     * Destructor.
     */
   ~BandwidthManager();


   // ====== Stream management ==============================================
   /**
     * Add stream to management.
     *
     * @param stream Stream to add.
     * @param session SessionID of session to add stream to (0 for no session).
     * @param name Stream name (only for log printing).
     */
   void addStream(ManagedStreamInterface* stream,
                  const cardinal          sessionID = 0,
                  const char*             name      = NULL);

   /**
     * Remove stream from management.
     *
     * @param stream Stream to remove.
     */
   void removeStream(ManagedStreamInterface* stream);

   /**
     * Update stream.
     *
     * @param stream Stream to be updated.
     */
   void updateStream(ManagedStreamInterface* stream);

   /**
     * Force a complete remapping.
     */
   inline void forceCompleteRemapping();

   // ====== Events =========================================================
   /**
     * Interval has changed.
     *
     * @param stream Stream with changed interval.
     * @param isNew true, if new interval has been reached; false otherwise.
     * @param when Microseconds to next interval.
     * @param newRUList true, if new resource/utiliztion list has been reached; false otherwise.
     */
   void intervalChangeEvent(ManagedStreamInterface* stream,
                            const bool              isNew,
                            const card64            when,
                            const bool              newRUList);

   /**
     * Report reception for given layer.
     *
     * @param stream Stream.
     * @param report Report.
     * @param layer Layer.
     */
   void reportEvent(ManagedStreamInterface*         stream,
                    const RTCPReceptionReportBlock* report,
                    const cardinal                  layer);

   /**
     * Buffer flush for a given layer.
     *
     * @param stream Stream.
     */
   void bufferFlushEvent(ManagedStreamInterface* stream,
                         const cardinal          layer);

   /**
     * Implementation of TimedThread's timerEvent() method.
     *
     * @see TimedThread#timerEvent
     */
   void timerEvent();


   // ====== Settings =======================================================
   /**
     * Set log stream.
     *
     * @param logStream Stream to write log information to; NULL to disable logging.
     */
   void setLogStream(std::ostream* logStream);

   /**
     * Get partial remapping parameters.
     *
     * @param enabled Reference to store true, if partial remappings are enabled; false otherwise.
     * @param reservedPortion Reference to store reserved bandwidth portion (out of [0,1]).
     * @param utilizationTolerance Reference to store utilization tolerance for partial remapping.
     * @param maxRemappingInterval Reference to store maximum interval between two complete remappings.
     */
   inline void getPartialRemapping(bool&   enabled,
                                   double& reservedPortion,
                                   double& utilizationTolerance,
                                   double& maxRemappingInterval) const;


   /**
     * Set partial remapping parameters.
     *
     * @param enabled true, if partial remappings are enabled; false otherwise.
     * @param reservedPortion Reserved bandwidth portion (out of [0,1]).
     * @param utilizationTolerance Utilization tolerance for partial remapping.
     * @param maxRemappingInterval Maximum interval between two complete remappings.
     */
   inline void setPartialRemapping(const bool   enabled,
                                   const double reservedPortion,
                                   const double utilizationTolerance,
                                   const double maxRemappingInterval);

   /**
     * Get fairnes settings.
     *
     * @param fairnessSession Reference to store session fairness.
     * @param fairnessStream Reference to store stream fairness.
     */
   inline void getFairness(double& fairnessSession,
                           double& fairnessStream) const;

   /**
     * Set fairnes settings.
     *
     * @param fairnessSession Session fairness.
     * @param fairnessStream Stream fairness.
     */
   inline void setFairness(const double fairnessSession,
                           const double fairnessStream);

   /**
     * Get QoS optimization parameters.
     *
     * @param maxRUPoints Reference to store maximum number of RU points per stream.
     * @param utilizationThreshold Reference to store utilization threshold.
     * @param bandwidthThreshold Reference to store bandwidth threshold.
     * @param systemDelayTolerance Reference to store the system's delay tolerance for the buffering optimization.
     */
   inline void getQoSOptimizationParameters(
                  cardinal& maxRUPoints,
                  double&   utilizationThreshold,
                  card64&   bandwidthThreshold,
                  double&   systemDelayTolerance,
                  bool&     unlayeredAllocation) const;

   /**
     * Set QoS optimization parameters.
     *
     * @param maxRUPoints Maximum number of RU points per stream.
     * @param utilizationThreshold Utilization threshold.
     * @param bandwidthThreshold Bandwidth threshold.
     * @param systemDelayTolerance The system's delay tolerance for the buffering optimization.
     */
   inline void setQoSOptimizationParameters(
                  const cardinal maxRUPoints,
                  const double   utilizationThreshold,
                  const card64   bandwidthThreshold,
                  const double   systemDelayTolerance,
                  const bool     unlayeredAllocation);


   // ====== Bandwidth variables ============================================
   card64 TotalAvailableBandwidth;
   card64 ClassAvailableBandwidthArray[TrafficClassValues::MaxValues];
   card64 TotalBandwidth;
   card64 ClassBandwidthArray[TrafficClassValues::MaxValues];
   int64  SLAUpdateRecommendation[TrafficClassValues::MaxValues];


   // ====== Information about remappings ===================================
   card64   StreamIDGenerator;
   card64   LastCompleteRemapping;
   card64   LastCompleteRemappingDuration;
   cardinal CompleteRemappings;
   cardinal PartialRemappings;
   cardinal TotalBufferFlushes;


   // ====== Simulator variables ============================================
   static card64 SimulatorTime;


   // ====== Stream and session management ==================================
   std::multimap<ManagedStreamInterface*,StreamDescription*> StreamSet;
   std::multimap<cardinal,SessionDescription*>               SessionSet;
   ServiceLevelAgreement*                                    SLA;
   cardinal                                                  Streams;
   cardinal                                                  Sessions;


   // ====== Constants ======================================================
   cardinal MaxRUPoints;
   double   UtilizationThreshold;
   card64   BandwidthThreshold;
   double   SystemDelayTolerance;
   double   FairnessSession;
   double   FairnessStream;
   double   AlphaLossRate;
   double   AlphaJitter;
   double   PartialRemappingPortion;
   double   PartialRemappingUtilizationTolerance;
   card64   MaxRemappingInterval;
   bool     EnablePartialRemappings;
   bool     UnlayeredAllocation;


   // ====== Private data ===================================================
   private:
   cardinal calculateSessionMultiPoints(
               SessionDescription*       session,
               const cardinal            offset,
               const cardinal            lastPoint,
               ResourceUtilizationMultiPoint* rumpList);
   void getRoundTripTimes(StreamDescription* sd);

   inline double getPriorityFactor(const int8 streamPriority) const;
   inline double getResourcePart(const ResourceUtilizationSimplePoint& rup) const;
   inline double getResourcePart(const ResourceUtilizationMultiPoint& rump) const;
   inline double getStreamSortingValue(const ResourceUtilizationSimplePoint& rup) const;
   inline double getSessionSortingValue(const ResourceUtilizationMultiPoint& rump) const;

   static inline void smoothedUpdate(double& value, const double measured, const double alpha);

   public:
   void updateReservation(StreamDescription* streamDescription);
   private:

   bool tryAllocation(
           ResourceUtilizationMultiPoint& rump,
           const card64                   bandwidthLimit = (card64)-1);

   void doAllocationTrials(
           ResourceUtilizationMultiPoint* rumpList,
           const cardinal                 points,
           const card64                   bandwidthLimit = (card64)-1);

   bool doPartialRemapping(StreamDescription* streamDescription);
   void doCompleteRemapping();


   RoundTripTimePinger* RTTP;
   std::ostream*        Log;
   card64               LogStartupTimeStamp;
   bool                 Changed;
};


#include "bandwidthmanager.icc"


#endif
