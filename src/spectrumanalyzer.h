// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Spectrum Analyzer                                                ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2021 by Thomas Dreibholz            ####
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


#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H


#include "tdsystem.h"
#include "audiowriterinterface.h"
#include "synchronizable.h"
#include "fft.h"


/**
  * This class implements a spectrum analyzer device implementing AudioWriterInterface.
  *
  * @short   Spectrum Analyzer
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class SpectrumAnalyzer : virtual public AudioWriterInterface,
                         public Synchronizable
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   SpectrumAnalyzer();

   /**
     * Destructor.
     */
   ~SpectrumAnalyzer();


   // ====== AudioQualityInterface implementation ===========================
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


   // ====== AudioInterface implementation ==================================
   /**
     * ready() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#ready
     */
   bool ready() const;

   /**
     * sync() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#sync
     */
   void sync();

   /**
     * write() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#write
     */
   bool write(const void* data, const size_t length);


   // ====== Spectrum analyzer functions ====================================
   /**
     * Do Fourier transformation and get spectrum.
     *
     * @param left Pointer to spectrum array for left channel.
     * @param right Pointer to spectrum array for right channel.
     * @param bars Number of bars.
     * @return true, if spectrum has been computed; false, if there is not enough input data available.
     */
   bool getSpectrum(cardinal* left, cardinal* right, const cardinal bars);


   // ====== Internal data ==================================================
   private:
   void doFourierTransformation(card16* data, cardinal* output, cardinal bars);


   static const cardinal      FFTPoints = 256;
   FastFourierTransformation* FFT;
   cardinal                   InputBufferPos;
   char                       InputBuffer[4 * FFTPoints];


   card16                     AudioSamplingRate;
   card8                      AudioBits;
   card8                      AudioChannels;
   card16                     AudioByteOrder;
};


#endif
