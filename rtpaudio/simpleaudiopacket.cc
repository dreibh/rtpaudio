// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Simple Audio Packet Implementation                               ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2015 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "simpleaudiopacket.h"
#include "tools.h"


// ###### Encoding name #####################################################
const char SimpleAudioPacket::SimpleAudioTypeName[] = "Simple Audio Encoding";


// ###### Constructor #######################################################
SimpleAudioPacket::SimpleAudioPacket()
{
   reset();
}


// ###### Reset #############################################################
void SimpleAudioPacket::reset()
{
   FormatID     = SimpleAudioFormatID;
   Position     = 0;
   MaxPosition  = 0;
   SamplingRate = 0;
   Channels     = 0;
   Bits         = 0;
   ErrorCode    = 0;
   Flags        = SAF_Data;
}


// ###### Translate byte order ##############################################
void SimpleAudioPacket::translate()
{
   FormatID     = translate32(FormatID);
   SamplingRate = translate16(SamplingRate);
   Position     = translate64(Position);
   MaxPosition  = translate64(MaxPosition);
}


// ###### Calculate encoder's quality #######################################
AudioQuality SimpleAudioPacket::calculateQualityForLimits(
                                   const AudioQualityInterface& userSetting,
                                   const AudioQualityInterface& inputQuality,
                                   const card64                 byteRateLimit,
                                   const cardinal               networkQualityDecrement,
                                   const cardinal               headerSize,
                                   const cardinal               maxPacketSize)
{
   // ====== Get maximum quality ============================================
   AudioQuality networkQuality = userSetting;
   networkQuality.decrease(networkQualityDecrement);
   AudioQuality newSetting = networkQuality - inputQuality;

   // ====== Calculate new quality using byte rate limit ====================
   card64 bps = calculateBytesPerSecond(newSetting.getBytesPerSecond(),
                   SimpleAudioFramesPerSecond,maxPacketSize,
                   sizeof(SimpleAudioPacket) + headerSize) +
                (SimpleAudioMediaInfoPacketsPerSecond *
                   (sizeof(SimpleAudioPacket) + headerSize + sizeof(MediaInfo)));
   while(bps > byteRateLimit) {
      newSetting--;
      bps = calculateBytesPerSecond(newSetting.getBytesPerSecond(),
               SimpleAudioFramesPerSecond,maxPacketSize,
               sizeof(SimpleAudioPacket) + headerSize) +
            (SimpleAudioMediaInfoPacketsPerSecond *
               (sizeof(SimpleAudioPacket) + headerSize + sizeof(MediaInfo)));
      if(newSetting.isLowest()) {
         break;
      }
   }

   return(newSetting);
}


// ###### Calculate output frame size #######################################
cardinal SimpleAudioPacket::calculateFrameSize(const cardinal inputBytesPerSecond,
                                               const cardinal inputFrameSize)
{
   return((inputBytesPerSecond * inputFrameSize) /
          AudioQuality::HighestQuality.getBytesPerSecond());
}
