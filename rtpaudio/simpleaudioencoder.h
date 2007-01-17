// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Simple Audio Encoder                                             ####
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


#ifndef SIMPLEAUDIOENCODER_H
#define SIMPLEAUDIOENCDOER_H


#include "tdsystem.h"
#include "audioencoderinterface.h"
#include "audioreaderinterface.h"
#include "audioquality.h"
#include "audioquality.h"


namespace Coral {


/**
  * This class is an simple audio encoder. It does no error correction or
  * redundant transmission.
  *
  * @short   Simple Audio Encoder
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */            
class SimpleAudioEncoder : public AudioEncoderInterface,
                           public AudioQuality
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor for the audio encoder.
     *
     * @param audioReader AudioReaderInterface for the audio input.
     */
   SimpleAudioEncoder(AudioReaderInterface* audioReader);
   
   /**
     * Destructor.
     */
   ~SimpleAudioEncoder();


   // ====== EncoderInterface implementation ================================
   /**
     * getTypeID() implementation of EncoderInterface.
     *
     * @see EncoderInterface#getTypeID
     */
   const card16 getTypeID() const;
   
   /**
     * getTypeName implementation of EncoderInterface.
     *
     * @see EncoderInterface#getTypeName
     */
   const char* getTypeName() const;

   /**
     * activate() implementation of EncoderInterface.
     *
     * @see EncoderInterface#activate
     */
   void activate();

   /**
     * deactivate() implementation of EncoderInterface.
     *
     * @see EncoderInterface#deactivate
     */
   void deactivate();

   /**
     * reset() implementation of EncoderInterface.
     *
     * @see EncoderInterface#reset
     */
   void reset();

   /**
     * checkInterval() implementation of EncoderInterface.
     *
     * @see EncoderInterface#checkInterval
     */
   bool checkInterval(card64& time, bool& newRUList);

   /**
     * prepareNextFrame() implementation of EncoderInterface.
     *
     * @see EncoderInterface#prepareNextFrame
     */
   bool prepareNextFrame(const cardinal headerSize,
                         const cardinal maxPacketSize,
                         const cardinal flags);

   /**
     * getNextPacket() implementation of EncoderInterface.
     *
     * @see EncoderInterface#getNextPacket
     */
   cardinal getNextPacket(EncoderPacket* encoderPacket);

   
   // ====== Settings ========================================================
   /**
     * getQoSDescription() implementation of EncoderInterface.
     *
     * @see EncoderInterface#getQoSDescription
     */
   AbstractQoSDescription* getQoSDescription(
                              const cardinal pktHeaderSize,
                              const cardinal pktMaxSize,
                              const card64   offset);

   /**
     * updateQuality() implementation of EncoderInterface.
     *
     * @see EncoderInterface#updateQuality
     */
   void updateQuality(const AbstractQoSDescription* aqd);


   // ====== Private data ===================================================
   private:
   AudioReaderInterface* Source;

   card8*       FrameBuffer;              // Current frame
   cardinal     FrameBufferPos;
   cardinal     FrameBufferSize;
   card64       FramePosition;
   card64       FrameMaxPosition;
   AudioQuality FrameQualitySetting;

   integer      MediaInfoCounter;

   card64       ByteRateLimit;
   cardinal     NetworkQualityDecrement;
   cardinal     SendError;
   card8        ErrorCode;
};


}


#endif
