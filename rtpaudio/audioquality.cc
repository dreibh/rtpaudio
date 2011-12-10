// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Quality Implementation                                     ####
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


#include "tdsystem.h"
#include "audioquality.h"
#include "randomizer.h"


// ###### Constants #########################################################
static const card16 _ValidRatesTable[] = {
   4410,   6615,  8820, 11025,
   13230, 15435, 17640, 19845,
   22050, 24255, 26460, 28665,
   30870, 33075, 35280, 37485,
   39690, 41895, 44100
};
const card16* AudioQuality::ValidRatesTable = (const card16*)&_ValidRatesTable;
const cardinal AudioQuality::ValidRates =
   sizeof(_ValidRatesTable) / sizeof(card16);

static const card8 _ValidBitsTable[] = {
   4, 8, 12, 16
};
const card8* AudioQuality::ValidBitsTable = (const card8*)&_ValidBitsTable;

const cardinal AudioQuality::ValidBits =
   sizeof(_ValidBitsTable) / sizeof(card8);

static const card8 _ValidChannelsTable[] = {
   1, 2
};
const card8* AudioQuality::ValidChannelsTable = (const card8*)&_ValidChannelsTable;
const cardinal AudioQuality::ValidChannels =
   sizeof(_ValidChannelsTable) / sizeof(card8);


const card16 AudioQuality::LowestSamplingRate   = 4410;
const card16 AudioQuality::HighestSamplingRate  = 44100;
const card8  AudioQuality::LowestBits           = 4;
const card8  AudioQuality::HighestBits          = 16;
const card8  AudioQuality::LowestChannels       = 1;
const card8  AudioQuality::HighestChannels      = 2;
const AudioQuality AudioQuality::LowestQuality  = AudioQuality(4410,4,1,BYTE_ORDER);
const AudioQuality AudioQuality::HighestQuality = AudioQuality(44100,16,2,BYTE_ORDER);
const cardinal AudioQuality::QualityLevels;


// ###### Constructor #######################################################
AudioQuality::AudioQuality()
{
   setSamplingRate(LowestSamplingRate);
   setBits(LowestBits);
   setChannels(LowestChannels);
   setByteOrder(BYTE_ORDER);
}


// ###### Constructor #######################################################
AudioQuality::AudioQuality(const card16 samplingRate,
                           const card8  bits,
                           const card8  channels,
                           const card16 byteOrder)
{
   setSamplingRate(samplingRate);
   setBits(bits);
   setChannels(channels);
   setByteOrder(byteOrder);
}


// ###### Constructor #######################################################
AudioQuality::AudioQuality(const AudioQualityInterface& quality)
{
   setSamplingRate(quality.getSamplingRate());
   setBits(quality.getBits());
   setChannels(quality.getChannels());
   setByteOrder(quality.getByteOrder());
}


// ###### Get number of channels ############################################
card8 AudioQuality::getChannels() const
{
   return(Channels);
}


// ###### Get number of bits ################################################
card8 AudioQuality::getBits() const
{
   return(Bits);
}


// ###### Get sampling rate #################################################
card16 AudioQuality::getSamplingRate() const
{
   return(SamplingRate);
}


// ###### Get byte order ####################################################
card16 AudioQuality::getByteOrder() const
{
   return(ByteOrder);
}


// ###### Get bytes per second ##############################################
cardinal AudioQuality::getBytesPerSecond() const
{
   return((SamplingRate * Channels * Bits) / 8);
}


// ###### Get bits per sample ###############################################
cardinal AudioQuality::getBitsPerSample() const
{
   return(Channels * Bits);
}


// ###### Set number of bits ################################################
card8 AudioQuality::setBits(const card8 bits) {
   for(cardinal i = 0;i < ValidBits;i++) {
      if(bits <= ValidBitsTable[i]) {
         Bits = ValidBitsTable[i];
         return(Bits);
      }
   }
   std::cerr << "WARNING: AudioQuality::setBits() - Invalid bits " << bits << std::endl;
   Bits = 16;
   return(Bits);
}


// ###### Set number of channels ############################################
card8 AudioQuality::setChannels(const card8 channels) {
   for(cardinal i = 0;i < ValidChannels;i++) {
      if(channels <= ValidChannelsTable[i]) {
         Channels = ValidChannelsTable[i];
         return(Channels);
      }
   }
   std::cerr << "WARNING: AudioQuality::setChannels() - Invalid channels " << channels << std::endl;
   Channels = 16;
   return(Channels);
}


// ###### Set sampling rate #################################################
card16 AudioQuality::setSamplingRate(const card16 rate) {
   for(cardinal i = 0;i < ValidRates;i++) {
      if(rate <= ValidRatesTable[i]) {
         SamplingRate = ValidRatesTable[i];
         return(SamplingRate);
      }
   }
   std::cerr << "WARNING: AudioQuality::setSamplingRate() - Invalid rate " << rate << std::endl;
   SamplingRate = 44100;
   return(SamplingRate);
}


// ###### Set byte order ####################################################
card16 AudioQuality::setByteOrder(const card16 byteOrder) {
   if((byteOrder == LITTLE_ENDIAN) || (byteOrder == BIG_ENDIAN)) {
      ByteOrder = byteOrder;
   }
   else {
      std::cerr << "WARNING: AudioQuality::setByteOrder() - Invalid value "
                << byteOrder << "!" << std::endl;
      ByteOrder = BYTE_ORDER;
   }
   return(ByteOrder);
}


// ###### "="-operator ######################################################
AudioQuality& AudioQuality::operator=(const AudioQualityInterface& quality)

