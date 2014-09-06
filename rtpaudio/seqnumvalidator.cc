// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Sequence Number Validator Implementation                         ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2014 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "seqnumvalidator.h"
#include "rtppacket.h"
#include "tools.h"


// ###### Constructor #######################################################
SeqNumValidator::SeqNumValidator(const cardinal minSequential,
                                 const cardinal maxMisorder,
                                 const cardinal maxDropout,
                                 const card64   seqMod)
{
   SeqMod        = seqMod;
   MaxDropout    = maxDropout;
   MaxMisorder   = maxMisorder;
   MinSequential = minSequential;
   reset();
}


// ###### Reset SeqNumValidator #############################################
void SeqNumValidator::reset()
{
   Jitter                = 0.0;
   PrevPacketTimeStamp   = 0;
   PrevPacketArrivalTime = 0;
   Jitter                = 0.0;
   FractionLost          = 0.0;

   const card64 sequenceNumber = 0;
   MaxSeq        = sequenceNumber - 1;
   Probation     = MinSequential;
   Received      = 0;
   ReceivedPrior = 0;
   ExpectedPrior = 0;
   Uninitialized = true;
   init(sequenceNumber);
}


// ###### Validate sequence number ##########################################
SeqNumValidator::ValidationResult SeqNumValidator::validate(const card64 sequenceNumber,
                                                            const card32 packetTimeStamp)
{
   const card64 udelta = (sequenceNumber - MaxSeq) % SeqMod;

   // ====== Sequence number check based on algorithm in RFC 1889 ===========
   // Source is not valid until MinSequential packets with
   // sequential sequence numbers have been Received.
   if(Probation) {
      // packet is in sequence
      if(sequenceNumber == MaxSeq + 1) {
         Probation--;
         MaxSeq = sequenceNumber;
         if(Probation == 0) {
            init(sequenceNumber);
            Received++;

            if(Uninitialized) {
               Uninitialized = false;
               Received      = ((Cycles + MaxSeq) - BaseSeq + 1);
            }

            PrevPacketArrivalTime = getMicroTime();
            PrevPacketTimeStamp   = (card64)rint((double)packetTimeStamp *
                                                 RTPConstants::RTPMicroSecondsPerTimeStamp);
            Jitter                = 0.0;
            return(Valid);
         }
      }
      else {
         MaxSeq    = sequenceNumber;
         Probation = MinSequential;
      }
      return(SourceProbation);
   }
   else if(udelta == 0) {
      // duplicate packet
      return(DuplicatePacket);
   }
   else if(udelta < MaxDropout) {
      /* in order, with permissible gap */
      if(sequenceNumber < MaxSeq) {
         /*
	  * Sequence number wrapped - count another 64K cycle.
	  */
         Cycles = Cycles + SeqMod;
      }
      MaxSeq = sequenceNumber;
   }
   else if(udelta <= SeqMod - MaxMisorder) {
      /* the sequence number made a very large jump */
      if(sequenceNumber == BadSeq) {
        /*
         * Two sequential packets -- assume that the other side
         * restarted without telling us so just re-sync
         * (i.e., pretend this was the first packet).
         */
        init(sequenceNumber);
        Received = 1;
        return(Jumped);
     } else {
        BadSeq = (sequenceNumber + 1) & (SeqMod - 1);
        return(InvalidSeqNum);
     }
   } else {
      // duplicate or reordered packet

   }
   Received++;

   // ====== Calculate jitter ===============================================
   if(packetTimeStamp != 0) {
      const card64 now = getMicroTime();
      if(PrevPacketArrivalTime != 0) {
         const double diff = fabs(((double)now - ((double)packetTimeStamp * RTPConstants::RTPMicroSecondsPerTimeStamp)) -
                                   ((double)PrevPacketArrivalTime - (double)PrevPacketTimeStamp));
         Jitter += (1.0/16.0) * (diff - Jitter);
      }
      PrevPacketArrivalTime = now;
      PrevPacketTimeStamp   = (card64)rint((double)packetTimeStamp *
                                           RTPConstants::RTPMicroSecondsPerTimeStamp);
   }

   return(Valid);
}


// ###### Calculate fraction lost ###########################################
double SeqNumValidator::calculateFractionLost()
{
   if(Uninitialized)
      return(0);

   const card64 expected          = (Cycles + MaxSeq) - BaseSeq + 1;
   const card64 expected_interval = expected - ExpectedPrior;
   const card64 Received_interval = Received - ReceivedPrior;

   ExpectedPrior = expected;
   ReceivedPrior = Received;

   card64 lost_interval = expected_interval - Received_interval;

   if((expected_interval == 0) || (lost_interval <= 0))
      FractionLost = 0.0;
   else
      FractionLost = (double)lost_interval / (double)expected_interval;

   return(FractionLost);
}
