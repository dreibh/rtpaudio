// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Advanced Audio Decoder Implementation                            ####
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


#include "tdsystem.h"
#include "advancedaudiodecoder.h"
#include "advancedaudiopacket.h"
#include "audioquality.h"
#include "seqnumvalidator.h"
#include "tools.h"
#include "audioconverter.h"


// Debug mode: Display some information on error correction and sequence
//             number checks.
// #define DEBUG


// ###### Constructor #######################################################
AdvancedAudioDecoder::AdvancedAudioDecoder(AudioWriterInterface* device)
   : TimedThread(4 * (1000000 / AdvancedAudioPacket::AdvancedAudioFramesPerSecond) - 15000,
                 "AdvancedAudioDecoder")
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

   // minSequential = 1 => Check is done by RTPReceiver -> unnecessary here!
   for(cardinal i = 0;i < AdvancedAudioPacket::AdvancedAudioMaxQualityLayers;i++) {
      SeqNumber[i].reset();
   }

#ifdef DEBUG
   std::cout << "AdvancedAudioBufferSize              = " << FrameBufferSize << " Frames" << std::endl;
   std::cout << "AdvancedAudioBufferCleanUpDifference = " << BufferCleanUpDifference << std::endl;
#endif
}


// ###### Destructor ########################################################
AdvancedAudioDecoder::~AdvancedAudioDecoder()
{
   deactivate();
}


// ###### Activate decoder ##################################################
void AdvancedAudioDecoder::activate()
{
   reset();
   if(!running()) {
      start();
   }
}


// ###### Deactivate decoder ################################################
void AdvancedAudioDecoder::deactivate()
{
   if(running()) {
      stop();
   }
   reset();
}


// ###### Reset decoder #####################################################
void AdvancedAudioDecoder::reset()
{
   synchronized();

   AudioChannels       = 0;
   AudioBits           = 0;
   AudioSamplingRate   = 0;
   Position            = 0;
   MaxPosition         = 0;
   ErrorCode           = 0;
   WantedQuality       = AudioQuality::HighestQuality;
   Media.reset();

   for(cardinal i = 0;i < AdvancedAudioPacket::AdvancedAudioMaxQualityLayers;i++) {
      SeqNumber[i].reset();
   }

   std::multiset<FrameNodeItem>::iterator frameIterator = FrameSet.begin();
   while(frameIterator != FrameSet.end()) {
      FrameNode* node = (*frameIterator).Node;
      deleteFragments(&node->FragmentSetLU);
      deleteFragments(&node->FragmentSetRU);
      deleteFragments(&node->FragmentSetLL);
      deleteFragments(&node->FragmentSetRL);
      delete node;
      FrameSet.erase(frameIterator);
      frameIterator = FrameSet.begin();
   }

   unsynchronized();
}


// ###### Get MediaInfo #####################################################
void AdvancedAudioDecoder::getMediaInfo(MediaInfo& mediaInfo) const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   mediaInfo = Media;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
}


// ###### Get error code ####################################################
card8 AdvancedAudioDecoder::getErrorCode() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const card8 errorCode = ErrorCode;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(errorCode);
}


// ###### Get number of audio channels ######################################
card8 AdvancedAudioDecoder::getChannels() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const card8 channels = AudioChannels;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(channels);
}


// ###### Get number of audio bits ##########################################
card8 AdvancedAudioDecoder::getBits() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const card8 bits = AudioBits;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(bits);
}


// ###### Get audio sampling rate ###########################################
card16 AdvancedAudioDecoder::getSamplingRate() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const card16 rate = AudioSamplingRate;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(rate);
}


// ###### Get byte order ####################################################
card16 AdvancedAudioDecoder::getByteOrder() const
{
   return(LITTLE_ENDIAN);
}


// ###### Get bytes per second ##############################################
cardinal AdvancedAudioDecoder::getBytesPerSecond() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const cardinal bps = (AudioSamplingRate * AudioChannels * AudioBits) / 8;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(bps);
}


// ###### Get bits per sample ###############################################
cardinal AdvancedAudioDecoder::getBitsPerSample() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const cardinal bps = AudioChannels * AudioBits;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(bps);
}


