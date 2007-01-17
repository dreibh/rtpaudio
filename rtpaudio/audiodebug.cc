// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Debug                                                      ####
// ####                                                                  ####
// #### Version 1.50  --  August 01, 2001                                ####
// ####                                                                  ####
// ####            Copyright (C) 1999-2001 by Thomas Dreibholz           ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@exp-math.uni-essen.de                           ####
// ####    WWW:   http://www.exp-math.uni-essen.de/~dreibh/rtpaudio      ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software; you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License      ####
// #### as published by the Free Software Foundation; either version 2   ####
// #### of the License, or (at your option) any later version.           ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// ##########################################################################



#include "tdsystem.h"
#include "audiodebug.h"
#include "tools.h"
#include "audioquality.h"


#include <sys/time.h>


namespace Coral {


// ###### Audio debug constructor ###########################################
AudioDebug::AudioDebug() {
   setQuality(AudioQuality::HighestQuality);
   LastWriteTimeStamp = 0;
   LastPrintTimeStamp = 0;
   BytesWritten       = 0;
   Balance            = 0;
}


// ###### Audio debug destructor ############################################
AudioDebug::~AudioDebug()
{
}


// ###### Get number of channels ############################################
card8 AudioDebug::getChannels() const
{
   return(AudioChannels);
}


// ###### Get number of bits ################################################
card8 AudioDebug::getBits() const
{
   return(AudioBits);
}


// ###### Get sampling rate #################################################
card16 AudioDebug::getSamplingRate() const
{
   return(AudioSamplingRate);
}


// ###### Get byte order ####################################################
card16 AudioDebug::getByteOrder() const
{
   return(AudioByteOrder);
}


// ###### Set number of bits ################################################
card8 AudioDebug::setBits(const card8 bits) {
   if(AudioBits != bits) {
      AudioBits = bits;
      sync();
   }
   return(AudioBits);
}


// ###### Set number of channels ############################################
card8 AudioDebug::setChannels(const card8 channels) {
   if(AudioChannels != channels) {
      AudioChannels = channels;
      sync();
   }
   return(AudioChannels);
}


// ###### Set sampling rate #################################################
card16 AudioDebug::setSamplingRate(const card16 rate) {
   if(AudioSamplingRate != rate) {
      AudioSamplingRate = rate;
      sync();
   }
   return(AudioSamplingRate);
}


// ###### Set byte order ####################################################
card16 AudioDebug::setByteOrder(const card16 byteOrder) {
   AudioByteOrder = byteOrder;
   return(AudioByteOrder);
}


// ###### Get bytes per second ##############################################
cardinal AudioDebug::getBytesPerSecond() const
{
   return((AudioSamplingRate * AudioChannels * AudioBits) / 8);
}


// ###### Get bits per sample ###############################################
cardinal AudioDebug::getBitsPerSample() const
{
   return(AudioChannels * AudioBits);
}

// ###### Check, if device is ready #########################################
bool AudioDebug::ready() const
{
   return(true);
}


// ###### Force buffers to be written to device #############################
void AudioDebug::sync()
{
   LastWriteTimeStamp = 0;
   LastPrintTimeStamp = 0;
   Balance            = 0;
   BytesWritten       = 0;
}


// ###### Write data to device ##############################################
bool AudioDebug::write(const void* data, const size_t length)
{
   const card64 now = getMicroTime();
   const double bytesPerMicroSecond = (double)
      (((cardinal)AudioSamplingRate * (cardinal)AudioChannels * (cardinal)AudioBits) / 8) /
      1000000.0;
   
   if(LastWriteTimeStamp != 0) {
      const card64 delay = now - LastWriteTimeStamp;
      Balance -= (integer)((double)delay * bytesPerMicroSecond);
      
      if((Balance > - 100000) && (Balance < 100000)) {
         if(now - LastPrintTimeStamp > 250000) {
            cout << "out=" << BytesWritten << "  ";
            cout << "balance=" << Balance << "  ";
            cout << "rate=" << ((card64)BytesWritten * 1000000) / (now - LastPrintTimeStamp) << " [bps]";
            LastPrintTimeStamp = now;
            if(Balance < 0) {
               cout << "  => reset";
               Balance = 0;
            }
            cout << endl;
            BytesWritten = 0;
         }
      }
      else {
         cout << "AudioDebug::sync()" << endl;
         sync();
      }
   }
   LastWriteTimeStamp = now;
   Balance          += (integer)length;
   BytesWritten     += length;

   return(true);
}


}
