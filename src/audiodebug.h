// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Debug                                                      ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2019 by Thomas Dreibholz            ####
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


#ifndef AUDIODEBUG_H
#define AUDIODEBUG_H


#include "tdsystem.h"
#include "audiowriterinterface.h"


/**
  * This class implements AudioWriterInterface for the audio debugger.
  *
  * @short   Audio Debug
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioDebug : virtual public AudioWriterInterface
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   AudioDebug();

   /**
     * Destructor.
     */
   ~AudioDebug();


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


   // ====== Internal data ==================================================
   private:
   card64   LastWriteTimeStamp;
   card64   LastPrintTimeStamp;
   cardinal BytesWritten;
   integer  Balance;

   card16   AudioSamplingRate;
   card8    AudioChannels;
   card8    AudioBits;
   card16   AudioByteOrder;
};


#endif