// ###### Get wanted quality ################################################
AudioQuality AdvancedAudioDecoder::getWantedQuality() const
{
   return(WantedQuality);
}


// ###### Set wanted quality ################################################
void AdvancedAudioDecoder::setWantedQuality(const AudioQualityInterface& wantedQuality)
{
   WantedQuality = wantedQuality;
}


// ###### Get position ######################################################
card64 AdvancedAudioDecoder::getPosition() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const card64 position = Position;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(position);
}


// ###### Get maximum position ##############################################
card64 AdvancedAudioDecoder::getMaxPosition() const
{
   ((AdvancedAudioDecoder*)this)->synchronized();
   const card64 maxPosition = MaxPosition;
   ((AdvancedAudioDecoder*)this)->unsynchronized();
   return(maxPosition);
}


// ###### Get decoder's type ID #############################################
const card16 AdvancedAudioDecoder::getTypeID() const
{
   return(AdvancedAudioPacket::AdvancedAudioTypeID);
}


// ###### Get decoder's type name ###########################################
const char* AdvancedAudioDecoder::getTypeName() const
{
   return((const char*)&AdvancedAudioPacket::AdvancedAudioTypeName);
}


// ###### Check next packet #################################################
bool AdvancedAudioDecoder::checkNextPacket(DecoderPacket* decoderPacket)
{
   // ====== Check packet ===================================================
   AdvancedAudioPacket* packet = (AdvancedAudioPacket*)decoderPacket->Buffer;
   if(translate32(packet->FormatID) != AdvancedAudioPacket::AdvancedAudioFormatID) {
      return(false);
   }

   // ====== Set layers and layer ===========================================
   decoderPacket->Layer  = 0;
   decoderPacket->Layers = AdvancedAudioPacket::calculateLayers(
      AudioQuality(translate16(packet->SamplingRate),packet->Bits,packet->Channels));

   if(decoderPacket->Layers > 3) {
      decoderPacket->Layers = 3;
   }
   if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteUpper)) ==
        (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteUpper)) {
      decoderPacket->Layer = 0;
   }
   else if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteLower)) ==
             (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteLower)) {
      decoderPacket->Layer = (decoderPacket->Layers > 2) ? 2 : 1;
   }
   else if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteUpper)) ==
             (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteUpper)) {
      decoderPacket->Layer = 1;
   }
   else if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteLower)) ==
             (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteLower)) {
      decoderPacket->Layer = 2;
   }
   else if((packet->Flags & AdvancedAudioPacket::AAF_MediaInfo) == AdvancedAudioPacket::AAF_MediaInfo) {
      decoderPacket->Layer = 0;
   }
   else {
      return(false);
   }
#ifdef DEBUG
//   std::cout << "Layer #" << decoderPacket->Layer << ", Seq-#" << decoderPacket->SequenceNumber << std::endl;
#endif

   // ====== Verify sequence number =========================================
   SeqNumValidator::ValidationResult valid =
      SeqNumber[decoderPacket->Layer].validate((card64)decoderPacket->SequenceNumber);
   if(valid >= SeqNumValidator::Invalid) {
#ifdef DEBUG
      std::cout << "Dropped packet of layer #" << decoderPacket->Layer << "!" << std::endl;
#endif
      return(false);
   }
   if(valid == SeqNumValidator::Jumped) {
      reset();
#ifdef DEBUG
      std::cout << "Sequence number made jump => Reset!" << std::endl;
#endif
   }

   return(true);
}


