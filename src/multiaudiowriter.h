// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Multi Audio Writer                                               ####
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


#ifndef MULTIAUDIOWRITER_H
#define MULTIAUDIOWRITER_H


#include "tdsystem.h"
#include "audiowriterinterface.h"
#include "synchronizable.h"

#include <set>


/**
  * This class implements AudioWriterInterface for a set of AudioWriterInterfaces.
  * Example: AudioDevice + AudioDebug + SpectrumAnalyzer.
  *
  * @short   Multi Audio Writer
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class MultiAudioWriter : virtual public AudioWriterInterface,
                         public Synchronizable
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   MultiAudioWriter();

   /**
     * Destructor.
     */
   ~MultiAudioWriter();


   // ====== MultiAudioWriter functions =====================================
   /**
     * Add new AudioWriterInferface to writer set.
     *
     * @param writer AudioWriterInterface object.
     * @return true, if writer has been added; false otherwise.
     */
   bool addWriter(AudioWriterInterface* writer);

   /**
     * Remove AudioWriterInterface object from writer set.
     */
   void removeWriter(AudioWriterInterface* writer);


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
     * setSamplingRate() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#setSamplingRate
     */
   card16 setSamplingRate(const card16 samplingRate);

   /**
     * setBits() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#setBits
     */
   card8 setBits(const card8 bits);

   /**
     * setChannels() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#setChannels
     */
   card8 setChannels(const card8 channels);


   /**
     * setByteOrder() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#setByteOrder
     */
   card16 setByteOrder(const card16 byteOrder);


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


   // ====== Internal data ==================================================
   private:
   std::multiset<AudioWriterInterface*> WriterSet;

   card16 AudioSamplingRate;
   card8  AudioBits;
   card8  AudioChannels;
   card16 AudioByteOrder;
};


#endif
