// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Mixer implementation                                       ####
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
#include "audiomixer.h"


#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/soundcard.h>


// ###### Constructor #######################################################
AudioMixer::AudioMixer(int mixerChannel, const char* name)
{
   Channel = mixerChannel;
   Device = open(name,O_RDWR);
   if(Device < 0) {
      std::cerr << "WARNING: Unable to open AudioMixer " << name << "." << std::endl;
   }
}


// ###### Destructor ########################################################
AudioMixer::~AudioMixer()
{
   if(Device >= 0) {
      close(Device);
   }
}


// ###### Get volume ########################################################
bool AudioMixer::getVolume(card8& left, card8& right)
{
   int volume;
   if(ioctl(Device,MIXER_READ(Channel),&volume) >= 0) {
      left = (card8)(volume & 0x7f);
      right = (card8)((volume >> 8) & 0x7f);
      return(true);
   }
   return(false);
}


// ###### Set volume ########################################################
bool AudioMixer::setVolume(const card8 left, const card8 right)
{
   unsigned int l = left;
   unsigned int r = right;
   if(l > 100) l = 100;
   if(r > 100) r = 100;
   int volume = left + (r << 8);
   return(ioctl(Device,MIXER_WRITE(Channel),&volume) >= 0);
}
