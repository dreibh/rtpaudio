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


#ifndef GENERICFRAMESIZESCALABILITY_H
#define GENERICFRAMESIZESCALABILITY_H


#include "tdsystem.h"
#include "framesizescalabilityinterface.h"


/**
  * This class is a generic implementation of FrameSizeScalabilityInterface.
  * It provides basic functionality for subclasses.
  * Important node: All frames sizes in this class are payload frame sizes!
  *
  * @short   Generic Frame Size Scalability
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class GenericFrameSizeScalability : virtual public FrameSizeScalabilityInterface
{
   // ====== Frame size methods =============================================
   public:
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#isValidPayloadFrameSize
     */
   bool isValidPayloadFrameSize(const double   frameRate,
                                const cardinal bufferDelay,
                                const cardinal frameSize) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getNearestValidPayloadFrameSize
     */
   cardinal getNearestValidPayloadFrameSize(const double   frameRate,
                                            const cardinal bufferDelay,
                                            const cardinal frameSize) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getNextFrameSizeForDelayAndSize
     */
   cardinal getNextPayloadFrameSizeForDelayAndSize(const double   frameRate,
                                                   const cardinal bufferDelay,
                                                   const cardinal frameSize) const;
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getPrevFrameSizeForDelayAndSize
     */
   cardinal getPrevPayloadFrameSizeForDelayAndSize(const double   frameRate,
                                                   const cardinal bufferDelay,
                                                   const cardinal frameSize) const;


   // ====== Scaling and utilization methods ================================
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getPayloadFrameSizeScaleFactorForDelayAndSize
     */
   double getPayloadFrameSizeScaleFactorForDelayAndSize(const double   frameRate,
                                                        const cardinal bufferDelay,
                                                        const cardinal frameSize) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getPayloadFrameSizeUtilizationForDelayAndSize
     */
   double getPayloadFrameSizeUtilizationForDelayAndSize(const double   frameRate,
                                                        const cardinal bufferDelay,
                                                        const cardinal frameSize) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getFrameSizeUtilizationWeight
     */
   double getFrameSizeUtilizationWeight(const double frameRate) const;


   // ====== Buffer delay methods ===========================================
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getNextBufferDelayForDelay
     */
   cardinal getNextBufferDelayForDelay(const double frameRate,
                                       const cardinal bufferDelay) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getPrevBufferDelayForDelay
     */
   cardinal getPrevBufferDelayForDelay(const double frameRate,
                                       const cardinal bufferDelay) const;
};


#endif