{
   SamplingRate = quality.getSamplingRate();
   Bits         = quality.getBits();
   Channels     = quality.getChannels();
   ByteOrder    = quality.getByteOrder();
   return(*this);
}


// ###### Use next valid sampling rate ######################################
bool AudioQuality::nextSamplingRate()
{
   for(cardinal i = 0;i < ValidRates;i++) {
      if(ValidRatesTable[i] > SamplingRate) {
         SamplingRate = ValidRatesTable[i];
         return(true);
      }
   }
   return(false);
}


// ###### Use previous valid sampling rate ##################################
bool AudioQuality::prevSamplingRate()
{
   for(cardinal i = 0;i < ValidRates;i++) {
      if(ValidRatesTable[i] >= SamplingRate) {
         if(i > 0) {
            SamplingRate = ValidRatesTable[i - 1];
            return(true);
         }
         return(false);
      }
   }
   return(false);
}

// ###### "++"-operator #####################################################
AudioQuality AudioQuality::operator++(int)
{
   if(SamplingRate < 8820)
      nextSamplingRate();
   else if(Bits < 8)
      Bits = 8;
   else if(SamplingRate < 11025)
      nextSamplingRate();
   else if(Channels < 2)
      Channels = 2;
   else if(SamplingRate < 22050)
      nextSamplingRate();
   else if(Bits < 12)
      Bits = 12;
   else if(SamplingRate < 35280)
      nextSamplingRate();
   else if(Bits < 16)
      Bits = 16;
   else
      nextSamplingRate();
   return(*this);
}


// ###### "--"-operator #####################################################
AudioQuality AudioQuality::operator--(int)
{
   if(SamplingRate > 35280)
      prevSamplingRate();
   else if(Bits > 12)
      Bits = 12;
   else if(SamplingRate > 22050)
      prevSamplingRate();
   else if(Bits > 8)
      Bits = 8;
   else if(SamplingRate > 11025)
      prevSamplingRate();
   else if(Channels > 1)
      Channels = 1;
   else if(SamplingRate > 8820)
      prevSamplingRate();
   else if(Bits > 4)
      Bits = 4;
   else
      prevSamplingRate();
   return(*this);
}


// ###### Increase sampling rate ############################################
void AudioQuality::increase(const cardinal steps)
{
   for(cardinal i = 0;i < std::min(steps,QualityLevels);i++) {
      (*this)++;
   }
}


// ###### Decrease sampling rate ############################################
void AudioQuality::decrease(const cardinal steps)
{
   for(cardinal i = 0;i < std::min(steps,QualityLevels);i++) {
      (*this)--;
   }
}


// ###### "<<"-operator #####################################################
std::ostream& operator<<(std::ostream& os, const AudioQualityInterface& quality)
{
   char        string[64];
   const char* byteOrder = "";
   if(quality.getBits() == 16) {
      switch(quality.getByteOrder()) {
         case LITTLE_ENDIAN:
            byteOrder = "l";
          break;
         case BIG_ENDIAN:
            byteOrder = "b";
           break;
         default:
            byteOrder = "?";
          break;
      }
   }
   snprintf((char*)&string,sizeof(string),"%d Hz / %d%s / %s",
            quality.getSamplingRate(),
            quality.getBits(),
            byteOrder,
            (quality.getChannels() == 1) ? "Mono" : "Stereo");
   os << string;
   return(os);
}


// ###### "+"-operator ######################################################
AudioQuality operator+(const AudioQualityInterface& q1, const AudioQualityInterface& q2)
{
   AudioQuality quality;
   quality.setSamplingRate(std::max(q1.getSamplingRate(),q2.getSamplingRate()));
   quality.setChannels(std::max(q1.getChannels(),q2.getChannels()));
   quality.setBits(std::max(q1.getBits(),q2.getBits()));
   return(quality);
}


// ###### "-"-operator ######################################################
AudioQuality operator-(const AudioQualityInterface& q1, const AudioQualityInterface& q2)
{
   AudioQuality quality;
   quality.setSamplingRate(std::min(q1.getSamplingRate(),q2.getSamplingRate()));
   quality.setChannels(std::min(q1.getChannels(),q2.getChannels()));
   quality.setBits(std::min(q1.getBits(),q2.getBits()));
   return(quality);
}


// ###### "-"-operator ######################################################
AudioQuality operator-(const AudioQualityInterface& q1, const cardinal bytesPerSecond)
{
   AudioQuality quality = q1;
   while(quality.getBytesPerSecond() > bytesPerSecond) {
      if(quality == AudioQuality::LowestQuality) {
         return(AudioQuality(0,0,0));
      }
      quality--;
   }
   return(quality);
}


// ###### Get maximum quality for given byte rate ###########################
AudioQuality AudioQuality::getQualityForByteRate(const cardinal bps)
{
   AudioQuality quality = HighestQuality;
   while(quality.getBytesPerSecond() > bps) {
      if(quality == AudioQuality::LowestQuality) {
         return(AudioQuality(0,0,0));
      }
      quality--;
   }
   return(quality);
}


// ###### Get random quality setting ########################################
AudioQuality AudioQuality::getRandomQuality(Randomizer* randomizer)
{
   const cardinal rateIndex     = randomizer->random(0,ValidRates - 1);
   const cardinal bitsIndex     = randomizer->random(0,ValidBits - 1);
   const cardinal channelsIndex = randomizer->random(0,ValidChannels - 1);
   return(AudioQuality(ValidRatesTable[rateIndex],
                       ValidBitsTable[bitsIndex],
                       ValidChannelsTable[channelsIndex]));
}
