// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Spectrum Analyzer Implementation                                 ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "spectrumanalyzer.h"
#include "fft.h"
#include "tools.h"
#include "audioquality.h"
#include "audioconverter.h"


#include <iostream>
#include <sys/time.h>


// ###### Constructor #######################################################
SpectrumAnalyzer::SpectrumAnalyzer()
   : Synchronizable("SpectrumAnalyzer")
{
   AudioSamplingRate = 0;
   AudioChannels     = 0;
   AudioBits         = 0;
   AudioByteOrder    = BYTE_ORDER;
   InputBufferPos    = 0;
   FFT               = new FastFourierTransformation(FFTPoints);
   setQuality(AudioQuality::HighestQuality);
}


// ###### Destructor ########################################################
SpectrumAnalyzer::~SpectrumAnalyzer()
{
   if(FFT != NULL) {
      delete FFT;
      FFT = NULL;
   }
}


// ###### Get number of channels ############################################
card8 SpectrumAnalyzer::getChannels() const
{
   return(AudioChannels);
}


// ###### Get number of bits ################################################
card8 SpectrumAnalyzer::getBits() const
{
   return(AudioBits);
}


// ###### Get sampling rate #################################################
card16 SpectrumAnalyzer::getSamplingRate() const
{
   return(AudioSamplingRate);
}


// ###### Get byte order ####################################################
card16 SpectrumAnalyzer::getByteOrder() const
{
   return(AudioByteOrder);
}


// ###### Set number of bits ################################################
card8 SpectrumAnalyzer::setBits(const card8 bits) {
   synchronized();
   if(AudioBits != bits) {
      AudioBits      = bits;
      InputBufferPos = 0;
   }
   unsynchronized();
   return(AudioBits);
}


// ###### Set number of channels ############################################
card8 SpectrumAnalyzer::setChannels(const card8 channels) {
   synchronized();
   if(AudioChannels != channels) {
      AudioChannels  = channels;
      InputBufferPos = 0;
   }
   unsynchronized();
   return(AudioChannels);
}


// ###### Set sampling rate #################################################
card16 SpectrumAnalyzer::setSamplingRate(const card16 rate) {
   synchronized();
   if(AudioSamplingRate != rate) {
      AudioSamplingRate = rate;
      InputBufferPos    = 0;
   }
   unsynchronized();
   return(AudioSamplingRate);
}


// ###### Set byte order ####################################################
card16 SpectrumAnalyzer::setByteOrder(const card16 byteOrder) {
   synchronized();
   if(AudioByteOrder != byteOrder) {
      AudioByteOrder = byteOrder;
      InputBufferPos = 0;
   }
   unsynchronized();
   return(AudioByteOrder);
}


// ###### Get bytes per second ##############################################
cardinal SpectrumAnalyzer::getBytesPerSecond() const
{
   return((AudioSamplingRate * AudioChannels * AudioBits) / 8);
}


// ###### Get bits per sample ###############################################
cardinal SpectrumAnalyzer::getBitsPerSample() const
{
   return(AudioChannels * AudioBits);
}


// ###### Check, if spectrum analyzer is ready ##############################
bool SpectrumAnalyzer::ready() const
{
   return(FFT != NULL);
}


// ###### Reset #############################################################
void SpectrumAnalyzer::sync()
{
   synchronized();
   InputBufferPos = 0;
   unsynchronized();
}


// ###### Write data to spectrum analyzer ###################################
bool SpectrumAnalyzer::write(const void* dataPtr, const size_t totalLength)
{
   synchronized();
   if(InputBufferPos < (FFTPoints * getBitsPerSample()) / 8) {
      // ====== Copy data into input buffer =================================
      cardinal    length = totalLength;
      const void* data   = dataPtr;
      while(length > 0) {
         cardinal size = std::min(length,((FFTPoints * getBitsPerSample()) / 8) - InputBufferPos);
         memcpy((void*)&InputBuffer[InputBufferPos],data,size);
         InputBufferPos += size;

         length -= size;
         data = (void*)((long)data + (long)size);

         // ====== Buffer full -> ready to do FFT ===========================
         if(InputBufferPos >= (FFTPoints * getBitsPerSample()) / 8) {
            break;
         }
      }
   }
   unsynchronized();

   return(true);
}


// ###### Do Fourier transformation on input data ###########################
void SpectrumAnalyzer::doFourierTransformation(card16*        inputData,
                                               cardinal*      output,
                                               const cardinal bars)
{
   // ====== Do Fourier transformation ======================================
   int16* data = (int16*)inputData;
   FFT->fft((int16*)data);
   integer* BitReversed = FFT->getBitReversed();
   const cardinal points = 80;

   // ====== Get real FFT values from complex ones ==========================
   integer  fftArray[points];
   cardinal i;
   cardinal j;
   for (i = 0;i < points;i++) {
      integer re  = (integer)data[BitReversed[i]];
      integer im  = (integer)data[BitReversed[i]+1];
      double  tmp = (double)(re*re + im*im);
      fftArray[i]=(integer)(sqrt(sqrt(tmp)));
   }

   // ====== Calculate bar values ===========================================
   const float valuesPerBar = (float)points / (float)bars;
   for(i = 0;i < bars;i++) {
      const cardinal pos = (cardinal)((float)i * (float)valuesPerBar);

      cardinal sum = fftArray[pos];
      for(j = 1;j < valuesPerBar;j++)
         sum += fftArray[pos + j];
      output[i] = sum / j;
   }
}


// ###### Get spectrum ######################################################
bool SpectrumAnalyzer::getSpectrum(cardinal* left, cardinal* right, const cardinal bars)
{
   synchronized();
   // ====== Check, if buffer is not filled =================================
   if(InputBufferPos < (FFTPoints * getBitsPerSample()) / 8) {
      unsynchronized();
      return(false);
   }

   const    cardinal maximumBytesPerSecond = AudioQuality::HighestQuality.getBytesPerSecond();
   const    card64   required = ((card64)InputBufferPos * (card64)maximumBytesPerSecond) / (card64)getBytesPerSecond();
   card8*   buffer[(size_t)required];
   card16*  buffer16 = (card16*)&buffer;
   card16   data[FFTPoints];
   AudioConverter(AudioQuality(AudioSamplingRate,AudioBits,AudioChannels,AudioByteOrder),
                  AudioQuality(AudioSamplingRate,16,2,BYTE_ORDER),
                  (card8*)&InputBuffer,(card8*)&buffer,InputBufferPos,required);

   for(cardinal i = 0;i < FFTPoints;i++) {
      data[i] = buffer16[2 * i];
   }
   doFourierTransformation((card16*)&data,left,bars);

   if(AudioChannels > 1) {
      for(cardinal i = 0;i < FFTPoints;i++) {
         data[i] = buffer16[(2 * i) + 1];
      }
      doFourierTransformation((card16*)&data,right,bars);
   }
   else {
      for(cardinal i = 0;i < bars;i++)
         right[i] = left[i];
   }

   InputBufferPos = 0;
   unsynchronized();
   return(true);
}
