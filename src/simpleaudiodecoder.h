// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Simple Audio Decoder                                             ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2023 by Thomas Dreibholz            ####
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


#ifndef SIMPLEAUDIODECODER_H
#define SIMPLEAUDIODECODER_H


#include "tdsystem.h"
#include "synchronizable.h"
#include "audiowriterinterface.h"
#include "audiodecoderinterface.h"
#include "seqnumvalidator.h"
#include "audioquality.h"


/**
  * This class is an simple audio decoder. It does no error correction or
  * redundant transmission.
  *
  * @short   Simple Audio Decoder
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
class SimpleAudioDecoder : public AudioDecoderInterface,
                           public Synchronizable
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor for the audio decoder.
     *
     * @param audioWriter AudioReaderInterface for the audio output.
     */
   SimpleAudioDecoder(AudioWriterInterface* audioWriter);

   /**
     * Destructor.
     */
   ~SimpleAudioDecoder();


   // ====== DecoderInterface implementation ================================
   /**
     * getTypeID() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getTypeID
     */
   const card16 getTypeID() const;

   /**
     * getTypeName implementation of DecoderInterface.
     *
     * @see DecoderInterface#getTypeName
     */
   const char* getTypeName() const;

   /**
     * activate() implementation of DecoderInterface.
     *
     * @see DecoderInterface#activate
     */
   void activate();

   /**
     * deactivate() implementation of DecoderInterface.
     *
     * @see DecoderInterface#deactivate
     */
   void deactivate();

   /**
     * reset() implementation of DecoderInterface.
     *
     * @see DecoderInterface#reset
     */
   void reset();

   /**
     * getMediaInfo() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getMediaInfo
     */
   void getMediaInfo(MediaInfo& mediaInfo) const;

   /**
     * getErrorCode() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getErrorCode
     */
   card8 getErrorCode() const;

   /**
     * getPosition() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getPosition
     */
   card64 getPosition() const;

   /**
     * getMaxPosition() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getMaxPosition
     */
   card64 getMaxPosition() const;

   /**
     * checkNextPacket() implementation of DecoderInterface.
     *
     * @see DecoderInterface#checkNextPacket
     */
   bool checkNextPacket(DecoderPacket* decoderPacket);

   /**
     * handleNextPacket() implementation of DecoderInterface.
     *
     * @see DecoderInterface#handleNextPacket
     */
   void handleNextPacket(const DecoderPacket* decoderPacket);


   // ====== AudioDecoderInterface implementation ===========================
   /**
     * getSamplingRate() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getSamplingRate
     */
   card16 getSamplingRate() const;

   /**
     * getBits() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getBits
     */
   card8 getBits() const;

   /**
     * getChannels() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getChannels
     */
   card8 getChannels() const;


   /**
     * getByteOrder() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getByteOrder
     */
   card16 getByteOrder() const;


   /**
     * getBytesPerSecond() implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getBytesPerSecond
     */
   cardinal getBytesPerSecond() const;

   /**
     * getBitsPerSample() implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getBitsPerSample
     */
   cardinal getBitsPerSample() const;


   /**
     * getWantedQuality() implementation of AudioDecoderInterface.
     *
     * see AudioDecoderInterface#getWantedQuality
     */
   AudioQuality getWantedQuality() const;

   /**
     * setWantedQuality() implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#setWantedQuality
     */
   void setWantedQuality(const AudioQualityInterface& wantedQuality);


   // ====== Private data ===================================================
   private:
   SeqNumValidator       SeqNumber;
   AudioWriterInterface* Device;
   card64                Position;
   card64                MaxPosition;
   AudioQuality          WantedQuality;

   MediaInfo             Media;
   card16                AudioSamplingRate;
   card8                 AudioBits;
   card8                 AudioChannels;
   card8                 ErrorCode;
};


#endif
