// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Device                                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2024 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: thomas.dreibholz@gmail.com                             ####
// ####    WWW:   https://www.nntb.no/~dreibh/rtpaudio                   ####
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
#include "tools.h"
#include "audiodevice.h"
#include "audioconverter.h"

#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#ifdef HAVE_PULSEAUDIO
#include <pulse/error.h>
#endif


// Debug mode: Print debug information.
// #define DEBUG


// ###### Audio device constructor ##########################################
AudioDevice::AudioDevice(const char* name)
#ifndef HAVE_PULSEAUDIO
   : Thread("AudioDeviceThread")
#endif
{
   // ====== Initialize =====================================================
   IsReady                   = false;
   SyncCount                 = 0;
   JitterCompensationLatency = 250000;   // 250ms

   AudioChannels             = 0;
   AudioBits                 = 0;
   AudioSamplingRate         = 0;
   AudioByteOrder            = BYTE_ORDER;

   // ====== Open Audio Device ==============================================
#ifdef HAVE_PULSEAUDIO
   MainLoop    = NULL;
   MainLoopAPI = NULL;
   Context     = NULL;
   Stream      = NULL;

   MainLoop    = pa_threaded_mainloop_new();
   if(MainLoop == NULL) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - pa_mainloop_new() failed!" << std::endl;
      return;
   }
   MainLoopAPI = pa_threaded_mainloop_get_api(MainLoop);
   Context     = pa_context_new(MainLoopAPI, "AudioDevice");
   if(MainLoop == NULL) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - pa_context_new() failed!" << std::endl;
      return;
   }
   pa_context_set_state_callback(Context, context_state_callback, (void*)this);
   const pa_context_flags_t flags = (pa_context_flags_t)0;
   if(pa_context_connect(Context, NULL, flags, NULL) < 0) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - pa_context_connect() failed: "
               << pa_strerror(pa_context_errno(Context)) << std::endl;
      return;
   }
   pa_threaded_mainloop_lock(MainLoop);
   int result = pa_threaded_mainloop_start(MainLoop);
   if(result >= 0) {
      pa_threaded_mainloop_wait(MainLoop);
   }
   pa_threaded_mainloop_unlock(MainLoop);
   if(result < 0) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - pa_threaded_mainloop_start() failed!" << std::endl;
      return;
   }

   DeviceByteOrder    = LITTLE_ENDIAN;
   DeviceSamplingRate = 44100;
   DeviceBits         = 16;
   DeviceChannels     = 2;
#else
   DeviceFD = open(name,O_WRONLY);
   if(DeviceFD < 0) {
      std::cerr << "************************************************************" << std::endl
                << "WARNING: AudioDevice::AudioDevice() - Unable to open device!" << std::endl
                << "************************************************************" << std::endl;
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
#endif

   // ====== Initialize quality =============================================
   setSamplingRate(AudioQuality::HighestSamplingRate);
   setQuality(AudioQuality::HighestQuality);


   // ====== Print results ==================================================
#ifdef DEBUG
   std::cout << "AudioDevice:" << std::endl
             << "   DeviceSamplingRate = " << DeviceSamplingRate << std::endl
             << "   DeviceBits         = " << (cardinal)DeviceBits << std::endl
             << "   DeviceChannels     = " << (cardinal)DeviceChannels << std::endl
             << "   DeviceByteOrder    = " << ((DeviceByteOrder == LITTLE_ENDIAN) ? "Little Endian" : "Big Endian") << std::endl;
#ifndef HAVE_PULSEAUDIO
   std::cout << "   DeviceFragmentSize = " << DeviceFragmentSize << std::endl
             << "   DeviceOSpace       = " << DeviceOSpace
             << " = " << AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels).bytesToTime(DeviceOSpace) << " [s]" << std::endl
             << "   Capabilities       = ";
   if(DeviceCapabilities & DSP_CAP_REALTIME) std::cout << "<Real-time> ";
   if(DeviceCapabilities & DSP_CAP_BATCH)    std::cout << "<Batch> ";
   if(DeviceCapabilities & DSP_CAP_DUPLEX)   std::cout << "<Duplex> ";
   if(DeviceCapabilities & DSP_CAP_TRIGGER)  std::cout << "<Trigger> ";
   if(DeviceCapabilities & DSP_CAP_MMAP)     std::cout << "<MMap> ";
   std::cout << std::endl
             << "RingBuffer:" << std::endl
             << "   ResizeThreshold    = " << ResizeThreshold
             << " = " << AudioQuality(DeviceSamplingRate,DeviceBits,DeviceChannels).bytesToTime(ResizeThreshold) << " [s]" << std::endl;
#endif
#endif


#ifdef HAVE_PULSEAUDIO
   IsReady = true;
#else
   // ====== Start thread ===================================================
   IsFillingBuffer           = true;
   ResizeThreshold           = (RingBufferSize * ResizeThresholdPercent) / 100;
   LastWriteTimeStamp        = 0;
   Balance                   = 0;
   JitterCompensationLatency = 100000;   // 100ms


   IsReady = Buffer.init(RingBufferSize);
   if(IsReady == false) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - Ring buffer initialization failed!" << std::endl;
   }
   IsReady = start();
   if(IsReady == false) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - Copy thread startup failed!" << std::endl;
   }
