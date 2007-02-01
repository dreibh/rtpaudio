// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Device                                                     ####
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
#include "audiodevice.h"
#include "audioconverter.h"


#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>


// Debug mode: Print debug information.
#define DEBUG


// ###### Audio device constructor ##########################################
AudioDevice::AudioDevice(const char* name)
   : Thread("AudioDeviceThread")
{
   // ====== Open Audio Device ==============================================
   IsReady            = false;

   AudioChannels      = 0;
   AudioBits          = 0;
   AudioSamplingRate  = 0;
   AudioByteOrder     = BYTE_ORDER;

   DeviceFD = open(name,O_WRONLY);
   if(DeviceFD < 0) {
      cerr << "************************************************************" << endl;
      cerr << "WARNING: AudioDevice::AudioDevice() - Unable to open device!" << endl;
      cerr << "************************************************************" << endl;
      return;
   }

   DeviceChannels     = 0;
   DeviceSamplingRate = 0;

   // ====== Initialize audio device ========================================
   if(ioctl(DeviceFD,SNDCTL_DSP_GETCAPS,&DeviceCapabilities) < 0) {
      cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_GETCAPS failed!" << endl;
      return;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_GETFMTS,&DeviceFormats) < 0) {
      cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_GETFMTS failed!" << endl;
      return;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_GETBLKSIZE,&DeviceBlockSize) < 0) {
      cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_GETBLKSIZE failed!" << endl;
      return;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_NONBLOCK,0) < 0) {
      cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_NONBLOCK failed!" << endl;
      return;
   }


   // ====== Select audio format ============================================
   int format = 0;
   if((DeviceFormats & AFMT_S16_BE) && (BYTE_ORDER == BIG_ENDIAN)) {
      DeviceBits      = 16;
      DeviceByteOrder = BIG_ENDIAN;
      format          = AFMT_S16_BE;
   }
   else if((DeviceFormats & AFMT_S16_LE) && (BYTE_ORDER == LITTLE_ENDIAN)) {
      DeviceBits      = 16;
      DeviceByteOrder = LITTLE_ENDIAN;
      format          = AFMT_S16_LE;
   }
   else if(DeviceFormats & AFMT_S16_BE) {
      DeviceBits      = 16;
      DeviceByteOrder = BIG_ENDIAN;
      format          = AFMT_S16_BE;
   }
   else if(DeviceFormats & AFMT_S16_LE) {
      DeviceBits      = 16;
      DeviceByteOrder = LITTLE_ENDIAN;
      format          = AFMT_S16_LE;
   }
   else if(DeviceFormats & AFMT_U8) {
      cerr << "NOTE: AudioDevice::AudioDevice() - "
              "Your audio device seems not to support 16 bits!" << endl;
      DeviceBits      = 8;
      DeviceByteOrder = BYTE_ORDER;
      format = AFMT_U8;
   }
   else {
      cerr << "ERROR: AudioDevice::AudioDevice() - "
           << "Your audio device does not support S16 or U8 format?!?!" << endl;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_SETFMT,&format) < 0) {
      cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_SETFMT failed!" << endl;
      return;
   }


   // ====== Get device buffer information ==================================
   audio_buf_info abinfo;
   if(ioctl(DeviceFD,SNDCTL_DSP_GETOSPACE,&abinfo) < 0) {
      cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_GETOSPACE failed!" << endl;
      return;
   }
   DeviceOSpace       = abinfo.bytes;
   DeviceFragmentSize = abinfo.fragsize;

   ResizeThreshold    = (RingBufferSize * ResizeThresholdPercent) / 100;


   // ====== Initialize quality =============================================
   setSamplingRate(AudioQuality::HighestSamplingRate);
   setQuality(AudioQuality::HighestQuality);


   // ====== Print results ==================================================
