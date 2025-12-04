// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Simple Audio Decoder Implementation                              ####
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
#include "simpleaudiodecoder.h"
#include "simpleaudiopacket.h"
#include "seqnumvalidator.h"
#include "tools.h"


// ###### Constructor #######################################################
SimpleAudioDecoder::SimpleAudioDecoder(AudioWriterInterface* device)
   : Synchronizable("SimpleAudioDecoder")
{
   Device            = device;
   AudioChannels     = 0;
   AudioBits         = 0;
   AudioSamplingRate = 0;
   Position          = 0;
   MaxPosition       = 0;
   ErrorCode         = 0;
   WantedQuality     = AudioQuality::HighestQuality;
   Media.reset();

   // SeqNumValidator with maxMisorder = 1
   //      => all packets not in sequence will be dropped!
   // minSequential = 1 => Check is done by RTPReceiver -> unnecessary here!
   SeqNumber = SeqNumValidator(1,1);
}


// ###### Destructor ########################################################
SimpleAudioDecoder::~SimpleAudioDecoder()
{
   deactivate();
}


// ###### Activate decoder ##################################################
void SimpleAudioDecoder::activate()
{
   reset();
}


// ###### Deactivate decoder ################################################
void SimpleAudioDecoder::deactivate()
{
   reset();
}


// ###### Reset decoder #####################################################
void SimpleAudioDecoder::reset()
{
   AudioChannels     = 0;
   AudioBits         = 0;
   AudioSamplingRate = 0;
   Position          = 0;
   MaxPosition       = 0;
   ErrorCode         = 0;
   WantedQuality     = AudioQuality::HighestQuality;
   Media.reset();
   SeqNumber.reset();
}


// ###### Get MediaInfo #####################################################
void SimpleAudioDecoder::getMediaInfo(MediaInfo& mediaInfo) const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   mediaInfo = Media;
   ((SimpleAudioDecoder*)this)->unsynchronized();
}


// ###### Get error code ####################################################
card8 SimpleAudioDecoder::getErrorCode() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const card8 errorCode = ErrorCode;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(errorCode);
}


// ###### Get number of audio channels ######################################
card8 SimpleAudioDecoder::getChannels() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const card8 channels = AudioChannels;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(channels);
}


// ###### Get number of audio bits ##########################################
card8 SimpleAudioDecoder::getBits() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const card8 bits = AudioBits;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(bits);
}


// ###### Get audio sampling rate ###########################################
card16 SimpleAudioDecoder::getSamplingRate() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const card16 rate = AudioSamplingRate;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(rate);
}


// ###### Get byte order ####################################################
card16 SimpleAudioDecoder::getByteOrder() const
{
   return(BIG_ENDIAN);
}


// ###### Get bytes per second ##############################################
cardinal SimpleAudioDecoder::getBytesPerSecond() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const cardinal bps = (AudioSamplingRate * AudioChannels * AudioBits) / 8;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(bps);
}


// ###### Get bits per sample ###############################################
cardinal SimpleAudioDecoder::getBitsPerSample() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const cardinal bps = AudioChannels * AudioBits;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(bps);
}


// ###### Get wanted quality ################################################
AudioQuality SimpleAudioDecoder::getWantedQuality() const
{
   return(WantedQuality);
}


// ###### Set wanted quality ################################################
void SimpleAudioDecoder::setWantedQuality(const AudioQualityInterface& wantedQuality)
{
   WantedQuality = wantedQuality;
}


// ###### Get position ######################################################
card64 SimpleAudioDecoder::getPosition() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const card64 position = Position;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(position);
}


// ###### Get maximum position ##############################################
card64 SimpleAudioDecoder::getMaxPosition() const
{
   ((SimpleAudioDecoder*)this)->synchronized();
   const card64 maxPosition = MaxPosition;
   ((SimpleAudioDecoder*)this)->unsynchronized();
   return(maxPosition);
}


// ###### Get decoder's type ID #############################################
const card16 SimpleAudioDecoder::getTypeID() const
{
   return(SimpleAudioPacket::SimpleAudioTypeID);
}


// ###### Get decoder's type name ###########################################
const char* SimpleAudioDecoder::getTypeName() const
{
   return((const char*)&SimpleAudioPacket::SimpleAudioTypeName);
}


// ###### Check next packet #################################################
bool SimpleAudioDecoder::checkNextPacket(DecoderPacket* decoderPacket)
{
   // ====== Check, if packet is in sequence ================================
   SeqNumValidator::ValidationResult valid =
      SeqNumber.validate((card64)decoderPacket->SequenceNumber);
   if(valid != SeqNumValidator::Valid) {
      return(false);
   }

   // ====== Check packet ===================================================
   SimpleAudioPacket* packet = (SimpleAudioPacket*)decoderPacket->Buffer;
   if(translate32(packet->FormatID) != SimpleAudioPacket::SimpleAudioFormatID) {
      return(false);
   }

   // ====== Set layers and layer ===========================================
   decoderPacket->Layer  = 0;
   decoderPacket->Layers = 1;
   return(true);
}


// ###### Handle next packet ################################################
void SimpleAudioDecoder::handleNextPacket(const DecoderPacket* decoderPacket)
{
   synchronized();

   SimpleAudioPacket* packet = (SimpleAudioPacket*)decoderPacket->Buffer;
   packet->translate();

   if(packet->Flags == SimpleAudioPacket::SAF_Data) {
      // ====== Set audio parameters and write data to AudioWriter ==========
      Position          = packet->Position;
      MaxPosition       = packet->MaxPosition;
      AudioSamplingRate = packet->SamplingRate;
      AudioChannels     = packet->Channels;
      AudioBits         = packet->Bits;
      ErrorCode         = packet->ErrorCode;

      if(ErrorCode == ME_NoError) {
         Device->setSamplingRate(AudioSamplingRate);
         Device->setChannels(AudioChannels);
         Device->setBits(AudioBits);
         Device->setByteOrder(BIG_ENDIAN);
         Device->write(&packet->Data,decoderPacket->Length - sizeof(SimpleAudioPacket));
      }
   }
   else if(packet->Flags == SimpleAudioPacket::SAF_MediaInfo) {
      const MediaInfo* mediaInfo = ((const MediaInfo*)&packet->Data[0]);
      Media = *mediaInfo;
      Media.translate();
   }

   unsynchronized();
}
