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
#include "tools.h"
#include "audiodevice.h"
#include "audioconverter.h"


#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>


// Debug mode: Print debug information.
// #define DEBUG


// ###### Audio device constructor ##########################################
AudioDevice::AudioDevice(const char* name)
   : Thread("AudioDeviceThread")
{
   // ====== Open Audio Device ==============================================
   IsReady                   = false;
   IsFillingBuffer           = true;

   AudioChannels             = 0;
   AudioBits                 = 0;
   AudioSamplingRate         = 0;
   AudioByteOrder            = BYTE_ORDER;

   LastWriteTimeStamp        = 0;
   Balance                   = 0;
   JitterCompensationLatency = 100000;   // 100ms

   DeviceFD = open(name,O_WRONLY);
   if(DeviceFD < 0) {
      std::cerr << "************************************************************" << std::endl;
      std::cerr << "WARNING: AudioDevice::AudioDevice() - Unable to open device!" << std::endl;
      std::cerr << "************************************************************" << std::endl;
      return;
   }

   DeviceChannels     = 0;
   DeviceSamplingRate = 0;

   // ====== Initialize audio device ========================================
   if(ioctl(DeviceFD,SNDCTL_DSP_GETCAPS,&DeviceCapabilities) < 0) {
      std::cerr << "WARNING: AudioDevice::AudioDevice() - "
                   "ioctl SNDCTL_DSP_GETCAPS failed!" << std::endl;
      return;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_GETFMTS,&DeviceFormats) < 0) {
      std::cerr << "WARNING: AudioDevice::AudioDevice() - "
                   "ioctl SNDCTL_DSP_GETFMTS failed!" << std::endl;
      return;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_GETBLKSIZE,&DeviceBlockSize) < 0) {
      std::cerr << "WARNING: AudioDevice::AudioDevice() - "
                   "ioctl SNDCTL_DSP_GETBLKSIZE failed!" << std::endl;
      return;
   }
#if 0
   if(ioctl(DeviceFD,SNDCTL_DSP_NONBLOCK,0) < 0) {
      std::cerr << "WARNING: AudioDevice::AudioDevice() - "
                   "ioctl SNDCTL_DSP_NONBLOCK failed!" << std::endl;
   }
#endif


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
      std::cerr << "NOTE: AudioDevice::AudioDevice() - "
                   "Your audio device seems not to support 16 bits!" << std::endl;
      DeviceBits      = 8;
      DeviceByteOrder = BYTE_ORDER;
      format = AFMT_U8;
   }
   else {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - "
                   "Your audio device does not support S16 or U8 format?!?!" << std::endl;
   }
   if(ioctl(DeviceFD,SNDCTL_DSP_SETFMT,&format) < 0) {
      std::cerr << "WARNING: AudioDevice::AudioDevice() - "
                   "ioctl SNDCTL_DSP_SETFMT failed!" << std::endl;
      return;
   }


   // ====== Get device buffer information ==================================
   audio_buf_info abinfo;
   if(ioctl(DeviceFD,SNDCTL_DSP_GETOSPACE,&abinfo) < 0) {
      std::cerr << "WARNING: AudioDevice::AudioDevice() - "
              "ioctl SNDCTL_DSP_GETOSPACE failed!" << std::endl;
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
   std::cout << "AudioDevice:" << std::endl;
   std::cout << "   DeviceSamplingRate = " << DeviceSamplingRate << std::endl;
   std::cout << "   DeviceBits         = " << (cardinal)DeviceBits << std::endl;
   std::cout << "   DeviceChannels     = " << (cardinal)DeviceChannels << std::endl;
   std::cout << "   DeviceByteOrder    = " << ((DeviceByteOrder == LITTLE_ENDIAN) ? "Little Endian" : "Big Endian") << std::endl;
   std::cout << "   DeviceFragmentSize = " << DeviceFragmentSize << std::endl;
   std::cout << "   DeviceOSpace       = " << DeviceOSpace
             << " = " << AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels).bytesToTime(DeviceOSpace) << " [s]" << std::endl;
   std::cout << "   Capabilities       = ";
   if(DeviceCapabilities & DSP_CAP_REALTIME) std::cout << "<Real-time> ";
   if(DeviceCapabilities & DSP_CAP_BATCH)    std::cout << "<Batch> ";
   if(DeviceCapabilities & DSP_CAP_DUPLEX)   std::cout << "<Duplex> ";
   if(DeviceCapabilities & DSP_CAP_TRIGGER)  std::cout << "<Trigger> ";
   if(DeviceCapabilities & DSP_CAP_MMAP)     std::cout << "<MMap> ";
   std::cout << std::endl;
   std::cout << "RingBuffer:" << std::endl;
   std::cout << "   ResizeThreshold    = " << ResizeThreshold
             << " = " << AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels).bytesToTime(ResizeThreshold) << " [s]" << std::endl;
