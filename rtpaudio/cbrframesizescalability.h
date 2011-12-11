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


#ifndef CBRFRAMESIZESCALABILITY_H
#define CBRFRAMESIZESCALABILITY_H


#include "tdsystem.h"
#include "framesizescalabilityinterface.h"
#include "genericframesizescalability.h"


/**
  * This class is an implementation of FrameSizeScalabilityInterface.
  * Important node: All frames sizes in this class are payload frame sizes!
  *
  * @short   Constant Bitrate Frame Size Scalability
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
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


#endif
