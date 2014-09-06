// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client App Packet                                          ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2014 by Thomas Dreibholz            ####
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


#ifndef AUDIOCLIENTAPPPACKET_H
#define AUDIOCLIENTAPPPACKET_H


// #include <linux/ip.h>


// ###### IPv6 Traffic Class Settings #######################################
/**
  * Default traffic class/TOS for RTP data connection from server to client.
  */
const card8 AudioServerDefaultTrafficClass = 0x00;
   // IPTOS_THROUGHPUT|IPTOS_RELIABILITY|IPTOS_LOWDELAY|IPTOS_PREC_FLASH;

/**
  * Default traffic class/TOS for RTCP control connection from client to server.
  */
const card8 AudioClientDefaultTrafficClass = 0x00;
   // IPTOS_RELIABILITY|IPTOS_PREC_PRIORITY;

/**
  * RTP Audio Server default port.
  */
static const cardinal RTPAudioDefaultPort = 7500;

/**
  * RTP Audio data PPID (for SCTP transport).
  */
static const card32 RTPAudioDataPPID = 0x2909ffff;

/**
  * RTP Audio control PPID (for SCTP transport).
  */
static const card32 RTPAudioControlPPID = 0x2909fffe;


/**
  * This struct defines the packet format for the audio client's
  * RTCP APP-PRIV messages.
  *
  * @short   Audio Client RTCP-APP Packet
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  *
  * @see AudioClient
  * @see AudioServer
  */
struct AudioClientAppPacket
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   AudioClientAppPacket();

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
     * Packet ID for AudioClient RTCP APP message.
     */
   static const card32 AudioClientFormatID = 0x75003388;

   /**
     * Definition of AudioClient commands in APP message.
     */
   enum AudioClientAppMode {
      ACAS_UnknownCommand = 0,
      ACAS_Play           = 1,
      ACAS_Pause          = 2
   };


   // ====== Packet data ====================================================
   public:
   /**
     * Packet ID.
     */
   card32 FormatID;

   /**
     * Sequence number.
     */
   card16 SequenceNumber;

   /**
     * Sequence number for position changes.
     */
   card16 PosChgSeqNumber;


   /**
     * Client status.
     */
   card16 Status;

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
     * Encoding.
     */
   card16 Encoding;

   /**
     * Suggested bandwidth or 0xffffffff, if unused.
     */
   card32 BandwidthLimit;

   /**
     * Start position in nanoseconds or 0xffff...ff, if unused.
     */
   card64 StartPosition;

   /**
     * Position to start from if server has been restarted.
     */
   card64 RestartPosition;

   /**
     * Media name, e.g. "AudioFiles/Test1.list".
     */
   char MediaName[128];
} __attribute__((packed));


/**
  * This struct defines the packet format for the audio client's
  * RTCP SDES-PRIV messages.
  *
  * @short   Audio Client RTCP SDES-PRIV Packet
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  *
  * @see AudioClient
  * @see AudioServer
  */
struct AudioClientSDESPrivPacket
{
   card8                PrefixLength;
   char                 Prefix[7];
   AudioClientAppPacket Status;
} __attribute__((packed));


#endif
