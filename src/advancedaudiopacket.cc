// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Advanced Audio Packet Implementation                             ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2026 by Thomas Dreibholz            ####
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
#include "advancedaudiopacket.h"
#include "tools.h"


// ###### Encoding name #####################################################
const char AdvancedAudioPacket::AdvancedAudioTypeName[] =
   "Advanced Audio Encoding";


// ###### Constructor #######################################################
AdvancedAudioPacket::AdvancedAudioPacket()
{
   reset();
}


// ###### Reset #############################################################
void AdvancedAudioPacket::reset()
{
   FormatID     = AdvancedAudioFormatID;
   Position     = 0;
   MaxPosition  = 0;
   SamplingRate = 0;
   Channels     = 0;
   Bits         = 0;
   ErrorCode    = 0;
   Flags        = 0;
   Fragment     = 0;
}


// ###### Translate byte order ##############################################
void AdvancedAudioPacket::translate()
{
   FormatID     = translate32(FormatID);
   SamplingRate = translate16(SamplingRate);
   Position     = translate64(Position);
   MaxPosition  = translate64(MaxPosition);
   Fragment     = translate16(Fragment);
}


struct Layer {
   card64 BytesPerSecond;
   card32 PacketsPerSecond;
   card32 FrameSize;
};

struct Level {
   static const cardinal MaxQualityLayers = 4;
   card32 BytesPerSecondScale;
   card32 PacketsPerSecondScale;
   card32 FramesPerSecond;
   card32 FramesPerSecondScale;
   card8  QualityLayers;
   Layer  QualityLayer[MaxQualityLayers];
};


// ###### Calculate TransportInfoLevel for given quality ####################
static void calculateLevelForQuality(
               Level&                       level,
               const cardinal               headerSize,
               const cardinal               maxPacketSize,
               const AudioQualityInterface& quality)
{
   // ====== Calculate layers and frame rate ================================
   const cardinal layers = AdvancedAudioPacket::calculateLayers(quality);
   cardinal fs1;
   if(quality.getBits() == 4)
      fs1 = AdvancedAudioPacket::calculateFrameSize(quality.getSamplingRate() / 2,AdvancedAudioPacket::AdvancedAudioFrameSize);
   else
      fs1 = AdvancedAudioPacket::calculateFrameSize(quality.getSamplingRate(),AdvancedAudioPacket::AdvancedAudioFrameSize);

   // ====== Initialize level ===============================================
   level.BytesPerSecondScale   = 65536;
   level.PacketsPerSecondScale = 65536;
   level.FramesPerSecond       = AdvancedAudioPacket::AdvancedAudioFramesPerSecond;
   level.FramesPerSecondScale  = 65536;

   // ====== Initialize level's layer #0 ====================================
   level.QualityLayers = 1;
   level.QualityLayer[0].BytesPerSecond   = calculateBytesPerSecond(
                                               fs1 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                               AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                               maxPacketSize, sizeof(AdvancedAudioPacket) + headerSize) +
                                            (AdvancedAudioPacket::AdvancedAudioMediaInfoPacketsPerSecond *
                                               (sizeof(AdvancedAudioPacket) + headerSize + sizeof(MediaInfo)));
   level.QualityLayer[0].PacketsPerSecond = calculatePacketsPerSecond(
                                               fs1 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                               AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                               maxPacketSize, sizeof(AdvancedAudioPacket) + headerSize) +
                                            AdvancedAudioPacket::AdvancedAudioMediaInfoPacketsPerSecond;
   level.QualityLayer[0].FrameSize =
      (card32)ceil((double)level.QualityLayer[0].BytesPerSecond /
                   (double)AdvancedAudioPacket::AdvancedAudioFramesPerSecond);

   // ====== Initialize level's layer #1 ====================================
   if(layers > 1) {
      cardinal fs2 = fs1;
      if((quality.getBits() == 12) && (quality.getChannels() == 1)) {
        fs2 = AdvancedAudioPacket::calculateFrameSize(quality.getSamplingRate() / 2,AdvancedAudioPacket::AdvancedAudioFrameSize);
      }

      level.QualityLayers = 2;
      level.QualityLayer[1].BytesPerSecond   = calculateBytesPerSecond(
                                                  fs2 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  maxPacketSize, sizeof(AdvancedAudioPacket) + headerSize);
      level.QualityLayer[1].PacketsPerSecond = calculatePacketsPerSecond(
                                                  fs2 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  maxPacketSize, sizeof(AdvancedAudioPacket) + headerSize);
      level.QualityLayer[1].FrameSize =
         (card32)ceil((double)level.QualityLayer[1].BytesPerSecond /
                      (double)AdvancedAudioPacket::AdvancedAudioFramesPerSecond);
   }

   // ====== Initialize level's layer #2 ====================================
   if(layers > 2) {
      cardinal factor;
      if(quality.getBits() == 12)
         factor = 1;
      else
         factor = 2;

      level.QualityLayers = 3;
      level.QualityLayer[2].BytesPerSecond   = factor * calculateBytesPerSecond(
                                                  fs1 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  maxPacketSize, sizeof(AdvancedAudioPacket) + headerSize);
      level.QualityLayer[2].PacketsPerSecond = factor * calculatePacketsPerSecond(
                                                  fs1 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  AdvancedAudioPacket::AdvancedAudioFramesPerSecond,
                                                  maxPacketSize, sizeof(AdvancedAudioPacket) + headerSize);
      level.QualityLayer[2].FrameSize =
         (card32)ceil((double)level.QualityLayer[2].BytesPerSecond /
                      (double)AdvancedAudioPacket::AdvancedAudioFramesPerSecond);
   }
}


