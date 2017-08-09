// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Advanced Audio Packet Defitition                                 ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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


#ifndef ADVANCEDAUDIOPACKET_H
#define ADVANCEDAUDIOPACKET_H


#include "tdsystem.h"
#include "mediainfo.h"
#include "audioquality.h"


/**
  * This struct defines the packet format for the advanced audio encoder.
  *
  * @short   Advanced Audio Packet
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  *
  * @see AdvancedAudioEncoder
  * @see AdvancedAudioDecoder
  */
struct AdvancedAudioPacket
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   AdvancedAudioPacket();

   // ====== Byte order translation =========================================
   /**
     * Translate byte order.
     */
   void translate();


   // ====== Status functions ===============================================
   /**
     * Reset report.
     */
   void reset();


   // ====== Constants and static members ===================================
   /**
     * Type ID for Advanced Audio Encoding.
     */
   static const card16 AdvancedAudioTypeID = 0x2961;

   /**
     * Name for Advanced Audio Encoding.
     */
   static const char AdvancedAudioTypeName[];

   /**
     * Advanced Audio Encoding package format ID.
     */
   static const card32 AdvancedAudioFormatID = 0x74660000 | AdvancedAudioTypeID;

   /**
     * Advanced Audio MediaInfo packets per second.
     */
   static const cardinal AdvancedAudioMediaInfoPacketsPerSecond = 2;

   /**
     * Advanced Audio frames per second.
     */
   static const cardinal AdvancedAudioFramesPerSecond = 35;

   /**
     * Advanced Audio frame size.
     */
   static const cardinal AdvancedAudioFrameSize =
      44100 * 2 * 2 / AdvancedAudioFramesPerSecond;

   /**
     * Advanced Audio maximum transfer delay.
     */
   static const cardinal AdvancedAudioMaxTransferDelay = 100 * 16; // 100 ms.

   /**
     * Advanced Audio maximum quality layers.
     */
   static const cardinal AdvancedAudioMaxQualityLayers = 3;

   /**
     * Advanced Audio quality levels.
     */
   static const cardinal AdvancedAudioQualityLevels = AudioQuality::QualityLevels;


   /**
     * Quality calculation for given user quality limited by input quality,
     * byte rate and network quality decrement with given header size
     * (eg. IP + UDP + RTP) and maximum packet size.
     *
     * @param userSetting User's quality setting.
     * @param inputQuality Input source's quality.
     * @param byteRateLimit Byte rate limit.
     * @param byteRateLimitL1 Layer #0 byte rate limit.
     * @param byteRateLimitL2 Layer #1 byte rate limit.
     * @param byteRateLimitL3 Layer #2 byte rate limit.
     * @param networkQualityDecrement Number of steps for decrement of user's quality.
     * @param headerSize Header size (eg. IP + UDP + RTP). AdvancedAudioPacket size is added automatically.
     * @param maxPacketSize Maximum packet size.
     * @return The calculated quality.
     */
   static AudioQuality calculateQualityForLimits(
                          const AudioQualityInterface& userSetting,
                          const AudioQualityInterface& inputQuality,
                          const card64                 totalByteRateLimit,
                          const card64                 byteRateLimitL1,
                          const card64                 byteRateLimitL2,
                          const card64                 byteRateLimitL3,
                          const cardinal               networkQualityDecrement,
                          const cardinal               headerSize,
                          const cardinal               maxPacketSize);


   /**
     * Calculate output frame size from given input bytes per second and input
     * frame size.
     *
     * @param inputBytesPerSecond Input source's bytes per second.
     * @param inputFrameSize Input source's frame size.
     * @return The calculated frame size.
     */
   static cardinal calculateFrameSize(const cardinal inputBytesPerSecond,
                                      const cardinal inputFrameSize);


   /**
     * Calculate number of layers for given quality.
     *
     * @param quality Quality.
     * @return Number of layers.
     */
   static cardinal calculateLayers(const AudioQualityInterface& quality);


   // ====== Packet data ====================================================
   public:
   /**
     * Packet format ID.
     */
   card32 FormatID;

   /**
     * Audio sampling rate.
     */
   card16 SamplingRate;

   /**
     * Number of audio channels.
     */
   card8 Channels;

   /**
     * Number of audio bits.
     */
   card8 Bits;

   /**
     * Current position in nanoseconds.
     */
   card64 Position;

   /**
     * Maximum position in nanoseconds.
     */
   card64 MaxPosition;

   /**
     * Error code.
     */
   card8 ErrorCode;

   /**
     * Advanced Audio Encoding Flags.
     */
   card8 Flags;

   /**
     * Emumeration of Flags.
     */
   enum AdvancedAudioFlags {
      AAF_ChannelLeft  = (1 << 0),
      AAF_ChannelRight = (1 << 1),
      AAF_ByteUpper    = (1 << 2),
      AAF_ByteLower    = (1 << 3),
      AAF_MediaInfo    = (1 << 4)
   };

   /**
     * Fragment number.
     */
   card16 Fragment;

   /**
     * Packet data.
     */
   char Data[0];
} __attribute__((packed));


#endif
