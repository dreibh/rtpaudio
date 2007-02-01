// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Resource Utilization Point Implementation                        ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#include "tdsystem.h"
#include "rtppacket.h"
#include "resourceutilizationpoint.h"


// ###### Reset #############################################################
void ResourceUtilizationPoint::reset()
{
   Bandwidth     = 0;
   BandwidthCost = 0.0;
   Utilization   = 0.0;
   FrameRate     = 0.0;
   Layers        = 0;
   for(cardinal i = 0;i < RTPConstants::RTPMaxQualityLayers;i++) {
      LayerBandwidthInfo[i].reset();
   }
}


// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, const ResourceUtilizationPoint& rup)
{
   char str[300 + (RTPConstants::RTPMaxQualityLayers * 64)];
   char lstr[64];

   // ====== Print header ===================================================
   const cardinal frameRate = (cardinal)rint(rup.FrameRate * 10.0);
   snprintf((char*)&str, 256,
            "U=%1.3f B=%7Ld C=%8.0f FR=%2d.%d   (",
            rup.Utilization, (card64)rup.Bandwidth, rup.BandwidthCost,
            (unsigned int)frameRate / 10, ((unsigned int)frameRate % 10));

   // ====== Print each layer's bandwidth ===================================
   for(cardinal i = 0;i < rup.Layers;i++) {
      snprintf((char*)&lstr, sizeof(lstr) - 2, "L%d=%7Ld/%02d",
               (unsigned int)i,
               (card64)rup.LayerBandwidthInfo[i].BytesPerSecond,
               (unsigned int)rup.LayerBandwidthInfo[i].BufferDelay);
      if(i < (rup.Layers - 1)) {
         strcat((char*)&lstr,", ");
      }
      strcat((char*)&str,(char*)&lstr);
   }

   os << str << ")";
   return(os);
}


// ###### Sort resource/utilization list ####################################
void ResourceUtilizationPoint::sortResourceUtilizationList(
                                  ResourceUtilizationPoint* rup,
                                  const integer            start,
                                  const integer            end)
{
   const double v = rup[(start + end) / 2].Utilization;
   integer i      = start;
   integer j      = end;

   do {
      while(rup[i].Utilization < v) i++;
      while(rup[j].Utilization > v) j--;
      if(i <= j) {
         swapResourceUtilizationPoints(rup[i],rup[j]);
         i++;
         j--;
      }
   } while(j >= i);

   if(start < j) {
      sortResourceUtilizationList(rup,start,j);
   }
   if(i < end) {
      sortResourceUtilizationList(rup,i,end);
   }
}


// ###### Optimize resource/utilization list ################################
cardinal ResourceUtilizationPoint::optimizeResourceUtilizationList(
                                      ResourceUtilizationPoint* rup,
                                      const cardinal            count)
{
   // ====== No optimization necessary for <= 1 points ======================
   if(count <= 1) {
      return(count);
   }

   // ====== Mark points to skip ============================================
   bool skip[count];
   card64 resource = rup[count - 1].Bandwidth;
   double cost     = rup[count - 1].BandwidthCost;
   skip[count - 1] = false;
   for(integer i = (integer)count - 2;i >= 0;i--) {
      if((resource < rup[i].Bandwidth) ||
         (cost     < rup[i].BandwidthCost))
       {
         skip[i] = true;
      }
      else {
         skip[i]  = false;
         resource = rup[i].Bandwidth;
         cost     = rup[i].BandwidthCost;
      }
   }

   // ====== Generate optimized list ========================================
   cardinal newCount = 0;
   for(cardinal i = 0;i < count;i++) {
      if((skip[i] == false) && (i != count)) {
         rup[newCount] = rup[i];
         newCount++;
      }
   }

   return(newCount);
}


// ###### Compute convex hull using Graham Scan algorithm ###################
cardinal ResourceUtilizationPoint::grahamScanResourceUtilizationList(
                                      ResourceUtilizationPoint* rup,
                                      const cardinal            count)
{
   // ResourceUtilizationPoint:
   //    x = Resource (Bandwidth)
   //    y = Utilization

   cardinal i;
   cardinal min;
   cardinal M;

   for(min = 0, i = 1;i < count;i++) {
      if(rup[i].Utilization < rup[min].Utilization) {
         min = i;
      }
   }

   for(i = 0;i < count;i++) {
      if(rup[i].Utilization == rup[min].Utilization) {
         if(rup[i].Bandwidth > rup[min].Bandwidth) {
            min = i;
         }
      }
   }
   swapResourceUtilizationPoints(rup[0],rup[min]);
   sortResourceUtilizationList(rup,0,count - 1);

   for(M = 2, i = 3; i < count;i++) {
      while(ccw(rup[M - 1],rup[M],rup[i]) >= 0) {
         M--;
         if(M == 0) {
            break;
         }
      }
      M++;
      swapResourceUtilizationPoints(rup[i],rup[M]);
   }
   return(M + 1);
}


// ###### Merge resource/utilization lists ##################################
cardinal ResourceUtilizationPoint::mergeResourceUtilizationLists(
                                       ResourceUtilizationPoint*  destination,
                                       ResourceUtilizationPoint** listArray,
                                       const cardinal*            listSizeArray,
                                       const cardinal             listCount)
{
   cardinal pos = 0;
   for(cardinal i = 0;i < listCount;i++) {
      for(cardinal j = 0;j < listSizeArray[i];j++) {
         destination[pos] = listArray[i][j];
         pos++;
      }
   }

   sortResourceUtilizationList(destination,0,pos - 1);
   return(pos);
}