#endif


   // ====== Start thread ===================================================
   SyncCount = 0;
   IsReady = Buffer.init(RingBufferSize);
   if(IsReady == false) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - Ring buffer initialization failed!" << std::endl;
   }
   IsReady = start();
   if(IsReady == false) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - Copy thread startup failed!" << std::endl;
   }
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
         std::cerr << "WARNING: AudioDevice::setChannels() - IOCTL error <"
              << strerror(errno) << ">" << std::endl;
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
         std::cerr << "WARNING: AudioDevice::setSamplingRate() - IOCTL error <"
                   << strerror(errno) << ">" << std::endl;
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
#ifdef DEBUG
   std::cout << "sync! buffered=" << Buffer.bytesReadable() << std::endl;
#endif
   Buffer.flush();
   IsReady = (ioctl(DeviceFD,SNDCTL_DSP_RESET,0) >= 0);
   if(!IsReady) {
      std::cerr << "WARNING: AudioDevice::sync() - IOCTL error <"
                << strerror(errno) << ">" << std::endl;
      return;
   }
   IsFillingBuffer    = true;
   LastWriteTimeStamp = 0;
   Balance            = 0;
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

   // ====== Write data to buffer ===========================================
   const bool ok = (Buffer.write((char*)buffer,len) == (ssize_t)len);

   const size_t bytes = Buffer.bytesReadable();
   if(bytes >= ResizeThreshold) {
#ifdef DEBUG
      printTimeStamp(std::cout);
      std::cout << "Content resize: " << bytes << " -> ";
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
         std::cout << out << std::endl;
#endif
         if(Buffer.write((char*)&resizeBuffer,out) != (ssize_t)out) {
            std::cerr << "ERROR: RingBuffer write error!" << std::endl;
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


// ###### Audio data copy thread ############################################
void AudioDevice::run()
{
   for(;;) {
      Buffer.wait();

      const double bytesPerMicroSecond = (double)
         (((cardinal)DeviceSamplingRate * (cardinal)DeviceChannels * (cardinal)DeviceBits) / 8) /
         1000000.0;
      const cardinal jitterCompensationBufferSize = (cardinal)
         rint(JitterCompensationLatency * bytesPerMicroSecond);

      if(IsFillingBuffer) {
#ifdef DEBUG
         std::cout << "filling: " << Buffer.bytesReadable() << "/"
                   << jitterCompensationBufferSize << std::endl;
#endif
         if(Buffer.bytesReadable() >= jitterCompensationBufferSize) {
            IsFillingBuffer    = false;
            LastWriteTimeStamp = 0;
         }
      }

      if((!IsFillingBuffer) && (IsReady == true)) {
         const card64 now = getMicroTime();

         if(LastWriteTimeStamp != 0) {
            const card64 delay = now - LastWriteTimeStamp;
            Balance -= (integer)((double)delay * bytesPerMicroSecond);
         }

#ifdef DEBUG
         std::cout << "balance=" << Balance << "; min="
                   << (jitterCompensationBufferSize / 2) << std::endl;
#endif
         if(Balance < (integer)(jitterCompensationBufferSize / 2)) {
#ifdef DEBUG
            std::cout << "  => reset << std::endl;
#endif
            Balance         = 0;
            IsFillingBuffer = (Buffer.bytesReadable() < jitterCompensationBufferSize);
         }

         if(!IsFillingBuffer) {
            char buffer[DeviceFragmentSize];
            ssize_t dataRead;
            ssize_t dataWritten;
            do {
               dataRead = Buffer.read((char*)&buffer,sizeof(buffer));
               if(dataRead > 0) {
                  dataWritten = ::write(DeviceFD,(char*)&buffer,dataRead);
                  if(dataWritten > 0) {
                     // ====== Update balance =====================================
                     Balance += (integer)dataWritten;
                  }
               }
               else {
                  dataRead = -1;
               }
            } while(dataWritten == dataRead);
         }

         LastWriteTimeStamp = now;
      }
   }
}
