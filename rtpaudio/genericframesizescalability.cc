// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Generic Frame Size Scalability                                   ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### Copyright (C) 2000 Thomas Dreibholz                              ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#include "tdsystem.h"
#include "genericframesizescalability.h"


// ###### Check, if frame size is valid #####################################
bool GenericFrameSizeScalability::isValidPayloadFrameSize(
                                     const double   frameRate,
                                     const cardinal bufferDelay,
                                     const cardinal frameSize) const
{
   return((frameSize >= getMinPayloadFrameSizeForDelay(frameRate,bufferDelay)) &&
          (frameSize <= getMaxPayloadFrameSizeForDelay(frameRate,bufferDelay)));
}


// ###### Get nearest valid frame size ######################################
cardinal GenericFrameSizeScalability::getNearestValidPayloadFrameSize(
                                         const double   frameRate,
                                         const cardinal bufferDelay,
                                         const cardinal frameSize) const
{
   const cardinal minFrameSize = getMinPayloadFrameSizeForDelay(frameRate,bufferDelay);
   const cardinal maxFrameSize = getMaxPayloadFrameSizeForDelay(frameRate,bufferDelay);

   if(frameSize < minFrameSize) {
      return(minFrameSize);
   }
   else if(frameSize > maxFrameSize) {
      return(maxFrameSize);
   }
   return(frameSize);
}


// ###### Get next frame size ###############################################
cardinal GenericFrameSizeScalability::getNextPayloadFrameSizeForDelayAndSize(
                                         const double   frameRate,
                                         const cardinal bufferDelay,
                                         const cardinal frameSize) const
{
   const cardinal maxFrameSize = getMaxPayloadFrameSizeForDelay(frameRate,bufferDelay);

   if(frameSize < maxFrameSize) {
      return(frameSize + 1);
   }
   else {
      return(maxFrameSize);
   }
}


// ###### Get previous frame size ###########################################
cardinal GenericFrameSizeScalability::getPrevPayloadFrameSizeForDelayAndSize(
                                         const double   frameRate,
                                         const cardinal bufferDelay,
                                         const cardinal frameSize) const
{
   const cardinal minFrameSize = getMinPayloadFrameSizeForDelay(frameRate,bufferDelay);

   if(frameSize > minFrameSize) {
      return(frameSize - 1);
   }
   else {
      return(minFrameSize);
   }
}


// ###### Get frame size scale factor #######################################
double GenericFrameSizeScalability::getPayloadFrameSizeScaleFactorForDelayAndSize(
                                       const double   frameRate,
                                       const cardinal bufferDelay,
                                       const cardinal frameSize) const
{
   const cardinal minFrameSize = getMinPayloadFrameSizeForDelay(frameRate,bufferDelay);
   const cardinal maxFrameSize = getMaxPayloadFrameSizeForDelay(frameRate,bufferDelay);
   if(frameSize < minFrameSize) {
      return(-HUGE_VAL);
   }
   else if(frameSize >= maxFrameSize) {
      return(1.0);
   }
   else if(minFrameSize != maxFrameSize) {
      return((double)(std::min(frameSize,maxFrameSize) - minFrameSize) /
                (double)(maxFrameSize - minFrameSize));
   }
   return(0.0);
}


// ###### Get frame size utilization ########################################
double GenericFrameSizeScalability::getPayloadFrameSizeUtilizationForDelayAndSize(
                                       const double   frameRate,
                                       const cardinal bufferDelay,
                                       const cardinal frameSize) const
{
   return(getPayloadFrameSizeScaleFactorForDelayAndSize(frameRate,bufferDelay,frameSize));
}


// ###### Get frame rate utilization ########################################
double GenericFrameSizeScalability::getFrameSizeUtilizationWeight(const double frameRate) const
{
   return(1.0);
}


// ###### Get next buffer delay #############################################
cardinal GenericFrameSizeScalability::getNextBufferDelayForDelay(
                                         const double   frameRate,
                                         const cardinal bufferDelay) const
{
   if(bufferDelay < getMaxBufferDelay(frameRate)) {
      return(bufferDelay + 1);
   }
   return(bufferDelay);
}


// ###### Get previous buffer delay #########################################
cardinal GenericFrameSizeScalability::getPrevBufferDelayForDelay(
                                         const double   frameRate,
                                         const cardinal bufferDelay) const
{
   if(bufferDelay > 1) {
      return(bufferDelay - 1);
   }
   return(1);
}