#ifdef DEBUG
   cout << "AudioDevice:" << endl;
   cout << "   DeviceSamplingRate = " << DeviceSamplingRate << endl;
   cout << "   DeviceBits         = " << (cardinal)DeviceBits << endl;
   cout << "   DeviceChannels     = " << (cardinal)DeviceChannels << endl;
   cout << "   DeviceByteOrder    = " << ((DeviceByteOrder == LITTLE_ENDIAN) ? "Little Endian" : "Big Endian") << endl;
   cout << "   DeviceFragmentSize = " << DeviceFragmentSize << endl;
   cout << "   DeviceOSpace       = " << DeviceOSpace
        << " = " << AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels).bytesToTime(DeviceOSpace) << " [s]" << endl;
   cout << "   Capabilities       = ";
   if(DeviceCapabilities & DSP_CAP_REALTIME) cout << "<Real-time> ";
   if(DeviceCapabilities & DSP_CAP_BATCH)    cout << "<Batch> ";
   if(DeviceCapabilities & DSP_CAP_DUPLEX)   cout << "<Duplex> ";
   if(DeviceCapabilities & DSP_CAP_TRIGGER)  cout << "<Trigger> ";
   if(DeviceCapabilities & DSP_CAP_MMAP)     cout << "<MMap> ";
   cout << endl;
   cout << "RingBuffer:" << endl;
   cout << "   ResizeThreshold    = " << ResizeThreshold
        << " = " << AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels).bytesToTime(ResizeThreshold) << " [s]" << endl;
#endif


   // ====== Start thread ===================================================
   SyncCount = 0;
   IsReady = Buffer.init(RingBufferSize);
   if(IsReady == false) {
      cerr << "ERROR: AudioDevice::AudioDevice() - Ring buffer initialization failed!" << endl;
   }
   IsReady = start();
   if(IsReady == false) {
      cerr << "ERROR: AudioDevice::AudioDevice() - Copy thread startup failed!" << endl;
   }
   Thread::delay(1500000);
}


// ###### Audio device destructor ###########################################
AudioDevice::~AudioDevice()
{
   IsReady = false;
   Buffer.flush();
   stop();
   if(DeviceFD >= 0) {
      close(DeviceFD);
      DeviceFD = -1;
   }
}


// ###### Get number of channels ############################################
card8 AudioDevice::getChannels() const
{
   return(AudioChannels);
}


// ###### Get number of bits ################################################
card8 AudioDevice::getBits() const
{
   return(AudioBits);
}


// ###### Get sampling rate #################################################
card16 AudioDevice::getSamplingRate() const
{
   return(AudioSamplingRate);
}


// ###### Get byte order ####################################################
card16 AudioDevice::getByteOrder() const
{
   return(AudioByteOrder);
}


// ###### Set number of bits ################################################
card8 AudioDevice::setBits(const card8 bits) {
   AudioBits = bits;
   return(AudioBits);
}


// ###### Set number of channels ############################################
card8 AudioDevice::setChannels(const card8 channels) {
   if(DeviceFD < 0) {
      return(AudioChannels);
   }

   if(channels != AudioChannels) {
      AudioChannels = channels;
      int arg = channels;
      sync();
      IsReady = (ioctl(DeviceFD,SNDCTL_DSP_CHANNELS,&arg) >= 0);
      if(!IsReady) {
         cerr << "WARNING: AudioDevice::setChannels() - IOCTL error <"
              << sys_errlist[errno] << ">" << endl;
      }
      else {
         DeviceChannels = arg;
      }
   }
   return(AudioChannels);
}


// ###### Set sampling rate #################################################
card16 AudioDevice::setSamplingRate(const card16 rate) {
   if(DeviceFD < 0) {
      return(AudioSamplingRate);
   }

   if(rate != AudioSamplingRate) {
      AudioSamplingRate = rate;
      int arg = rate;
      sync();
      IsReady = (ioctl(DeviceFD,SNDCTL_DSP_SPEED,&arg) >= 0);
      if(!IsReady) {
         cerr << "WARNING: AudioDevice::plingRate() - IOCTL error <"
              << sys_errlist[errno] << ">" << endl;
      }
      else {
         DeviceSamplingRate = arg;

         // ES1371 soundcard reports rate - 1 =>
         // Set DeviceSamplingRate to wanted sampling rate, if difference < 4.
         if(abs((integer)DeviceSamplingRate - (integer)rate) <= 4) {
            DeviceSamplingRate = rate;
         }
      }
   }
   return(AudioSamplingRate);
}


