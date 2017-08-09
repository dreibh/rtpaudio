// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Source State Info                                                ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   https://www.uni-due.de/~be0001/rtpaudio                ####
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


#ifndef SOURCESTATEINFO_H
#define SOURCESTATEINFO_H


#include "tdsystem.h"
#include "synchronizable.h"
#include "seqnumvalidator.h"


/**
  * This class manages the source state information of an RTP receiver to
  * be transmitted by a RTCPSender.
  * See also RFC 1889 for more information on RTP.
  *
  * @short   Source State Info
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  *
  * @see RTPReceiver
  * @see RTCPSender
  */
class SourceStateInfo : public SeqNumValidator,
                        public Synchronizable
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   SourceStateInfo();
   
   /**
     * Copy operation.
     */
   SourceStateInfo& operator=(const SourceStateInfo& original);

 
   // ====== Reset ==========================================================
   /**
     * Reset.
     */
   void reset();


   // ====== Status functions ===============================================
   /**
     * Get SSRC.
     *
     * @return SSRC.
     */
   inline card32 getSSRC() const;

   /**
     * Get last sender report time stamp.
     *
     * @return LSR.
     */
   inline card32 getLSR() const;

   /**
     * Calculate delay since last sender report time stamp using current time.
     *
     * @return DLSR.
     */
   card32 calculateDLSR() const;

   /**
     * Set last sender report time stamp.
     *
     * @param lsr LSR.
     */
   inline void setLSR(const card32 lsr);

   /**
     * Set SSRC.
     *
     * @return SSRC.
     */
   inline void setSSRC(card32 ssrc);


   // ====== Private data ===================================================
   private:
   card64 LSRUpdateTimeStamp;             // Timestamp of last setLSR() call

   card32 ReceivedPrior;                  // Packet received at last interval
   card32 ExpectedPrior;                  // Packet expected at last interval
   card32 FractionLost;                   // Fraction lost
   card32 SSRC;                           // Data source identifier
   card32 LSR;                            // Last SR packet from this source
};


#include "sourcestateinfo.icc"


#endif
