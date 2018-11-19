// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Mixer implementation                                       ####
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


#include "tdsystem.h"
#include "audiomixer.h"


#ifdef HAVE_PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/introspect.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <pulse/xmalloc.h>
#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/soundcard.h>
#endif


// ###### Constructor #######################################################
AudioMixer::AudioMixer(AudioDevice* audioDevice,
                       int          mixerChannel,
                       const char*  name)
{
#ifdef HAVE_PULSEAUDIO
   Device = audioDevice;
   pa_cvolume_set(&Volume, 2, PA_VOLUME_NORM / 2);
#else
   Channel = mixerChannel;
   Device  = open(name,O_RDWR);
   if(Device < 0) {
      std::cerr << "WARNING: Unable to open AudioMixer " << name << "." << std::endl;
   }
#endif
}


// ###### Destructor ########################################################
AudioMixer::~AudioMixer()
{
#ifndef HAVE_PULSEAUDIO
   if(Device >= 0) {
      close(Device);
   }
#endif
}


#ifdef HAVE_PULSEAUDIO
// ###### Get sink info #####################################################
void AudioMixer::sink_info_cb(pa_context* context, const pa_sink_info* si, int eol, void* userData)
{
   AudioMixer* audioMixer = (AudioMixer*)userData;
   if(!eol) {
      audioMixer->Volume = si->volume;
   }
   audioMixer->VolumeUpdated.signal();
}
#endif


// ###### Get volume ########################################################
bool AudioMixer::getVolume(card8& left, card8& right)
{
#ifdef HAVE_PULSEAUDIO
   pa_threaded_mainloop_lock(Device->MainLoop);
   VolumeUpdated.fired();
   pa_operation* result =  pa_context_get_sink_info_by_index(Device->Context, 0, &sink_info_cb, (void*)this);
   pa_operation_unref(result);
   pa_threaded_mainloop_unlock(Device->MainLoop);
   if(result != NULL) {
      VolumeUpdated.wait();
   }

   left  = (card8)rint( Volume.values[0] * 100.0 / PA_VOLUME_NORM );
   right = (card8)rint( Volume.values[1] * 100.0 / PA_VOLUME_NORM );
   if(left > 100)  left  = 100;
   if(right > 100) right = 100;
#else
   int volume;
   if(ioctl(Device,MIXER_READ(Channel),&volume) < 0) {
      return(false);
   }
   else {
      left  = (card8)(volume & 0x7f);
      right = (card8)((volume >> 8) & 0x7f);
   }
#endif
   return(true);
}


// ###### Set volume ########################################################
bool AudioMixer::setVolume(const card8 left, const card8 right)
{
#ifdef HAVE_PULSEAUDIO
   assert(left  <= 100);
   assert(right <= 100);
   const double l = (left  / 100.0) * PA_VOLUME_NORM;
   const double r = (right / 100.0) * PA_VOLUME_NORM;
   Volume.values[0] = l;
   Volume.values[1] = r;

   // ====== Set the volume =================================================
   pa_threaded_mainloop_lock(Device->MainLoop);
   pa_operation* result =  pa_context_set_sink_volume_by_index(Device->Context, 0, &Volume, NULL, NULL);
   pa_operation_unref(result);
   pa_threaded_mainloop_unlock(Device->MainLoop);
   return(true);
#else
   unsigned int l = left;
   unsigned int r = right;
   if(l > 100) l = 100;
   if(r > 100) r = 100;
   int volume = left + (r << 8);
   return(ioctl(Device,MIXER_WRITE(Channel),&volume) >= 0);
#endif
}