// ###### Handle next packet ################################################
void AdvancedAudioDecoder::handleNextPacket(const DecoderPacket* decoderPacket)
{
   if(!running()) {
      std::cerr << "WARNING: AdvancedAudioDecoder::handleNextPacket() - Output thread is not running!" << std::endl;
      return;
   }

   AdvancedAudioPacket* packet = (AdvancedAudioPacket*)decoderPacket->Buffer;
   packet->translate();

   // ====== Add packet to list =============================================
   if(packet->ErrorCode == ME_NoError) {
      char* buffer = new char[sizeof(FrameFragment) + decoderPacket->Length - sizeof(AdvancedAudioPacket)];
      FrameFragment* fragment = (FrameFragment*)buffer;
      fragment->Fragment      = packet->Fragment;
      fragment->Length        = decoderPacket->Length - sizeof(AdvancedAudioPacket);
      memcpy((void*)&fragment->Data,(void*)&packet->Data,decoderPacket->Length - sizeof(AdvancedAudioPacket));

      synchronized();
      std::multiset<FrameNodeItem>::iterator found = FrameSet.begin();
      while(found != FrameSet.end()) {
         if((*found).Position == packet->Position) {
            break;
         }
         found++;
      }

      FrameNode* node;
      if(found == FrameSet.end()) {
         node = new FrameNode;
         node->Position     = packet->Position;
         node->MaxPosition  = packet->MaxPosition;
         node->SamplingRate = packet->SamplingRate;
         node->Channels     = packet->Channels;
         node->Bits         = packet->Bits;
         node->ErrorCode    = packet->ErrorCode;
         const AudioQuality frameQuality = AudioQuality(node->SamplingRate,
                                                        node->Bits,
                                                        node->Channels);
         node->FrameSize = getAlignedLength(frameQuality,frameQuality,
                              AdvancedAudioPacket::calculateFrameSize(
                                 frameQuality.getBytesPerSecond(),
                                 AdvancedAudioPacket::AdvancedAudioFrameSize));
         FrameNodeItem item;
         item.Position = node->Position;
         item.Node     = node;
         FrameSet.insert(item);
      }
      else {
         node = (*found).Node;
      }

      if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteUpper)) ==
            (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteUpper)) {
         if(node->FragmentSetLU.find(packet->Fragment) == node->FragmentSetLU.end()) {
            node->FragmentSetLU.insert(std::pair<const card16,FrameFragment*>(packet->Fragment,fragment));
         }
         else {
#ifdef DEBUG
            std::cout << "Received duplicate LU" << std::endl;
#endif
            delete [] (char*)fragment;
         }
      }
      else if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteLower)) ==
            (AdvancedAudioPacket::AAF_ChannelLeft|AdvancedAudioPacket::AAF_ByteLower)) {
         if(node->FragmentSetLL.find(packet->Fragment) == node->FragmentSetLL.end()) {
            node->FragmentSetLL.insert(std::pair<const card16,FrameFragment*>(packet->Fragment,fragment));
         }
         else {
#ifdef DEBUG
            std::cout << "Received duplicate LL" << std::endl;
#endif
            delete [] (char*)fragment;
         }
      }
      else if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteUpper)) ==
            (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteUpper)) {
         if(node->FragmentSetRU.find(packet->Fragment) == node->FragmentSetRU.end()) {
            node->FragmentSetRU.insert(std::pair<const card16,FrameFragment*>(packet->Fragment,fragment));
         }
         else {
#ifdef DEBUG
            std::cout << "Received duplicate RU" << std::endl;
#endif
            delete [] (char*)fragment;
         }
      }
      else if((packet->Flags & (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteLower)) ==
            (AdvancedAudioPacket::AAF_ChannelRight|AdvancedAudioPacket::AAF_ByteLower)) {
         if(node->FragmentSetRL.find(packet->Fragment) == node->FragmentSetRL.end()) {
            node->FragmentSetRL.insert(std::pair<const card16,FrameFragment*>(packet->Fragment,fragment));
         }
         else {
#ifdef DEBUG
            std::cout << "Received duplicate RL" << std::endl;
#endif
            delete [] (char*)fragment;
         }
      }
      else if((packet->Flags & AdvancedAudioPacket::AAF_MediaInfo) == AdvancedAudioPacket::AAF_MediaInfo) {
         const MediaInfo* mediaInfo = ((const MediaInfo*)&packet->Data[0]);
         Media = *mediaInfo;
         Media.translate();
         delete [] (char*)fragment;
      }
      else {
         std::cerr << "WARNING: AdvancedAudioDecoder::handleNextPacket() - Bad Fragment!" << std::endl;
         delete [] (char*)fragment;
      }
      unsynchronized();
   }

   // ====== Handle encoder errors ==========================================
   else {
      FrameNode* node = new FrameNode;
      node->Position     = packet->Position;
      node->MaxPosition  = packet->MaxPosition;
      node->SamplingRate = packet->SamplingRate;
      node->Channels     = packet->Channels;
      node->Bits         = packet->Bits;
      node->ErrorCode    = packet->ErrorCode;
      node->FrameSize    = 0;

      // Set unrecoverable error code immediately
      if(node->ErrorCode >= ME_UnrecoverableError) {
         ErrorCode = node->ErrorCode;
      }

      synchronized();
      FrameNodeItem item;
      item.Position = node->Position;
      item.Node     = node;
      FrameSet.insert(item);
      unsynchronized();
   }
}


