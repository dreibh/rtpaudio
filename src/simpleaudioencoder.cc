// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Simple Audio Encoder Implementation                              ####
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


#include "tdsystem.h"
#include "simpleaudioencoder.h"
#include "simpleaudiopacket.h"
#include "tools.h"
#include "audioconverter.h"


// ###### Constructor #######################################################
SimpleAudioEncoder::SimpleAudioEncoder(AudioReaderInterface* audioReader)
{
   Source           = audioReader;
   FrameBufferPos   = 0;
   FrameBufferSize  = 0;
   SendError        = 0;
   MediaInfoCounter = 0;

   setSamplingRate(AudioQuality::HighestSamplingRate);
   setChannels(2);
   setBits(16);

   NetworkQualityDecrement = 0;
   ByteRateLimit           = (card64)-1;

   FrameQualitySetting = AudioQuality(0,0,0);
   FrameBuffer = new card8[SimpleAudioPacket::SimpleAudioFrameSize];
   if(FrameBuffer == NULL)
      ErrorCode = ME_OutOfMemory;
   else
      ErrorCode = 0;
}


// ###### Destructor ########################################################
SimpleAudioEncoder::~SimpleAudioEncoder()
{
   if(FrameBuffer != NULL) {
      delete [] FrameBuffer;
      FrameBuffer = NULL;
   }
}


// ###### Activate encoder ##################################################
void SimpleAudioEncoder::activate()
{
   reset();
}


// ###### Deactivate encoder ################################################
void SimpleAudioEncoder::deactivate()
{
   reset();
}


// ###### Reset encoder #####################################################
void SimpleAudioEncoder::reset()
{
   NetworkQualityDecrement = 0;
   ByteRateLimit           = (card64)-1;

   FrameQualitySetting = AudioQuality(0,0,0);
   FrameBufferPos   = 0;
   FrameBufferSize  = 0;
   SendError        = 0;
   MediaInfoCounter = 0;
   if(FrameBuffer != NULL)
      ErrorCode = 0;
   else
      ErrorCode = ME_OutOfMemory;
}


// ###### Get encoder's type ID ##############################################
const card16 SimpleAudioEncoder::getTypeID() const
{
   return(SimpleAudioPacket::SimpleAudioTypeID);
}


// ###### Get encoder's type name ############################################
const char* SimpleAudioEncoder::getTypeName() const
{
   return((const char*)&SimpleAudioPacket::SimpleAudioTypeName);
}


// ###### Get frame rate ####################################################
double SimpleAudioEncoder::getFrameRate() const
{
   return(SimpleAudioPacket::SimpleAudioFramesPerSecond);
}


// ###### Get QoS description ################################################
AbstractQoSDescription* SimpleAudioEncoder::getQoSDescription(
                           const cardinal pktHeaderSize,
                           const cardinal pktMaxSize,
                           const card64   offset)
{
   return(NULL);
}


// ###### Update quality #####################################################
void SimpleAudioEncoder::updateQuality(const AbstractQoSDescription* aqd)
{
}


// ###### Check for new interval #############################################
bool SimpleAudioEncoder::checkInterval(card64& time, bool& newRUList)
{
   return(false);
}


