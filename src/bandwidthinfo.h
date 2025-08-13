// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Bandwidth Info                                                   ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2025 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: thomas.dreibholz@gmail.com                             ####
// ####    WWW:   https://www.nntb.no/~dreibh/rtpaudio                   ####
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


#ifndef BANDWIDTHINFO_H
#define BANDWIDTHINFO_H


#include "tdsystem.h"


/**
  * This is a description of bandwidth requirements.
  *
  * @short   Bandwidth Info
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
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
