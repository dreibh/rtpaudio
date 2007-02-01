// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Bandwidth Info                                                   ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#ifndef BANDWIDTHINFO_H
#define BANDWIDTHINFO_H


#include "tdsystem.h"


/**
  * This is a description of bandwidth requirements.
  *
  * @short   Bandwidth Info
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
  */
struct BandwidthInfo
{
   // ====== Public data ====================================================
   public:
   /**
     * Buffer delay in microseconds.
     */
   cardinal BufferDelay;

   /**
     * Bytes per second.
     */
   card64 BytesPerSecond;

   /**
     * Packets per second.
     */
   cardinal PacketsPerSecond;

   /**
     * Maximum transfer delay.
     */
   double MaxTransferDelay;

   /**
     * Maximum loss rate.
     */
   double MaxLossRate;

   /**
     * Maximum jitter.
     */
   double MaxJitter;


   // ====== Comparision operators ==========================================
   /**
     * Reset.
     */
   void reset();

   /**
     * Operator "==".
     */
   inline int operator==(const BandwidthInfo& rup) const;

   /**
     * Operator "!=".
     */
   inline int operator!=(const BandwidthInfo& rup) const;
};


/**
  * Operator "<<".
  */
std::ostream& operator<<(std::ostream& os, const BandwidthInfo& bi);


#include "bandwidthinfo.icc"


#endif