#endif
}


// ###### Audio device destructor ###########################################
AudioDevice::~AudioDevice()
{
   IsReady = false;
#ifdef HAVE_PULSEAUDIO
   closeStream();
   if(MainLoop) {
      pa_threaded_mainloop_stop(MainLoop);
   }
   if(Context) {
      pa_context_unref(Context);
      Context = NULL;
   }
   if(MainLoop) {
      pa_threaded_mainloop_free(MainLoop);
      MainLoop    = NULL;
      MainLoopAPI = NULL;
   }
#else
   Buffer.flush();
   stop();
   if(DeviceFD >= 0) {
      close(DeviceFD);
      DeviceFD = -1;
   }
#endif
}


#ifdef HAVE_PULSEAUDIO
// ###### PulseAudio context callback #######################################
void AudioDevice::context_state_callback(pa_context* context, void* userData)
{
   AudioDevice* device = (AudioDevice*)userData;
   // printf("CALLBACK: %d\n", pa_context_get_state(context));
   switch(pa_context_get_state(context)) {
        case PA_CONTEXT_READY:
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(device->MainLoop, 0);
         break;
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
         break;
    }
}

// ###### Open PulseAudio stream ############################################
bool AudioDevice::openStream()
{
   bool result = false;

   assert(Stream == NULL);

   pa_threaded_mainloop_lock(MainLoop);
   pa_sample_spec sampleSpec;
   sampleSpec.format   = PA_SAMPLE_S16LE;
   sampleSpec.rate     = DeviceSamplingRate;
   sampleSpec.channels = DeviceChannels;
   Stream = pa_stream_new(Context, "AudioDeviceStream", &sampleSpec, NULL);
   if(Stream != NULL) {
      pa_buffer_attr attr;
      memset(&attr, 0, sizeof(attr));
      const double bps = DeviceSamplingRate * DeviceChannels * DeviceBits / 8;
      attr.tlength   = (uint32_t)rint(bps * JitterCompensationLatency / 1000000.0);
      attr.maxlength = (uint32_t)4*attr.tlength;
      attr.prebuf    = (uint32_t)-1;
      attr.minreq    = (uint32_t)-1;

      const pa_stream_flags_t flags = (pa_stream_flags_t)(PA_STREAM_INTERPOLATE_TIMING|PA_STREAM_AUTO_TIMING_UPDATE|PA_STREAM_EARLY_REQUESTS);
      if(pa_stream_connect_playback(Stream, NULL, &attr, flags, NULL, NULL) >= 0) {
         result = true;
      }
      else {
         std::cerr << "ERROR: AudioDevice::openStream() - pa_stream_connect_playback() failed: "
                   << pa_strerror(pa_context_errno(Context)) << std::endl;
      }
   }
   else {
      std::cerr << "ERROR: AudioDevice::openStream() - pa_stream_new() failed: "
                << pa_strerror(pa_context_errno(Context)) << std::endl;
   }
   pa_threaded_mainloop_unlock(MainLoop);

   return(result);
}


// ###### Close PulseAudio stream ###########################################
void AudioDevice::closeStream()
{
   if(Stream) {
      pa_threaded_mainloop_lock(MainLoop);
      pa_stream_disconnect(Stream);
      pa_stream_unref(Stream);
      pa_threaded_mainloop_unlock(MainLoop);
      Stream = NULL;
   }
}
#endif


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
   if(bits != AudioBits) {
      AudioBits = bits;
   }
   return(AudioBits);
}


// ###### Set number of channels ############################################
card8 AudioDevice::setChannels(const card8 channels) {
#ifdef HAVE_PULSEAUDIO
   if(channels != AudioChannels) {
      AudioChannels  = channels;
      DeviceChannels = channels;
      sync();
   }
#else
   if(DeviceFD < 0) {
      return(AudioChannels);
   }

   synchronized();
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
   unsynchronized();
#endif
   return(AudioChannels);
}