// ###### Prepare frame for packaging #######################################
bool SimpleAudioEncoder::prepareNextFrame(const cardinal headerSize,
                                          const cardinal maxPacketSize,
                                          const cardinal flags)
{
   if(ErrorCode == ME_OutOfMemory)
      return(false);

   // ====== Calculate frame parameters =====================================
   FrameBufferPos      = 0;
   FrameBufferSize     = 0;
   FramePosition       = Source->getPosition();
   FrameMaxPosition    = Source->getMaxPosition();
   const AudioQuality inputQuality(*Source);
   FrameQualitySetting = SimpleAudioPacket::calculateQualityForLimits(
                            (AudioQuality)*this,(AudioQuality)*Source,
                            ByteRateLimit,NetworkQualityDecrement,
                            headerSize,maxPacketSize);
   FrameQualitySetting.setByteOrder(BIG_ENDIAN);

   // ====== Read frame from AudioReader ====================================
   cardinal len = SimpleAudioPacket::calculateFrameSize(
                      inputQuality.getBytesPerSecond(),
                      SimpleAudioPacket::SimpleAudioFrameSize);
   if((Source->getPosition() < Source->getMaxPosition()) && (Source->getNextBlock((void*)FrameBuffer,len) == len)) {
      // Check, if conversion is necessary
      if(inputQuality != FrameQualitySetting) {
         AudioConverter(inputQuality,FrameQualitySetting,
                        FrameBuffer,FrameBuffer,len,len);
      }
      FrameBufferSize = getAlignedLength(inputQuality,FrameQualitySetting,len);

      ErrorCode = ME_NoError;
      SendError = 0;
      MediaInfoCounter--;
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
            ErrorCode       = error;
            FrameBufferPos  = 0;
            FrameBufferSize = 4;
            memset(FrameBuffer, 0x00, 4);
            SendError = SimpleAudioPacket::SimpleAudioFramesPerSecond;
            return(true);
         }
         if(SendError > 0) SendError--;
         return(false);
      }
   }
   return(false);
}


// ###### Get next packet from current frame ################################
cardinal SimpleAudioEncoder::getNextPacket(EncoderPacket* encoderPacket)
{
   // ====== Create packet ===============================================
   SimpleAudioPacket* packet  = (SimpleAudioPacket*)encoderPacket->Buffer;
   card8*             data    = (card8*)FrameBuffer;
   encoderPacket->Layer       = 0;
   encoderPacket->PayloadType = (card8)(SimpleAudioPacket::SimpleAudioTypeID & 0x00ff);

   packet->FormatID     = SimpleAudioPacket::SimpleAudioFormatID;
   packet->Position     = FramePosition;
   packet->MaxPosition  = FrameMaxPosition;

   packet->SamplingRate = FrameQualitySetting.getSamplingRate();
   packet->Bits         = FrameQualitySetting.getBits();
   packet->Channels     = FrameQualitySetting.getChannels();
   packet->ErrorCode    = ErrorCode;
   packet->Flags        = SimpleAudioPacket::SAF_Data;

   // ====== Get payload data ===============================================
   if(FrameBufferPos < FrameBufferSize) {

      // ====== Ensure correct alignment of data ============================
      cardinal bytes = std::min((cardinal)(encoderPacket->MaxLength - sizeof(SimpleAudioPacket)),
                                FrameBufferSize - FrameBufferPos);
      if(packet->Bits == 16) {
         if(packet->Channels == 2)
            bytes = bytes - (bytes % 4);
         else
            bytes = bytes - (bytes % 2);
      }
      else if(packet->Bits == 12) {
         if(packet->Channels == 2)
            bytes = bytes - (bytes % 6);
         else
            bytes = bytes - (bytes % 3);
      }
      if(bytes == 0) {
         return(0);
      }

      for(cardinal i = 0;i < bytes;i++) {
         packet->Data[i] = data[FrameBufferPos++];
      }

      // ====== Translate byte order ========================================
      packet->translate();
      return(bytes + sizeof(SimpleAudioPacket));
   }
   else if(MediaInfoCounter <= 0) {
      MediaInfoCounter = 2 + (SimpleAudioPacket::SimpleAudioFramesPerSecond /
                                 SimpleAudioPacket::SimpleAudioMediaInfoPacketsPerSecond);

      if(encoderPacket->MaxLength >= sizeof(SimpleAudioPacket) + sizeof(MediaInfo)) {
         MediaInfo* media = (MediaInfo*)&packet->Data[0];
         Source->getMediaInfo(*media);
         packet->Flags = SimpleAudioPacket::SAF_MediaInfo;
         packet->translate();
         return(sizeof(MediaInfo) + sizeof(SimpleAudioPacket));
      }
      else {
         std::cerr << "WARNING: SimpleAudioEncoder::getNextPacket() - "
              << "Packet size too low for media info!" << std::endl;
      }
   }
   return(0);
}
