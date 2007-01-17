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


#ifndef CBRFRAMESIZESCALABILITY_H
#define CBRFRAMESIZESCALABILITY_H


#include "tdsystem.h"
#include "framesizescalabilityinterface.h"
#include "genericframesizescalability.h"


namespace Coral {


/**
  * This class is an implementation of FrameSizeScalabilityInterface.
  * Important node: All frames sizes in this class are payload frame sizes!
  *
  * @short   Constant Bitrate Frame Size Scalability
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
  */
class ConstantBitrateFrameSizeScalability
   : public GenericFrameSizeScalability
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   ConstantBitrateFrameSizeScalability();

   /**
     * Desstructor.
     */
   virtual ~ConstantBitrateFrameSizeScalability();


   // ====== Initialization =================================================
   /**
     * Initialize object with new maximum payload frame size and scale factor.
     * MinFrameSize = scaleFactor * MaxFrameSize.
     *
     * @param maxFrameSize Maximum payload frame size.
     * @param scaleFactor Scale factor.
     */
   void initConstantBitrateFrameSizeScalability(
           const cardinal maxFrameSize,
           const double   scaleFactor);


   // ====== Scalability information methods ================================
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getFrameSizeScalabilityClass
     */
   const char* getFrameSizeScalabilityClass() const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#isFrameSizeScalable
     */
   bool isFrameSizeScalable() const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#isVariableBitRate
     */
   bool isVariableBitrate() const;


   // ====== Frame size methods =============================================
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getMinPayloadFrameSizeForDelay
     */
   cardinal getMinPayloadFrameSizeForDelay(const double   frameRate,
                                           const cardinal bufferDelay) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getMaxPayloadFrameSizeForDelay
     */
   cardinal getMaxPayloadFrameSizeForDelay(const double   frameRate,
                                           const cardinal bufferDelay) const;

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getMaxFrameCountForDelay
     */
   cardinal getMaxFrameCountForDelay(const double   frameRate,
                                     const cardinal bufferDelay) const;


   // ====== Buffer delay methods ===========================================
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getMaxBufferDelay
     */
   cardinal getMaxBufferDelay(const double frameRate) const;


   // ====== Protected data =================================================
   protected:
   cardinal MinFrameSize;
   cardinal MaxFrameSize;
};


}


#endif
