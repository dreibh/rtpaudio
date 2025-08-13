// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Mixer                                                      ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2025 by Thomas Dreibholz            ####
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


#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H


#include "tdsystem.h"
#include "audiodevice.h"

#ifdef HAVE_PULSEAUDIO
#include "condition.h"
#include <pulse/volume.h>
#include <pulse/introspect.h>
#define SOUND_MIXER_PCM 0   // Dummy value
#else
#include <sys/soundcard.h>
#endif


/**
  * This class is an interface to an audio mixer.
  *
  * @short   Audio Mixer
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
class AudioMixer
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param audioDevice AudioDevice object.
     * @param mixerChannel Mixer channel (e.g. SOUND_MIXER_PCM).
     * @param name Mixer device name (e.g. "/dev/mixer").
     */
   AudioMixer(AudioDevice* audioDevice,
              int          mixerChannel = SOUND_MIXER_PCM,
              const char*  name         = "/dev/mixer");

   /**
     * Destructor.
     */
   ~AudioMixer();


   // ====== Mixer functions ================================================
   /**
     * Check, if mixer is ready.
     *
     * @return true, if mixer is ready; false otherwise.
     */
   inline bool ready() const;

   /**
     * Get volume.
     *
     * @param left Volume of left channel.
     * @param right Volume of right channel.
     * @return true, if volume has been written into variables; false otherwise.
     */
   bool getVolume(card8& left, card8& right);

   /**
     * Set volume.
     *
     * @param left Volume of left channel.
     * @param right Volume of right channel.
     * @return true, if volume has been set; false otherwise.
     */
   bool setVolume(const card8 left, const card8 right);


   // ====== Private data ===================================================
   private:
#ifdef HAVE_PULSEAUDIO
   static void sink_info_cb(pa_context* context, const pa_sink_info* si, int eol, void* userData);

   AudioDevice* Device;
   pa_cvolume   Volume;
   Condition    VolumeUpdated;
#else
   int          Device;
   int          Channel;
#endif
};


#include "audiomixer.icc"


#endif