// ###### Set byte order ####################################################
card16 AudioDevice::setByteOrder(const card16 byteOrder) {
   AudioByteOrder = byteOrder;
   return(AudioByteOrder);
}


// ###### Get bytes per second ##############################################
cardinal AudioDevice::getBytesPerSecond() const
{
   return((AudioSamplingRate * AudioChannels * AudioBits) / 8);
}


// ###### Get bits per sample ###############################################
cardinal AudioDevice::getBitsPerSample() const
{
   return(AudioChannels * AudioBits);
}


// ###### Force buffers to be written to device #############################
void AudioDevice::sync()
{
   if(DeviceFD < 0)
      return;

   // ====== Do SNDCTL_DSP_RESET ============================================
   // SNDCTL_DSP_SYNC has been replaced by SNDCTL_DSP_RESET, because it seems
   // that SNDCTL_DSP_SYNC has a longer delay.
   Buffer.flush();
   IsReady = (ioctl(DeviceFD,SNDCTL_DSP_RESET,0) >= 0);
   if(!IsReady) {
      cerr << "WARNING: AudioDevice::sync() - IOCTL error <"
           << sys_errlist[errno] << ">" << endl;
      return;
   }
   SyncCount++;
}


// ###### Check, if device is ready #########################################
bool AudioDevice::ready() const
{
   return(IsReady);
}


// ###### Write data to device ##############################################
bool AudioDevice::write(const void* data, const size_t length)
{
   if(DeviceFD < 0) {
      return(false);
   }

   // ====== Always convert to 16 bit for better output quality =============
   // (Even if quality is 8 bits, most soundcards will produce better output,
   //    if data is in 16 bit format!)
   const    cardinal maximumBytesPerSecond = AudioQuality::HighestQuality.getBytesPerSecond();
   const    card64   required = ((card64)length * (card64)maximumBytesPerSecond)
                                   / (card64)getBytesPerSecond();
   card8*   buffer[(size_t)required];
   cardinal len = AudioConverter(
                     *this,
                     AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels,DeviceByteOrder),
                     (card8*)data,(card8*)&buffer,length,required);


   Buffer.synchronized();

   // Force moving data from ring buffer to audio device here.
   // There may be not enough time to wait for the thread to do the
   // job. Some soundcards have awfully small audio buffers...
   moveAudioData();


   // ====== Write data to buffer ===========================================
   const bool ok = (Buffer.write((char*)buffer,len) == (ssize_t)len);

   const size_t bytes = Buffer.bytesReadable();
   if(bytes >= ResizeThreshold) {
#ifdef DEBUG
      printTimeStamp(cout);
      cout << "Content resize: " << bytes << " -> ";
#endif
      char resizeBuffer[bytes];
      if(Buffer.read((char*)&resizeBuffer,bytes) == (ssize_t)bytes) {
         size_t out = 0;
         for(size_t i = 0;i < bytes;i += 4) {
            if((i % (4 * ResizeModulo)) != 0) {
               resizeBuffer[out++] = resizeBuffer[i + 0];
               resizeBuffer[out++] = resizeBuffer[i + 1];
               resizeBuffer[out++] = resizeBuffer[i + 2];
               resizeBuffer[out++] = resizeBuffer[i + 3];
            }
         }
#ifdef DEBUG
         cout << out << endl;
#endif
         if(Buffer.write((char*)&resizeBuffer,out) != (ssize_t)out) {
            cerr << "ERROR: RingBuffer write error!" << endl;
         }
      }
   }

   Buffer.unsynchronized();

   return(ok);
}


// ###### Constructor #######################################################
cardinal AudioDevice::getCurrentCapacity()
{
   return(Buffer.bytesWritable());
}


// ###### Copy data from ring buffer to device ##############################
void AudioDevice::moveAudioData()
{
   if(IsReady == true) {
      char buffer[512];
      ssize_t length;
      ssize_t result;
      do {
         length = Buffer.read((char*)&buffer,sizeof(buffer));
         if(length > 0) {
            result = ::write(DeviceFD,(char*)&buffer,length);
         }
         else {
            result = -1;
         }
      } while(result == length);
   }
}


// ###### Audio data copy thread ############################################
void AudioDevice::run()
{
   for(;;) {
      Buffer.wait();
      moveAudioData();
   }
}
