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
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de.de                               ####
// ####    WWW:   http://www.iem.uni-due.de.de/~dreibh/rn                ####
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


#include "tdsystem.h"
#include "cbrframesizescalability.h"


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
      std::cerr << "ERROR: ConstantBitrateFrameSizeScalability::initConstantBitratePayloadFrameSizeScalability() - " << std::endl
                << "Invalid parameter " << scaleFactor << "!" << std::endl;
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
