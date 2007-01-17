// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Advanced Audio Encoder                                           ####
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


#ifndef ADVANCEDAUDIOENCODER_H
#define ADVANCEDAUDIOENCDOER_H


#include "tdsystem.h"
#include "audioencoderinterface.h"
#include "audioreaderinterface.h"
#include "audioquality.h"


namespace Coral {


/**
  * This class is an advanced audio encoder. It does error correction by
  * using nearly redundant data of left and right channel to "reconstruct"
  * the full data.
  *
  * @short   Advanced Audio Encoder
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */            
class AdvancedAudioEncoder : public AudioEncoderInterface,
                             public AudioQuality
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor for the audio encoder.
     *
     * @param audioReader AudioReaderInterface for the audio input.
     */
   AdvancedAudioEncoder(AudioReaderInterface* audioReader);
   
   /**
     * Destructor.
     */
   ~AdvancedAudioEncoder();


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

   card64       FramePosition;                       // Current frame
   card64       FrameMaxPosition;
   AudioQuality FrameQualitySetting;
   card8*       FrameBufferLL;
   card8*       FrameBufferRL;
   card8*       FrameBufferLU;
   card8*       FrameBufferRU;
   cardinal     FrameBufferPosLL;
   cardinal     FrameBufferPosRL;
   cardinal     FrameBufferPosLU;
   cardinal     FrameBufferPosRU;
   cardinal     FrameFragmentLL;
   cardinal     FrameFragmentRL;
   cardinal     FrameFragmentLU;
   cardinal     FrameFragmentRU;
   cardinal     FrameBufferSizeLL;
   cardinal     FrameBufferSizeRL;
   cardinal     FrameBufferSizeLU;
   cardinal     FrameBufferSizeRU;
   cardinal     FrameLayerLL;
   cardinal     FrameLayerRL;
   cardinal     FrameLayerLU;
   cardinal     FrameLayerRU;

   integer      MediaInfoCounter;

   card64       TotalByteRateLimit;   
   card64       ByteRateLimitL1;
   card64       ByteRateLimitL2;   
   card64       ByteRateLimitL3;
   cardinal     NetworkQualityDecrement;
   cardinal     SendError;
   cardinal     SentError;
   card8        ErrorCode;
};


}


#endif
