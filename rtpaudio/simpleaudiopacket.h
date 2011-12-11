// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Simple Audio Packet                                              ####
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


#ifndef SIMPLEAUDIOPACKET_H
#define SIMPLEAUDIOPACKET_H


#include "tdsystem.h"
#include "mediainfo.h"
#include "audioquality.h"


/**
  * This class defines the packet format for the simple audio encoder.
  *
  * @short   Simple Audio Packet
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  *
  * @see SimpleAudioEncoder
  * @see SimpleAudioDecoder
  */
class SimpleAudioPacket
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   SimpleAudioPacket();

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


   // ====== Constants ======================================================
   /**
     * Type ID for Simple Audio Encoding.
     */
   static const card16 SimpleAudioTypeID = 0x1234;

   /**
     * Name for Simple Audio Encoding.
     */
   static const char SimpleAudioTypeName[];

   /**
     * Simple Audio Encoding package format ID.
     */
   static const card32 SimpleAudioFormatID = 0x74660000 | SimpleAudioTypeID;

   /**
     * Simple Audio MediaInfo packets per second.
     */
   static const cardinal SimpleAudioMediaInfoPacketsPerSecond = 1;

   /**
     * Simple Audio frames per second.
     */
   static const cardinal SimpleAudioFramesPerSecond = 15;

   /**
     * Simple Audio frame size.
     */
   static const cardinal SimpleAudioFrameSize = 2352 * 5;

   /**
     * Simple Audio maximum transfer delay.
     */
   static const cardinal SimpleAudioMaxTransferDelay = 1500 * 16; // 1500 ms.

   /**
     * Simple Audio number of quality levels.
     */
   static const cardinal SimpleAudioQualityLevels = AudioQuality::QualityLevels;

   /**
     * Simple Audio loss threshold for quality decrement.
     */
   static const double SimpleAudioUpperLossThreshold = 0.04;

   /**
     * Simple Audio loss threshold for quality increment.
     */
   static const double SimpleAudioLowerLossThreshold = 0.01;


   /**
     * Quality calculation for given user quality limited by input quality,
     * byte rate and network quality decrement with given header size
     * (eg. IP + UDP + RTP) and maximum packet size.
     *
     * @param userSetting User's quality setting.
     * @param inputQuality Input source's quality.
     * @param byteRateLimit Byte rate limit.
     * @param networkQualityDecrement Number of steps for decrement of user's quality.
     * @param headerSize Header size (eg. IP + UDP + RTP). SimpleAudioPacket size is added automatically.
     * @param maxPacketSize Maximum packet size.
     * @return The calculated quality.
     */
   static AudioQuality calculateQualityForLimits(
                          const AudioQualityInterface& userSetting,
                          const AudioQualityInterface& inputQuality,
                          const card64                 totalByteRateLimit,
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
     * Flags.
     */
   card8 Flags;

   /**
     * Emumeration of Flags.
     */
   enum SimpleAudioFlags {
      SAF_Data      = 0,
      SAF_MediaInfo = 1,
   };

   /**
     * Packet data.
     */
   char Data[0];
};


#endif

