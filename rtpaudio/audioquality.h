// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Quality                                                    ####
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


#ifndef AUDIOQUALITY_H
#define AUDIOQUALITY_H


#include "tdsystem.h"
#include "randomizer.h"
#include "audioqualityinterface.h"


/**
  * This class manages audio quality.
  *
  * @short   Audio Quality
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioQuality : virtual public AdjustableAudioQualityInterface
{
   // ====== Constructors ===================================================
   public:
   /**
     * Default constructor.
     */
   AudioQuality();

   /**
     * Constructor for new AudioQuality object with given quality
     *
     * @param samplingRate SamplingRate.
     * @param bits Number of bits.
     * @param channels Number of channels.
     * @param byteOrder Byte order: BIG_ENDIAN, LITTLE_ENDIAN.
     */
   AudioQuality(const card16 samplingRate,
                const card8  bits,
                const card8  channels,
                const card16 byteOrder = BYTE_ORDER);

   /**
     * Constructor for new AudioQuality object from given AudioQualityInterface
     *
     * @param quality AudioQualityInterface.
     */
   AudioQuality(const AudioQualityInterface& quality);


   // ====== Quality functions ==============================================
   /**
     * getSamplingRate() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getSamplingRate
     */
   card16 getSamplingRate() const;

   /**
     * getBits() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getBits
     */
   card8 getBits() const;

   /**
     * getChannels() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getChannels
     */
   card8 getChannels() const;


   /**
     * getByteOrder() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getByteOrder
     */
   card16 getByteOrder() const;


   /**
     * getBytesPerSecond() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getBytesPerSecond
     */
   cardinal getBytesPerSecond() const;

   /**
     * getBitsPerSample() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getBitsPerSample
     */
   cardinal getBitsPerSample() const;


   /**
     * setSamplingRate() Implementation of AdjustableAudioQualityInterface.
     *
     * @see AdjustableAudioQualityInterface#setSamplingRate
     */
   card16 setSamplingRate(const card16 samplingRate);

   /**
     * setBits() Implementation of AdjustableAudioQualityInterface.
     *
     * @see AdjustableAudioQualityInterface#setBits
     */
   card8 setBits(const card8 bits);

   /**
     * setChannels() Implementation of AdjustableAudioQualityInterface.
     *
     * @see AdjustableAudioQualityInterface#setChannels
     */
   card8 setChannels(const card8 channels);


   /**
     * setByteOrder() Implementation of AdjustableAudioQualityInterface.
     *
     * @see AdjustableAudioQualityInterface#setByteOrder
     */
   card16 setByteOrder(const card16 byteOrder);


   /**
     * Check, if quality is lowest quality.
     *
     * @return true, if quality is lowest; false otherwise.
     */
   inline bool isLowest() const;

   /**
     * Check, if quality is highest quality.
     *
     * @return true, is quality is highest; false otherwise.
     */
   inline bool isHighest() const;


   /**
     * Increase quality by given number of steps. The number of steps available
     * is given by QualityLevels constant.
     *
     * @param steps Number of steps.
     */
   void increase(const cardinal steps);

   /**
     * Decrease quality by given number of steps. The number of steps available
     * is given by QualityLevels constant.
     *
     * @param steps Number of steps.
     */
   void decrease(const cardinal setps);


   /**
     * Set sampling rate to next lower value.
     *
     * @return true, if sampling rate has been set; false, if it was already lowest.
     */
   bool prevSamplingRate();

   /**
     * Set sampling rate to next higher value.
     *
     * @return true, if sampling rate has been set; false, if it was already highest.
     */
   bool nextSamplingRate();


   /**
     * Convert bytes to microseconds.
     */
   inline card64 bytesToTime(const size_t bytes) const;

   /**
     * Convert microseconds to bytes.
     */
   inline size_t timeToBytes(const card64 microseconds) const;


   // ====== Operators ======================================================
   /**
     * Implementation of = operator.
     */
   AudioQuality& operator=(const AudioQualityInterface& quality);

   /**
     * Implementation of ++ operator.
     */
   AudioQuality operator++(int);

   /**
     * Implementation of -- operator.
     */
   AudioQuality operator--(int);


   // ====== Static functions ===============================================
   public:
   /**
     * Get maximum audio quality for a given byte rate.
     *
     * @param bps Bytes per second.
     * @return AudioQuality.
     */
   static AudioQuality getQualityForByteRate(const cardinal bps);

   /**
     * Get a random quality setting. All settings have the same probability.
     *
     * @return Random quality setting.
     */
   static AudioQuality getRandomQuality(Randomizer* randomizer);


   /**
     * Table with valid sampling rate values.
     */
   static const card16* ValidRatesTable;

   /**
     * Number of valid sampling rates in ValidRatesTable.
     */
   static const cardinal ValidRates;

   /**
     * Table with valid bit values.
     */
   static const card8* ValidBitsTable;

   /**
     * Number of valid bits values in ValidRatesTable.
     */
   static const cardinal ValidBits;

   /**
     * Table with valid channel values.
     */
   static const card8* ValidChannelsTable;

   /**
     * Number of valid channels values in ValidRatesTable.
     */
   static const cardinal ValidChannels;


   /**
     * Constant for lowest quality.
     */
   static const AudioQuality LowestQuality;

   /**
     * Constant for highest quality.
     */
   static const AudioQuality HighestQuality;

   /**
     * Constant for lowest sampling rate.
     */
   static const card16 LowestSamplingRate;

   /**
     * Constant for highest sampling rate.
     */
   static const card16 HighestSamplingRate;

   /**
     * Constant for lowest number of bits.
     */
   static const card8 LowestBits;

   /**
     * Constant for highest number of bits.
     */
   static const card8 HighestBits;

   /**
     * Constant for lowest number of channels.
     */
   static const card8 LowestChannels;

   /**
     * Constant for highest number of channels.
     */
   static const card8 HighestChannels;

   /**
     * Number of quality levels supported by operator++/operator--.
     */
   static const cardinal QualityLevels = 23;


   // ====== Private data ===================================================
   private:
   card16 SamplingRate;
   card8  Bits;
   card8  Channels;
   card16 ByteOrder;
};


/**
  * Implementation of << operator.
  */
std::ostream& operator<<(std::ostream& out, const AudioQualityInterface& quality);

/**
  * Implementation of + operator.
  */
AudioQuality operator+(const AudioQualityInterface& q1, const AudioQualityInterface& q2);

/**
  * Implementation of - operator.
  */
AudioQuality operator-(const AudioQualityInterface& q1, const AudioQualityInterface& q2);

/**
  * Implementation of - operator.
  * Limits resulting audio quality by a given byte rate.
  */
AudioQuality operator-(const AudioQualityInterface& q1, const cardinal bytesPerSecond);


#include "audioquality.icc"


#endif