// ###### Set sampling rate #################################################
card16 AudioDevice::setSamplingRate(const card16 rate) {
#ifdef HAVE_PULSEAUDIO
   if(rate != AudioSamplingRate) {
      AudioSamplingRate  = rate;
      DeviceSamplingRate = rate;
      sync();
   }
#else
   if(DeviceFD < 0) {
      return(AudioSamplingRate);
   }

   synchronized();
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
   unsynchronized();
#endif
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
   // ====== Flush device ===================================================
#ifdef HAVE_PULSEAUDIO
#ifdef DEBUG
   std::cout << "sync!" << std::endl;
#endif
   closeStream();
   openStream();
#else
#ifdef DEBUG
   std::cout << "sync! buffered=" << Buffer.bytesReadable() << std::endl;
#endif
   synchronized();
   if(DeviceFD >= 0) {
      // SNDCTL_DSP_SYNC has been replaced by SNDCTL_DSP_RESET, because it
      // seems that SNDCTL_DSP_SYNC has a longer delay.
      Buffer.flush();
      IsReady = (ioctl(DeviceFD,SNDCTL_DSP_RESET,0) >= 0);
      if(!IsReady) {
         std::cerr << "WARNING: AudioDevice::sync() - IOCTL error <"
                  << strerror(errno) << ">" << std::endl;
      }
   }
   IsFillingBuffer    = true;
   LastWriteTimeStamp = 0;
   Balance            = 0;
   unsynchronized();
#endif

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
#ifdef HAVE_PULSEAUDIO
   if(Stream == NULL) {
      return(false);
   }
#else
   if(DeviceFD < 0) {
      return(false);
   }
#endif

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
   if(len % (DeviceBits * DeviceChannels / 8)) {
      std::cerr << "ERROR: AudioDevice::AudioDevice() - Bad input length: " << len
                << " at bytes/sample=" << (DeviceBits * DeviceChannels / 8) << std::endl;
      return(false);
   }

#ifdef HAVE_PULSEAUDIO
   // ====== Write data into buffer =========================================
   pa_threaded_mainloop_lock(MainLoop);
   const int allowedBytes = pa_stream_writable_size(Stream);
   bool      ok           = false;
   if(allowedBytes > 0) {
      // If the buffer would be filled higher than the target length,
      // cut off some existing data to fully store the new frame.
      const int64_t delta = std::min((int64_t)0, (int64_t)allowedBytes - (int64_t)len);
#ifdef DEBUG
      if(delta < 0) {
         std::cout << "buffer almost full => delta=" << delta << std::endl;
      }
#endif
      ok = (pa_stream_write(Stream,(const char*)&buffer,len,NULL,delta,PA_SEEK_RELATIVE) >= 0);
      if(!ok) {
         std::cerr << "ERROR: AudioDevice::AudioDevice() - pa_stream_write() failed: "
                   << pa_strerror(pa_context_errno(Context)) << std::endl;
      }
   }
#ifdef DEBUG
   else {
      std::cout << "buffer full => drop!" << std::endl;
   }
#endif
   pa_threaded_mainloop_unlock(MainLoop);

#else

   // ====== Write data into buffer =========================================
   Buffer.synchronized();
   const bool ok = (Buffer.write((const char*)buffer,len) == (ssize_t)len);
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
#endif

   return(ok);
}


#ifndef HAVE_PULSEAUDIO
// ###### Audio data copy thread ############################################
void AudioDevice::run()
{
   for(;;) {
      Buffer.wait();

      synchronized();

      // ====== Calculate some constants ====================================
      const double bytesPerMicroSecond = (double)
         (((cardinal)DeviceSamplingRate * (cardinal)DeviceChannels * (cardinal)DeviceBits) / 8) /
         1000000.0;
      const cardinal jitterCompensationBufferSize = (cardinal)
         rint(JitterCompensationLatency * bytesPerMicroSecond);

      // ====== Fill buffer =================================================
      // If buffer fill mode is on, collect at least a data amount of
      // "jitterCompensationBufferSize" in the ring buffer, without playing.
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

      // ====== Check balance ===============================================
      // The buffer has been filled, now play it!
      const card64 now = getMicroTime();
      if((!IsFillingBuffer) && (IsReady == true)) {

         // ====== Update balance ===========================================
         if(LastWriteTimeStamp != 0) {
            const card64 delay = now - LastWriteTimeStamp;
            Balance -= (integer)((double)delay * bytesPerMicroSecond);
         }
#ifdef DEBUG
         std::cout << "balance=" << Balance << "; min="
                   << (jitterCompensationBufferSize / 2) << std::endl;
#endif

         // ====== Reset, if necessary ======================================
         // The buffer is assumed to be underful, if the balance is less
         // than (jitterCompensationBufferSize / 2).
         if(Balance < (integer)(jitterCompensationBufferSize / 2)) {
#ifdef DEBUG
            std::cout << "  => reset" << std::endl;
#endif
            Balance         = 0;   // Reset balance
            IsFillingBuffer = (Buffer.bytesReadable() < jitterCompensationBufferSize);
            // If we have not collected enough data yet, go into
            // buffer filling mode!
         }
      }

      // ====== Play data ===================================================
      if((!IsFillingBuffer) && (IsReady == true)) {
         unsynchronized();
         char buffer[DeviceFragmentSize];
         ssize_t dataRead;
         ssize_t dataWritten;
         do {
            dataRead = Buffer.read((char*)&buffer,sizeof(buffer));
            if(dataRead > 0) {
               dataWritten = ::write(DeviceFD,(char*)&buffer,dataRead);
               if(dataWritten > 0) {
                  // ====== Update balance ===============================
                  synchronized();
                  Balance += (integer)dataWritten;
                  unsynchronized();
               }
            }
            else {
               break;
            }
         } while(dataWritten == dataRead);
         synchronized();

         LastWriteTimeStamp = now;
      }

      unsynchronized();
   }
}
#endif
