// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Mixer                                                      ####
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


#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H


#include "tdsystem.h"

#include <sys/soundcard.h>


namespace Coral {


/**
  * This class is an interface to an audio mixer.
  *
  * @short   Audio Mixer
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class AudioMixer
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param mixerChannel Mixer channel (e.g. SOUND_MIXER_PCM).
     * @param name Mixer device name (e.g. "/dev/mixer").
     */
   AudioMixer(int mixerChannel = SOUND_MIXER_PCM, const char* name = "/dev/mixer");

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
   int Device;
   int Channel;
};

}


#include "audiomixer.icc"


#endif