// ###### TimedThread's timerEvent implementation: Play received audio data #
void AdvancedAudioDecoder::timerEvent()
{
   // ====== Do nothing in case of an unrecoverable error ===================
   if(ErrorCode >= ME_UnrecoverableError) {
      return;
   }
   synchronized();

   // ====== Check, if buffer has to be cleaned up ==========================
   std::multiset<FrameNodeItem>::iterator frameIterator = FrameSet.begin();
   card64 minPosition = (card64)-1;
   card64 maxPosition = 0;
   cardinal count     = 0;
   while(frameIterator != FrameSet.end()) {
      if((*frameIterator).Position < minPosition) {
         minPosition = (*frameIterator).Position;
      }
      if((*frameIterator).Position > maxPosition) {
         maxPosition = (*frameIterator).Position;
      }
      count++;
      frameIterator++;
   }

   // Clean up buffer, if the frame numbers are varying too much
   // (User has changed position, lots of transmission errors, etc.).
   if(count > 0) {
      const card64 diff = maxPosition - minPosition;
      if(diff > BufferCleanUpDifference) {
#ifdef DEBUG
         std::cout << "Buffer clean-up necessary - difference is "
              << diff << "." << std::endl;
#endif
         frameIterator = FrameSet.begin();
         while(frameIterator != FrameSet.end()) {
            FrameNode* node = (*frameIterator).Node;
            deleteFragments(&node->FragmentSetLU);
            deleteFragments(&node->FragmentSetRU);
            deleteFragments(&node->FragmentSetLL);
            deleteFragments(&node->FragmentSetRL);
            FrameSet.erase(frameIterator);
            delete node;
            frameIterator = FrameSet.begin();
         }
      }

   }


   // ====== Copy data to frame buffer; try to repair missing data ==========
   frameIterator = FrameSet.begin();
   while(frameIterator != FrameSet.end()) {
      FrameNode* node = (*frameIterator).Node;
/*
      const cardinal capacity = Device->getCurrentCapacity();
      if(capacity < node->FrameSize) {
         printTimeStamp(cout);
         printf("%d < fs=%d\n",capacity,node->FrameSize);
         break;
      }
*/

      // ====== Check, if there is a frame ready to play ====================
      if(FrameSet.size() < FrameBufferSize) {
         break;
      }
      FrameSet.erase(frameIterator);

      // ====== Decode frame ================================================
      char frameBuffer[AdvancedAudioPacket::AdvancedAudioFrameSize];
      FrameFragment* fragmentLU = NULL;
      FrameFragment* fragmentLL = NULL;
      FrameFragment* fragmentRU = NULL;
      FrameFragment* fragmentRL = NULL;
      cardinal pos       = 0;
      cardinal fragments = std::max( std::max(node->FragmentSetLU.size(),node->FragmentSetRU.size()),
                                     std::max(node->FragmentSetLL.size(),node->FragmentSetRL.size()) );
      for(cardinal fragmentNumber = 0; fragmentNumber < fragments;fragmentNumber++) {
         // ====== Left audio channel, upper 8 bits =========================
         fragmentLU = getFragment(&node->FragmentSetLU,fragmentNumber);
         if(fragmentLU == NULL) {
            fragmentLU = getFragment(&node->FragmentSetRU,fragmentNumber);
#ifdef DEBUG
            if(fragmentLU != NULL) std::cout << "Repaired LU using RU" << std::endl;
#endif
         }


         // ====== Left audio channel, lower 8 bits =========================
         if(node->Bits >= 12) {
            fragmentLL = getFragment(&node->FragmentSetLL,fragmentNumber);
            if((fragmentLL == NULL) && (fragmentLU == getFragment(&node->FragmentSetRU,fragmentNumber))) {
               fragmentLL = getFragment(&node->FragmentSetRL,fragmentNumber);
#ifdef DEBUG
               if(fragmentLL != NULL) std::cout << "Repaired LL using RL (LU==RU)" << std::endl;
#endif
            }
         }
         else
            fragmentLL = NULL;

         // ====== Right audio channel, upper 8 bits =========================
         if(node->Channels == 2) {
            fragmentRU = getFragment(&node->FragmentSetRU,fragmentNumber);
            if(fragmentRU == NULL) {
               fragmentRU = fragmentLU;
#ifdef DEBUG
               if(fragmentRU != NULL) std::cout << "Repaired RU using LU" << std::endl;
#endif
            }

            // ====== Right audio channel, lower 8 bits =========================
            if(node->Bits >= 12) {
               fragmentRL = getFragment(&node->FragmentSetRL,fragmentNumber);
               if((fragmentRL == NULL) && (fragmentRU == fragmentLU)) {
                  fragmentRL = fragmentLL;
#ifdef DEBUG
                  if(fragmentRL != NULL) std::cout << "Repaired RL using LL (RU==LU)" << std::endl;
#endif
               }
            }
            else
               fragmentRL = NULL;
         }


         // ====== Calculate fragment length ================================
         cardinal length = 0;
         if(fragmentLU != NULL)      length = fragmentLU->Length;
         else if(fragmentRU != NULL) length = fragmentRU->Length;
         else if(fragmentLL != NULL) length = fragmentLL->Length;
         else if(fragmentRL != NULL) length = fragmentRL->Length;


         // ====== Print debug information ==================================
#ifdef DEBUG
         if(fragmentLU == NULL) {
            std::cerr << "Missing LU" << std::endl;
         }
         if((node->Bits > 8) && (fragmentLL == NULL)) {
            std::cerr << "Missing LL" << std::endl;
         }
         if(node->Channels > 1) {
            if(fragmentRU == NULL) {
               std::cerr << "Missing RU" << std::endl;
            }
            if((node->Bits > 12) && (fragmentRL == NULL)) {
               std::cerr << "Missing RL" << std::endl;
            }
         }
#endif


         // ====== Join data into frame buffer ==============================
         if(length > 0) {
            if(length >= AdvancedAudioPacket::AdvancedAudioFrameSize) {
               std::cerr << "WARNING: Sum of fragments > FrameSize?!" << std::endl;
               break;
            }

            if(node->Bits <= 8) {
               if(node->Channels > 1) {
                  for(cardinal i = 0;i < length;i++) {
                     frameBuffer[pos++] = (fragmentLU == NULL) ? 0 : fragmentLU->Data[i];
                     frameBuffer[pos++] = (fragmentRU == NULL) ? 0 : fragmentRU->Data[i];
                  }
               }
               else {
                  for(cardinal i = 0;i < length;i++) {
                     frameBuffer[pos++] = (fragmentLU == NULL) ? 0 : fragmentLU->Data[i];
                  }
               }
            }
            else if(node->Bits <= 12) {
               if(node->Channels > 1) {
                  cardinal j = 0;
                  cardinal k = 0;
                  for(cardinal i = 0;i < length;i+=2) {
                     if(fragmentLU != NULL) {
                        frameBuffer[pos++] = fragmentLU->Data[j + 0];
                        frameBuffer[pos++] = fragmentLU->Data[j + 1];
                        frameBuffer[pos++] = (fragmentLL == NULL) ? 0 : fragmentLL->Data[k];
                     }
                     else {
                        frameBuffer[pos++] = 0;
                        frameBuffer[pos++] = 0;
                        frameBuffer[pos++] = 0;
                     }
                     if(fragmentRU != NULL) {
                        frameBuffer[pos++] = fragmentRU->Data[j + 0];
                        frameBuffer[pos++] = fragmentRU->Data[j + 1];
                        frameBuffer[pos++] = (fragmentLL == NULL) ? 0 : fragmentLL->Data[k + 1];
                     }
                     else {
                        frameBuffer[pos++] = 0;
                        frameBuffer[pos++] = 0;
                        frameBuffer[pos++] = 0;
                     }
                     j += 2;
                     k += 2;
                  }
               }
               else {
                  cardinal j = 0;
                  cardinal k = 0;
                  for(cardinal i = 0;i < length;i+=2) {
                     if(fragmentLU != NULL) {
                        frameBuffer[pos++] = fragmentLU->Data[j + 0];
                        frameBuffer[pos++] = fragmentLU->Data[j + 1];
                        frameBuffer[pos++] = (fragmentLL == NULL) ? 0 : fragmentLL->Data[k];
                     }
                     else {
                        frameBuffer[pos++] = 0;
                        frameBuffer[pos++] = 0;
                        frameBuffer[pos++] = 0;
                     }
                     j += 2;
                     k++;
                  }
               }
            }
            else {
               if(node->Channels > 1) {
                  for(cardinal i = 0;i < length;i++) {
                     frameBuffer[pos++] = (fragmentLL == NULL) ? 0 : fragmentLL->Data[i];
                     frameBuffer[pos++] = (fragmentLU == NULL) ? 0 : fragmentLU->Data[i];
                     frameBuffer[pos++] = (fragmentRL == NULL) ? 0 : fragmentRL->Data[i];
                     frameBuffer[pos++] = (fragmentRU == NULL) ? 0 : fragmentRU->Data[i];
                  }
               }
               else {
                  for(cardinal i = 0;i < length;i++) {
                     frameBuffer[pos++] = (fragmentLL == NULL) ? 0 : fragmentLL->Data[i];
                     frameBuffer[pos++] = (fragmentLU == NULL) ? 0 : fragmentLU->Data[i];
                  }
               }
            }
         }
      }


      // ====== Write frame to output device ================================
      if(pos > 0) {
         // ====== Clear rest of frame ======================================
         for(;pos < node->FrameSize;pos++) {
            frameBuffer[pos] = 0;
         }

         // ====== Write frame to output ====================================
         Device->setSamplingRate(node->SamplingRate);
         Device->setChannels(node->Channels);
         Device->setBits(node->Bits);
         Device->setByteOrder(LITTLE_ENDIAN);
         Device->write(&frameBuffer,pos);
      }


      // ====== Set status ==================================================
      if((pos > 0) || (node->ErrorCode > ME_NoError)) {
         Position          = node->Position;
         MaxPosition       = node->MaxPosition;
         AudioSamplingRate = node->SamplingRate;
         AudioBits         = node->Bits;
         AudioChannels     = node->Channels;
         ErrorCode         = node->ErrorCode;
      }


      // ====== Delete fragments and frame node =============================
      deleteFragments(&node->FragmentSetLU);
      deleteFragments(&node->FragmentSetRU);
      deleteFragments(&node->FragmentSetLL);
      deleteFragments(&node->FragmentSetRL);
      delete node;

      frameIterator = FrameSet.begin();
   }

   unsynchronized();
}


// ###### Delete all entries of a FrameFragment set #########################
void AdvancedAudioDecoder::deleteFragments(std::multimap<const card16,FrameFragment*>* set)
{
   if(set != NULL) {
      std::multimap<const card16,FrameFragment*>::iterator fragmentIterator = set->begin();
      while(fragmentIterator != set->end()) {
         FrameFragment* fragment = fragmentIterator->second;
         set->erase(fragmentIterator);
         delete [] fragment;
         fragmentIterator = set->begin();
      }
   }
}


// ###### Get fragment from set #############################################
AdvancedAudioDecoder::FrameFragment* AdvancedAudioDecoder::getFragment(
                  std::multimap<const card16, FrameFragment*>* set,
                  const card16                                 fragmentNumber)
{
   std::multimap<const card16, FrameFragment*>::iterator found =
      set->find(fragmentNumber);
   if(found == set->end()) {
      return(NULL);
   }
   return(found->second);
}
