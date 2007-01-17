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


#ifndef GENERICFRAMESIZESCALABILITY_H
#define GENERICFRAMESIZESCALABILITY_H


#include "tdsystem.h"
#include "framesizescalabilityinterface.h"


namespace Coral {


/**
  * This class is a generic implementation of FrameSizeScalabilityInterface.
  * It provides basic functionality for subclasses.
  * Important node: All frames sizes in this class are payload frame sizes!
  *
  * @short   Generic Frame Size Scalability
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
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


}


#endif
