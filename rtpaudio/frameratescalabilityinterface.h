// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Frame Rate Scalability Interface                                 ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#ifndef FRAMERATESCALABILITYINTERFACE_H
#define FRAMERATESCALABILITYINTERFACE_H


#include "tdsystem.h"


/**
  * This class is an interface for frame rate scalability.
  *
  * @short   Frame Rate Scalability Interface
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
  */
class FrameRateScalabilityInterface
{
   // ====== Scalability information methods ================================
   public:
   /**
     * Destructor.
     */
   virtual ~FrameRateScalabilityInterface();

   /**
     * Get name of the frame rate scalability class.
     *
     * @return Frame rate scalability class name.
     */
   virtual const char* getFrameRateScalabilityClass() const = 0;

   /**
     * Check, if frame rate is scalable.
     *
     * @return true, if frame rate is scalable; false otherwise.
     */
   virtual bool isFrameRateScalable() const = 0;


   // ====== Frame rate methods =============================================
   /**
     * Get minimum frame rate.
     *
     * @return Minimum frame rate.
     */
   virtual double getMinFrameRate() const = 0;

   /**
     * Get maximum frame rate.
     *
     * @return Maximum frame rate.
     */
   virtual double getMaxFrameRate() const = 0;

   /**
     * Check, if given frame rate is a valid value.
     *
     * @param frameRate Frame rate to be checked.
     * @return true, if given rate is valid; false otherwise.
     */
   virtual bool isValidFrameRate(const double frameRate) const = 0;

   /**
     * Get nearest lower valid frame rate for given frame rate.
     *
     * @param rate Frame rate.
     * @return Valid frame rate nearest to given rate.
     */
   virtual double getNearestValidFrameRate(const double frameRate) const = 0;

   /**
     * Get next higher valid frame rate for given frame rate.
     *
     * @param frameRate Frame rate.
     * @return Next higher valid frame rate.
     */
   virtual double getNextFrameRateForRate(const double frameRate) const = 0;

   /**
     * Get next lower valid frame rate for given frame rate.
     *
     * @param frameRate Frame rate.
     * @return Next lower valid frame rate.
     */
   virtual double getPrevFrameRateForRate(const double frameRate) const = 0;


   // ====== Scaling and utilization methods ================================
   /**
     * Get scale factor for given frame rate:
     * (rate - MinFrameRate) / (MaxFrameRate - MinFrameRate)
     *
     * @param frameRate Frame rate.
     * @return Scale factor (out of [0,1])..
     */
   virtual double getFrameRateScaleFactorForRate(const double frameRate) const = 0;

   /**
     * Get utilization for given frame rate.
     *
     * @param frameRate Frame rate.
     * @return Utilization (out of [0,1]).
     */
   virtual double getFrameRateUtilizationForRate(const double frameRate) const = 0;

   /**
     * Get frame rate utilization weight.
     *
     * @param frameRate Frame rate.
     * @return Utilization weight.
     */
   virtual double getFrameRateUtilizationWeight(const double frameRate) const = 0;
};


#endif
