// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Resource Utilization Point                                       ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#ifndef RESOURCEUTILIZATIONPOINT_H
#define RESOURCEUTILIZATIONPOINT_H


#include "tdsystem.h"
#include "rtppacket.h"
#include "bandwidthinfo.h"
#include "trafficclassvalues.h"


class StreamDescription;


/**
  * This structure contains a possible layer to DiffServ class mapping.
  *
  * @short   Layer Class Mapping Possibility
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
*/
struct LayerClassMappingPossibility
{
   /**
     * Class ID.
     */
   cardinal Class;

   /**
     * Buffer delay.
     */
   cardinal BufferDelay;

   /**
     * Cost.
     */
   double Cost;

   /**
     * Bandwidth.
     */
   card64 Bandwidth;
};


/**
  * This structure contains a list of possible layer to DiffServ class mapping.
  *
  * @short   Layer Class Mapping
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
*/
struct LayerClassMapping
{
   /**
     * Number of possible DiffServ classes for the layer.
     */
   cardinal Possibilities;

   /**
     * List of possible DiffServ classes for the layer.
     */
   LayerClassMappingPossibility Possibility[TrafficClassValues::MaxValues];
};


/**
  * This class is a resource/utilization point used for the bandwidth mapping
  * algorithm.
  *
  * @short   Resource Utilization Point
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
*/
class ResourceUtilizationPoint
{
   // ====== Public data ====================================================
   public:
   /**
     * Total bandwidth.
     */
   card64 Bandwidth;

   /**
     * Bandwidth cost.
     */
   double BandwidthCost;

   /**
     * Total utilization.
     */
   double Utilization;

   /**
     * Frame rate.
     */
   double FrameRate;

   /**
     * Number of layers.
     */
   cardinal Layers;


   /**
     * Array of layers' bandwidth requirements.
     */
   BandwidthInfo LayerBandwidthInfo[RTPConstants::RTPMaxQualityLayers];

   /**
     * Layer to DiffServ class mapping possibilities.
     */
   LayerClassMapping Mapping[RTPConstants::RTPMaxQualityLayers];


   // ====== Operations =====================================================
   /**
     * Reset.
     */
   void reset();

   /**
     * Merge resource/utilization lists.
     *
     * @param destination Destination list.
     * @param listArray Array of lists to merge.
     * @param listSizeArray Array of list sizes.
     * @param listCount Number of lists.
     * @return Number of points in destination list.
     */
   static cardinal mergeResourceUtilizationLists(
                      ResourceUtilizationPoint*  destination,
                      ResourceUtilizationPoint** listArray,
                      const cardinal*            listSizeArray,
                      const cardinal             listCount);

   /**
     * Sort resource/utilization list by utilization.
     *
     * @param rup List.
     * @param start First point number.
     * @param end Last point number.
     */
   static void sortResourceUtilizationList(
                  ResourceUtilizationPoint* rup,
                  const integer            start,
                  const integer            end);

   /**
     * Optimize resource/utilization list by utilization:
     * Eliminate points which have higher resource requirements or cost
     * than higher-utilized points following.
     *
     * @param rup List.
     * @param count Number of entries in list.
     * @return Number of entries remaining in list.
     */
   static cardinal optimizeResourceUtilizationList(
                      ResourceUtilizationPoint* rup,
                      const cardinal            count);

   /**
     * Compute convex hull on resource/utilization list using Graham Scan
     * algorithm.
     *
     * @param rup List.
     * @param count Number of entries in list.
     * @return Number of entries remaining in list.
     */
   static cardinal grahamScanResourceUtilizationList(ResourceUtilizationPoint* rup,
                                                     const cardinal            count);


   // ====== Comparision operators ==========================================
   /**
     * Operator "==".
     */
   inline int operator==(const ResourceUtilizationPoint& rup) const;

   /**
     * Operator "!=".
     */
   inline int operator!=(const ResourceUtilizationPoint& rup) const;


   // ====== Private data ===================================================
   private:
   static inline void swapResourceUtilizationPoints(ResourceUtilizationPoint& a,
                                                    ResourceUtilizationPoint& b);
   static inline integer ccw(const ResourceUtilizationPoint& p0,
                             const ResourceUtilizationPoint& p1,
                             const ResourceUtilizationPoint& p2);
};


/**
  * Output operator.
  */
std::ostream& operator<<(std::ostream& os, const ResourceUtilizationPoint& rup);


#include "resourceutilizationpoint.icc"


#endif
