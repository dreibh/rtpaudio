// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Advanced Audio Encoder Implementation                            ####
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


#include "tdsystem.h"
#include "advancedaudioencoder.h"
#include "advancedaudiopacket.h"
#include "audioconverter.h"
#include "tools.h"


namespace Coral {


// ###### Constructor #######################################################
AdvancedAudioEncoder::AdvancedAudioEncoder(AudioReaderInterface* audioReader)
{
   Source            = audioReader;
   FrameBufferPosLL  = 0;
   FrameBufferPosRL  = 0;
   FrameBufferPosLU  = 0;
   FrameBufferPosRU  = 0;
   FrameBufferSizeLL = 0;
   FrameBufferSizeRL = 0;
   FrameBufferSizeRU = 0;
   FrameBufferSizeLU = 0;
   ErrorCode         = 0;
   SendError         = 0;
   SentError         = 0;
   MediaInfoCounter  = 0;

   setSamplingRate(AudioQuality::HighestSamplingRate);
   setChannels(2);
   setBits(16);
      
   NetworkQualityDecrement = 0;
   TotalByteRateLimit      = (card64)-1;
   ByteRateLimitL1         = (card64)-1;
   ByteRateLimitL2         = (card64)-1;
   ByteRateLimitL3         = (card64)-1;

   FrameQualitySetting = AudioQuality(0,0,0);
   FrameBufferLL = new card8[AdvancedAudioPacket::AdvancedAudioFrameSize];
   FrameBufferRL = new card8[AdvancedAudioPacket::AdvancedAudioFrameSize];
   FrameBufferLU = new card8[AdvancedAudioPacket::AdvancedAudioFrameSize];
   FrameBufferRU = new card8[AdvancedAudioPacket::AdvancedAudioFrameSize];
   if((FrameBufferLL == NULL) || (FrameBufferRL == NULL) ||
      (FrameBufferLU == NULL) || (FrameBufferRU == NULL)) {
      ErrorCode = ME_OutOfMemory;
   }
}


// ###### Destructor ########################################################
AdvancedAudioEncoder::~AdvancedAudioEncoder()
{
   if(FrameBufferLL != NULL) {
      delete FrameBufferLL;
      FrameBufferLL = NULL;
   }
   if(FrameBufferRL != NULL) {
      delete FrameBufferRL;
      FrameBufferRL = NULL;
   }
   if(FrameBufferLU != NULL) {
      delete FrameBufferLU;
      FrameBufferLU = NULL;
   }
   if(FrameBufferRU != NULL) {
      delete FrameBufferRU;
      FrameBufferRU = NULL;
   }
}


// ###### Activate encoder ##################################################
void AdvancedAudioEncoder::activate()
{
   reset();
}


// ###### Deactivate encoder ################################################
void AdvancedAudioEncoder::deactivate()
{
   reset();
}


// ###### Reset encoder #####################################################
void AdvancedAudioEncoder::reset()
{
   NetworkQualityDecrement = 0;
   TotalByteRateLimit      = (card64)-1;
   ByteRateLimitL1         = (card64)-1;
   ByteRateLimitL2         = (card64)-1;
   ByteRateLimitL3         = (card64)-1;

   FrameQualitySetting = AudioQuality(0,0,0);
   FrameBufferPosLL  = 0;
   FrameBufferPosRL  = 0;
   FrameBufferPosLU  = 0;
   FrameBufferPosRU  = 0;
   FrameBufferSizeLL = 0;
   FrameBufferSizeRL = 0;
   FrameBufferSizeLU = 0;
   FrameBufferSizeRU = 0;
   SendError         = 0;
   SentError         = 0;
   if((FrameBufferLL == NULL) || (FrameBufferRL == NULL) ||
      (FrameBufferLU == NULL) || (FrameBufferRU == NULL)) {
      ErrorCode = ME_OutOfMemory;
   }
   else {
      ErrorCode = 0;
   }

   MediaInfoCounter = 0;
}


// ###### Get encoder's type ID ##############################################
const card16 AdvancedAudioEncoder::getTypeID() const
{
   return(AdvancedAudioPacket::AdvancedAudioTypeID);
}


// ###### Get encoder's type name ############################################
const char* AdvancedAudioEncoder::getTypeName() const
{
   return((const char*)&AdvancedAudioPacket::AdvancedAudioTypeName);
}


// ###### Get QoS description ################################################
AbstractQoSDescription* AdvancedAudioEncoder::getQoSDescription(
                           const cardinal pktHeaderSize,
                           const cardinal pktMaxSize,
                           const card64   offset)
{
   return(NULL);
}


// ###### Update quality #####################################################
void AdvancedAudioEncoder::updateQuality(const AbstractQoSDescription* aqd)
{
}


// ###### Check for new interval #############################################
bool AdvancedAudioEncoder::checkInterval(card64& time, bool& newRUList)
{
   return(false);
}


// ###### Prepare frame for packaging #######################################
bool AdvancedAudioEncoder::prepareNextFrame(const cardinal headerSize,
                                            const cardinal maxPacketSize,
                                            const cardinal flags)
{
   if(ErrorCode == ME_OutOfMemory) {
      return(false);
   }
 
   // ====== Calculate frame parameters =====================================
   FrameBufferPosLL  = 0;
   FrameBufferPosRL  = 0;
   FrameBufferPosLU  = 0;
   FrameBufferPosRU  = 0;
   FrameBufferSizeLL = 0;
   FrameBufferSizeRL = 0;
   FrameBufferSizeLU = 0;
   FrameBufferSizeRU = 0;
   FrameLayerLU      = 0;
   FrameLayerRU      = 1;
   FrameLayerLL      = 2;
   FrameLayerRL      = 2;
   FrameFragmentLL   = 0;
   FrameFragmentRL   = 0;
   FrameFragmentLU   = 0;
   FrameFragmentRU   = 0;
   FramePosition     = Source->getPosition();
   FrameMaxPosition  = Source->getMaxPosition();
   const AudioQuality inputQuality(*Source);
   FrameQualitySetting = AdvancedAudioPacket::calculateQualityForLimits(
                            (AudioQuality)*this,inputQuality,
                            TotalByteRateLimit,
                            ByteRateLimitL1,ByteRateLimitL2,ByteRateLimitL3,
                            NetworkQualityDecrement,
                            headerSize,maxPacketSize);
   FrameQualitySetting.setByteOrder(LITTLE_ENDIAN);

   // ====== Read frame from AudioReader ====================================
   cardinal len = AdvancedAudioPacket::calculateFrameSize(
                           inputQuality.getBytesPerSecond(),
                           AdvancedAudioPacket::AdvancedAudioFrameSize);
   card8 buffer[len];
   if((Source->getPosition() < Source->getMaxPosition()) && (Source->getNextBlock((void*)&buffer,len) == len)) {
      // Check, if conversion is necessary
      if(inputQuality != FrameQualitySetting) {
         AudioConverter(inputQuality,FrameQualitySetting,
                           (card8*)&buffer,(card8*)&buffer,len,len);
      }
      len = getAlignedLength(inputQuality,FrameQualitySetting,len);

      cardinal i;
      cardinal blocks;

      card8* bufferLL = FrameBufferLL;
      card8* bufferRL = FrameBufferRL;
      card8* bufferLU = FrameBufferLU;
      card8* bufferRU = FrameBufferRU;
      if(FrameQualitySetting.getBits() <= 8) {
         if(FrameQualitySetting.getChannels() == 1) {
            blocks = len;
            for(i = 0;i < blocks;i++) {
               bufferLU[i] = buffer[i];
            }
            FrameBufferSizeLU = blocks;
         }
         else {
            blocks = len >> 1;
            for(i = 0;i < blocks;i++) {
               bufferLU[i] = buffer[i << 1];
               bufferRU[i] = buffer[(i << 1) + 1];
            }
            FrameBufferSizeLU = blocks;
            FrameBufferSizeRU = blocks;
         }
      }
      else if(FrameQualitySetting.getBits() <= 12) {
         if(FrameQualitySetting.getChannels() == 1) {
            FrameLayerLL = 1;
            blocks = len / 3;
            cardinal j = 0;
            cardinal k = 0;
            for(i = 0;i < len;i += 3) {
               bufferLU[j++] = buffer[i    ];
               bufferLU[j++] = buffer[i + 1];
               bufferLL[k++] = buffer[i + 2];
            }
            FrameBufferSizeLU = j;
            FrameBufferSizeLL = k;
         }
         else {
            FrameLayerRU = 1;
            FrameLayerLL = 2;
            cardinal j = 0;
            cardinal k = 0;
            for(i = 0;i < len;i += 6) {
               bufferLU[j]   = buffer[i    ];
               bufferRU[j++] = buffer[i + 3];
               bufferLU[j]   = buffer[i + 1];
               bufferRU[j++] = buffer[i + 4];
               bufferLL[k++] = buffer[i + 2];
               bufferLL[k++] = buffer[i + 5];
            }
            FrameBufferSizeLU = j;
            FrameBufferSizeRU = j;
            FrameBufferSizeLL = k;
         }
      }
      else {
         if(FrameQualitySetting.getChannels() == 1) {
            FrameLayerLL = 1;
            blocks = len / 2;
            for(i = 0;i < blocks;i++) {
               bufferLU[i] = buffer[(i << 1) + 1];
               bufferLL[i] = buffer[(i << 1) + 0];
            }
            FrameBufferSizeLL = blocks;
            FrameBufferSizeLU = blocks;
         }
         else {
            blocks = len / 4;
            for(i = 0;i < blocks;i++) {
               bufferLU[i] = buffer[(i << 2) + 1];
               bufferLL[i] = buffer[(i << 2) + 0];
               bufferRU[i] = buffer[(i << 2) + 3];
               bufferRL[i] = buffer[(i << 2) + 2];
            }
            FrameBufferSizeLL = blocks;
            FrameBufferSizeRL = blocks;
            FrameBufferSizeLU = blocks;
            FrameBufferSizeRU = blocks;
         }
      }

      MediaInfoCounter--;
      ErrorCode = ME_NoError;
      SendError = 0;
      SentError = 0;

      return(true);
   }
   else {
      // Source has an error -> send error message
      card8 error = (card8)Source->getErrorCode();
      if(error == 0) {
         if(Source->getPosition() >= Source->getMaxPosition()) {
            error = ME_EOF;
         }
      }
      if(error > 0) {
         if(SendError == 0) {
            ErrorCode         = error;
            FrameBufferPosLU  = 0;
            FrameBufferPosRU  = AdvancedAudioPacket::AdvancedAudioFrameSize;
            FrameBufferPosLL  = AdvancedAudioPacket::AdvancedAudioFrameSize;
            FrameBufferPosRL  = AdvancedAudioPacket::AdvancedAudioFrameSize;
            FrameBufferSizeLU = 4;
            FrameBufferLU[0]  = 0x00;

            // Send empty frames for 1 second before decreasing send rate to
            // let the decoder play all frames stored in buffer.
            if((SentError < 1 * AdvancedAudioPacket::AdvancedAudioFramesPerSecond) && (error < ME_UnrecoverableError))
               SentError++;
            else
               SendError = AdvancedAudioPacket::AdvancedAudioFramesPerSecond;

            return(true);
         }
         if(SendError > 0) SendError--;
      }
   }

   return(false);
}


// ###### Get next packet from current frame ################################
cardinal AdvancedAudioEncoder::getNextPacket(EncoderPacket* encoderPacket)
{ 
   // ====== Create packet ==================================================
   cardinal bytes = 0;
   AdvancedAudioPacket* packet = (AdvancedAudioPacket*)encoderPacket->Buffer;
   encoderPacket->Layer        = 0;
   encoderPacket->PayloadType  = (card8)(AdvancedAudioPacket::AdvancedAudioTypeID & 0x00ff);

   packet->FormatID     = AdvancedAudioPacket::AdvancedAudioFormatID;
   packet->Position     = FramePosition;
   packet->MaxPosition  = FrameMaxPosition;
   packet->ErrorCode    = ErrorCode;

   packet->SamplingRate = FrameQualitySetting.getSamplingRate();
   packet->Bits         = FrameQualitySetting.getBits();
   packet->Channels     = FrameQualitySetting.getChannels();

   // ====== Get payload data ===============================================
   if(FrameBufferPosLU < FrameBufferSizeLU) {
      bytes = min(encoderPacket->MaxLength - sizeof(AdvancedAudioPacket),
                  FrameBufferSizeLU - FrameBufferPosLU);
      encoderPacket->Layer = FrameLayerLU;
      for(cardinal i = 0;i < bytes;i++) {
         packet->Data[i] = FrameBufferLU[FrameBufferPosLU++];
      }
      packet->Flags    = AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteUpper;
      packet->Fragment = (card16)FrameFragmentLU;
      FrameFragmentLU++;
   }
   else if(FrameBufferPosRU < FrameBufferSizeRU) {
      bytes = min(encoderPacket->MaxLength - sizeof(AdvancedAudioPacket),
                  FrameBufferSizeRU - FrameBufferPosRU);
      encoderPacket->Layer = FrameLayerRU;
      for(cardinal i = 0;i < bytes;i++) {
         packet->Data[i] = FrameBufferRU[FrameBufferPosRU++];
      }
      packet->Flags    = AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteUpper;
      packet->Fragment = (card16)FrameFragmentRU;
      FrameFragmentRU++;
   }
   else if(FrameBufferPosLL < FrameBufferSizeLL) {
      bytes = min(encoderPacket->MaxLength - sizeof(AdvancedAudioPacket),
                  FrameBufferSizeLL - FrameBufferPosLL);
      encoderPacket->Layer = FrameLayerLL;
      for(cardinal i = 0;i < bytes;i++) {
         packet->Data[i] = FrameBufferLL[FrameBufferPosLL++];
      }
      packet->Flags    = AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteLower;
      packet->Fragment = (card16)FrameFragmentLL;
      FrameFragmentLL++;
   }
   else if(FrameBufferPosRL < FrameBufferSizeRL) {
      bytes = min(encoderPacket->MaxLength - sizeof(AdvancedAudioPacket),
                  FrameBufferSizeRL - FrameBufferPosRL);
      encoderPacket->Layer = FrameLayerRL;
      for(cardinal i = 0;i < bytes;i++) {
         packet->Data[i] = FrameBufferRL[FrameBufferPosRL++];
      }
      packet->Flags    = AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteLower;
      packet->Fragment = (card16)FrameFragmentRL;
      FrameFragmentRL++;
   }
   else if(MediaInfoCounter <= 0) {
      MediaInfoCounter = 2 + (AdvancedAudioPacket::AdvancedAudioFramesPerSecond /
                                 AdvancedAudioPacket::AdvancedAudioMediaInfoPacketsPerSecond);

      if(encoderPacket->MaxLength >= sizeof(AdvancedAudioPacket) + sizeof(MediaInfo)) {
         MediaInfo* media = (MediaInfo*)&packet->Data[0];
         Source->getMediaInfo(*media);
         packet->Flags    = AdvancedAudioPacket::AAF_MediaInfo;
         packet->Fragment = 0;
         bytes = sizeof(MediaInfo);
      }
      else {
         cerr << "WARNING: AdvancedAudioEncoder::getNextPacket() - "
              << "Packet size too low for media info!" << endl;
      }
   }

   if(bytes > 0) {
      packet->translate();
      return(bytes + sizeof(AdvancedAudioPacket));      
   }
   return(0);
}


}