// ###### Calculate encoder's quality #######################################
AudioQuality AdvancedAudioPacket::calculateQualityForLimits(
                const AudioQualityInterface& userSetting,
                const AudioQualityInterface& inputQuality,
                const card64                 totalByteRateLimit,
                const card64                 byteRateLimitL1,
                const card64                 byteRateLimitL2,
                const card64                 byteRateLimitL3,
                const cardinal               networkQualityDecrement,
                const cardinal               headerSize,
                const cardinal               maxPacketSize)
{
   // ====== Get maximum quality ============================================
   AudioQuality networkQuality = userSetting;
   networkQuality.decrease(networkQualityDecrement);
   AudioQuality newSetting = networkQuality - inputQuality;

   // cout << "=> " << newSetting << endl;

   // ====== Calculate new quality using total byte rate limit ==============
   Level level;
   while(!newSetting.isLowest()) {
      calculateLevelForQuality(level,headerSize,maxPacketSize,newSetting);
      const cardinal layers = calculateLayers(newSetting);

      // ====== Calculate total bandwidth requirements ======================
      card64 requiredTotal = level.QualityLayer[0].BytesPerSecond;
      for(cardinal i = 1;i < level.QualityLayers;i++) {
         requiredTotal += level.QualityLayer[i].BytesPerSecond;
      }

      // ====== Calculate bandwidth requirements for each level =============
      card64 required1 = level.QualityLayer[0].BytesPerSecond;
      card64 required2;
      card64 required3;
      if(layers > 1)
         required2 = level.QualityLayer[1].BytesPerSecond;
      else
         required2 = 0;
      if(layers > 2)
         required3 = level.QualityLayer[2].BytesPerSecond;
      else
         required3 = 0;

      // ====== Check, if limits are acceptable for this level ==============
      if((requiredTotal <= totalByteRateLimit) &&
         (required1 <= byteRateLimitL1) &&
         (required2 <= byteRateLimitL2) &&
         (required3 <= byteRateLimitL3)) {
         break;
      }

/*
      cout << "(" << newSetting << ")--   => ";
      cout.flush();
      printf("req=[%Ld %Ld %Ld] <-> lim=[%Ld %Ld %Ld]\n",
         required1,required2,required3,byteRateLimitL1,byteRateLimitL2,byteRateLimitL3);
*/

      newSetting--;
   }

   // cout << "<= " << newSetting << endl;

   return(newSetting);
}


// ###### Get number of layers for given quality ############################
cardinal AdvancedAudioPacket::calculateLayers(const AudioQualityInterface& quality)
{

   cardinal layers = 1;
   if(quality.getChannels() > 1)
      layers = 2;
   if(quality.getBits() >= 16)
      layers = layers << 1;
   else if(quality.getBits() >= 12)
      layers = layers + 1;
   return(layers);
}


// ###### Calculate output frame size #######################################
cardinal AdvancedAudioPacket::calculateFrameSize(const cardinal inputBytesPerSecond,
                                                 const cardinal inputFrameSize)
{
   return((inputBytesPerSecond * inputFrameSize) /
          AudioQuality::HighestQuality.getBytesPerSecond());
}
