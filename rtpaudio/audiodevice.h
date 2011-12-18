// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Device                                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
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


#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H


#include "tdsystem.h"
#include "audiowriterinterface.h"
#include "thread.h"
#include "ringbuffer.h"

#ifdef HAVE_PULSEAUDIO
#include <pulse/simple.h>
#endif


/**
  * This class implements AudioWriterInterface for the audio device.
  *
  * @short   Audio Device
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioDevice : virtual public AudioWriterInterface,
                    public Thread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param name Name of the audio device (normally "/dev/dsp").
     */
   AudioDevice(const char* name = "/dev/dsp");

   /**
     * Destructor.
     */
   ~AudioDevice();


   // ====== Status functions ===============================================
   /**
     * Get number of times, sync() has been called.
     *
     * @return Number of times, sync() has been called.
     *
     * @see AudioWriterInterface#sync
     */
   inline cardinal getSyncCount() const;

   /**
     * Reset number of times, sync() has been called.
     *
     * @see AudioWriterInterface#sync
     */
   inline void resetSyncCount();


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

   /**
     * getCurrentCapacity() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#write
     */
   cardinal getCurrentCapacity();


   // ====== Constants ======================================================
   /**
     * Size of audio ringbuffer in bytes.
     */
   static const cardinal RingBufferSize = 128 * 1024;

   /**
     * Buffer fill threshold to Resize (in percent).
     */
   static const cardinal ResizeThresholdPercent = 75;

   /**
     * Buffer resize modulo: The buffer's size will be removed
     * by the fraction of (1/ResizeModulo) by removing every
     * ResizeModulo-th 32-bit word.
     */
   static const cardinal ResizeModulo = 4;


   // ====== Internal data ==================================================
   private:
   void run();


   bool       IsReady;

#ifdef HAVE_PULSEAUDIO
   friend class AudioMixer;
   pa_simple* Device;
#else
   int        DeviceFD;           // Device and its properties.
   int        DeviceCapabilities;
   int        DeviceFormats;
#endif
   int        DeviceBlockSize;
   integer    DeviceFragmentSize;
   integer    DeviceOSpace;

   cardinal   SyncCount;

   card16     DeviceSamplingRate; // Format of data written to real device.
   card8      DeviceBits;
   card8      DeviceChannels;
   card16     DeviceByteOrder;

   card16     AudioSamplingRate;  // Format of data written to AudioDevice.
   card8      AudioBits;
   card8      AudioChannels;
   card16     AudioByteOrder;

   RingBuffer Buffer;
   bool       IsFillingBuffer;
   cardinal   ResizeThreshold;

   cardinal   JitterCompensationLatency;
   card64     LastWriteTimeStamp;
   integer    Balance;
};


#include "audiodevice.icc"


#endif
