// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Sequence Number Validator                                        ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2023 by Thomas Dreibholz            ####
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


#ifndef SEQNUMVALIDATOR_H
#define SEQNUMVALIDATOR_H


#include "tdsystem.h"


/**
  * This class is a validator for sequence numbers. It is based on the algorithm
  * described in RFC 1889. It can use sequence numbers up to a size of 64 bits.
  * Jitter and fraction loss calculation is also done by this class.
  *
  * @short   Sequence Number Validator
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
class SeqNumValidator
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor for new sequence number validator.
     *
     * @param minSequential Minimum number of packets in sequence for the source to be valid.
     * @param maxMisorder Maximum difference for packets to be misordered.
     * @param maxDropout Maximum gap.
     * @param seqMod Sequence number modulo.
     */
   SeqNumValidator(const cardinal minSequential = 2,
                   const cardinal maxMisorder   = 100,
                   const cardinal maxDropout    = 3000,
                   const card64   seqMod        = (1 << 16));


   // ====== Status functions ===============================================
   /**
     * Get number of packets received.
     *
     * @return Number of packets received.
     */
   inline card64 getPacketsReceived() const;

   /**
     * Get number of packets lost. The loss is calculated by the number of
     * sequence number cycles and gaps.
     *
     * @return Number of packets lost.
     */
   inline card64 getPacketsLost() const;

   /**
     * Get extended last sequence number. This number is extended by the
     * calculated number of sequence number cycles!
     *
     * @return Last sequence number.
     */
   inline card64 getLastSeqNum() const;

   /**
     * Get fraction of packets lost. Note: No calculation of the fraction
     * lost is done here! The fraction lost value is the value of the
     * last call of calculateFractionLost()!
     *
     * @return Fraction of packets lost.
     *
     * @see calculateFractionLost
     */
   inline double getFractionLost() const;

   /**
     * Get jitter.
     *
     * @return Jitter.
     */
   inline double getJitter() const;


   // ====== Sequence number validation =====================================
   enum ValidationResult {
      Valid           = 0,
      SourceProbation = 1,
      Jumped          = 2,
      Invalid         = 10,
      DuplicatePacket = Invalid + 0,
      InvalidSeqNum   = Invalid + 1
   };
   /**
     * Validate a new sequence number. If the packet is valid, jitter value
     * will be calculated using packetTimeStamp. To disable jitter calculation,
     * set packetTimeStamp to 0.
     *
     * @param sequenceNumber Sequence number to be validated.
     * @param packetTimeStamp Time stamp of the packet for jitter calculation.
     * @return ValidationResult containing result of validation.
     */
   ValidationResult validate(const card64 sequenceNumber,
                             const card32 packetTimeStamp = 0);

   /**
     * Reset SeqNumValidator.
     */
   void reset();

   /**
     * Calculate and get fraction of packets lost.
     *
     * @return Fraction lost.
     */
   double calculateFractionLost();


   // ====== Private data ===================================================
   private:
   inline void init(const card64 sequenceNumber);


   card64   SeqMod;                // Constants.
   cardinal MaxDropout;
   cardinal MaxMisorder;
   cardinal MinSequential;

   card64   PrevPacketTimeStamp;   // Time stamp of previous packet.
   card64   PrevPacketArrivalTime; // Arrival time of previous packet.
   double   Jitter;                // Estimated jitter.
   double   FractionLost;          // Fraction lost.

   card64   MaxSeq;                // Highest seq. number seen.
   card64   BaseSeq;               // Base seq. number.
   card64   BadSeq;                // Last 'bad' seq. number + 1.
   card32   Probation;             // Sequential packets till source is valid.

   card64   Cycles;                // Shifted count of seq. number cycles.
   card64   Received;              // Packets received.
   card64   ReceivedPrior;         // Packet received at last interval.
   card64   ExpectedPrior;         // Packet expected at last interval.

   bool     Uninitialized;         // Waiting for the first valid seq. number.
};


#include "seqnumvalidator.icc"


#endif
