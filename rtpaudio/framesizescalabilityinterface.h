// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Frame Size Scalability Interface                                 ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
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


#ifndef FRAMESIZESCALABILITYINTERFACE_H
#define FRAMESIZESCALABILITYINTERFACE_H


#include "tdsystem.h"


/**
  * This class is an interface for frame size scalability.
  * Important node: All frames sizes in this class are payload frame sizes!
  *
  * @short   Frame Rate Scalability Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class FrameSizeScalabilityInterface
{
   // ====== Scalability information methods ================================
   public:
   /**
     * Get name of the frame size scalability class.
     *
     * @return Frame size scalability class name.
     */
   virtual const char* getFrameSizeScalabilityClass() const = 0;

   /**
     * Check, if frame size is scalable.
     *
     * @return true, if frame size is scalable; false otherwise.
     */
   virtual bool isFrameSizeScalable() const = 0;

   /**
     * Check, if frame size is variable bitrate (frame sizes are different
     * for each frame; the frame size given is the frame size necessary
     * to be reserved for a given buffer delay).
     *
     * @return true, if frame size is variable bitrate; false otherwise.
     */
   virtual bool isVariableBitrate() const = 0;


   // ====== Frame size methods =============================================
   /**
     * Get minimum payload frame size for given buffer delay (in frame rate units).
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @return Minimum payload frame size.
     */
   virtual cardinal getMinPayloadFrameSizeForDelay(
                       const double   frameRate,
                       const cardinal bufferDelay) const = 0;

   /**
     * Get maximum payload frame size for given buffer delay (in frame rate units).
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @return Maximum payload frame size.
     */
   virtual cardinal getMaxPayloadFrameSizeForDelay(
                       const double   frameRate,
                       const cardinal bufferDelay) const = 0;

   /**
     * Get maximum number of frames for given buffer delay (in frame rate units).
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @return Maximum number of frames.
     */
   virtual cardinal getMaxFrameCountForDelay(
                       const double   frameRate,
                       const cardinal bufferDelay) const = 0;

   /**
     * Check, if given payload frame size is a valid value for given buffer delay
     * (in frame rate units).
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @param frameSize Payload frame size to be checked.
     * @return true, if given size is valid; false otherwise.
     */
   virtual bool isValidPayloadFrameSize(
                   const double   frameRate,
                   const cardinal bufferDelay,
                   const cardinal frameSize) const = 0;

   /**
     * Get nearest lower valid payload frame rate for given frame rate for given buffer delay
     * (in frame rate units).
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @param frameSize Payload frame size.
     * @return Valid payload frame size nearest to given size for given buffer delay.
     */
   virtual cardinal getNearestValidPayloadFrameSize(
                       const double   frameRate,
                       const cardinal bufferDelay,
                       const cardinal frameSize) const = 0;

   /**
     * Get next higher valid payload frame size for given buffer delay
     * (in frame rate units) and payload frame size.
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @param frameSize Payload frame size.
     * @return Next higher valid payload frame size for given buffer delay.
     */
   virtual cardinal getNextPayloadFrameSizeForDelayAndSize(
                       const double   frameRate,
                       const cardinal bufferDelay,
                       const cardinal frameSize) const = 0;

   /**
     * Get next lower valid payload frame size for given buffer delay
     * (in frame rate units) and frame size.
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @param frameSize Payload frame size.
     * @return Next lower valid payload frame size for given buffer delay.
     */
   virtual cardinal getPrevPayloadFrameSizeForDelayAndSize(
                        const double   frameRate,
                        const cardinal bufferDelay,
                        const cardinal frameSize) const = 0;


   // ====== Scaling and utilization methods ================================
   /**
     * Get scale factor for given buffer delay
     * (in frame rate units) and payload frame size:
     * (rate - MinFrameSize) / (MaxFrameRate - MinFrameSize)
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Scale factor (out of [0,1])..
     */
   virtual double getPayloadFrameSizeScaleFactorForDelayAndSize(
                     const double   frameRate,
                     const cardinal bufferDelay,
                     const cardinal frameSize) const = 0;

   /**
     * Get utilization for given buffer delay
     * (in frame rate units) and payload frame size.
     *
     * @param frameRate Frame rate.
     * @param frameSize Payload frame size.
     * @return Utilization (out of [0,1])..
     */
   virtual double getPayloadFrameSizeUtilizationForDelayAndSize(
                     const double   frameRate,
                     const cardinal bufferDelay,
                     const cardinal frameSize) const = 0;

   /**
     * Get frame size utilization weight.
     *
     * @param frameRate Frame rate.
     * @return Utilization weight.
     */
   virtual double getFrameSizeUtilizationWeight(const double frameRate) const = 0;


   // ====== Buffer delay methods ===========================================
   /**
     * Get maximum buffer delay. The *minimum* buffer delay is always 1.
     *
     * @param frameRate Frame rate.
     * @return Maximum buffer delay.
     */
   virtual cardinal getMaxBufferDelay(const double frameRate) const = 0;

   /**
     * Get next higher valid buffer delay for given buffer delay.
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @return Next higher valid buffer delay.
     */
   virtual cardinal getNextBufferDelayForDelay(const double   frameRate,
                                               const cardinal bufferDelay) const = 0;

   /**
     * Get next lower valid buffer delay for given buffer delay.
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay in frame rate units.
     * @return Next lower valid buffer delay.
     */
   virtual cardinal getPrevBufferDelayForDelay(const double   frameRate,
                                               const cardinal bufferDelay) const = 0;
};


#endif
