// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Constant Bitrate Frame Size Scalability                          ####
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
#include "cbrframesizescalability.h"


namespace Coral {


// ###### Constructor #######################################################
ConstantBitrateFrameSizeScalability::ConstantBitrateFrameSizeScalability()
{
   MinFrameSize = (cardinal)-1;
   MaxFrameSize = (cardinal)-1;
}


// ###### Desstructor #######################################################
ConstantBitrateFrameSizeScalability::~ConstantBitrateFrameSizeScalability()
{
}


// ###### Initialize ########################################################
void ConstantBitrateFrameSizeScalability::initConstantBitrateFrameSizeScalability(
                                             const cardinal maxFrameSize,
                                             const double   scaleFactor)
{
   if((scaleFactor < 0.0) || (scaleFactor > 1.0)) {
      cerr << "ERROR: ConstantBitrateFrameSizeScalability::initConstantBitratePayloadFrameSizeScalability() - " << endl
           << "Invalid parameter " << scaleFactor << "!" << endl;
      exit(1);
   }

   MinFrameSize  = (cardinal)ceil(scaleFactor * (double)maxFrameSize);
   MaxFrameSize = maxFrameSize;
}


// ###### Get frame size scalability class name #############################
const char* ConstantBitrateFrameSizeScalability::getFrameSizeScalabilityClass() const
{
   return((const char*)" Constant Bitrate Frame-size Scalability");
}


// ###### Check, if frame size is scalable ##################################
bool ConstantBitrateFrameSizeScalability::isFrameSizeScalable() const
{
   return(true);
}


// ###### Check, if frame size is variable bit rate #########################
bool ConstantBitrateFrameSizeScalability::isVariableBitrate() const
{
   return(false);
}


// ###### Get minimum frame size ############################################
cardinal ConstantBitrateFrameSizeScalability::getMinPayloadFrameSizeForDelay(
                                                 const double   frameRate,
                                                 const cardinal bufferDelay) const
{
   return(MinFrameSize);
}


// ###### Get maxmum frame size ############################################
cardinal ConstantBitrateFrameSizeScalability::getMaxPayloadFrameSizeForDelay(
                                                 const double   frameRate,
                                                 const cardinal bufferDelay) const
{
   return(MaxFrameSize);
}


// ###### Get maximum frame count ###########################################
cardinal ConstantBitrateFrameSizeScalability::getMaxFrameCountForDelay(
                                                 const double   frameRate,
                                                 const cardinal bufferDelay) const
{
   return(bufferDelay);
}


// ###### Get maximum buffer delay ##########################################
cardinal ConstantBitrateFrameSizeScalability::getMaxBufferDelay(
                                                 const double frameRate) const
{
   return(1);
}


}
