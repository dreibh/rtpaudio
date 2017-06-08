// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Decoder Repository Implementation                          ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "audiodecoderrepository.h"


// ###### Constructor #######################################################
AudioDecoderRepository::AudioDecoderRepository()
{
   Decoder    = NULL;
   AutoDelete = true;
}


// ###### Destructor ########################################################
AudioDecoderRepository::~AudioDecoderRepository()
{
   if(Decoder) {
      Decoder->deactivate();
   }
   if(AutoDelete) {
      while(Repository.begin() != Repository.end()) {
         std::multimap<const card16,AudioDecoderInterface*>::iterator decoderIterator =
            Repository.begin();
         Decoder = decoderIterator->second;
         Repository.erase(decoderIterator);
         delete Decoder;
      }
   }
   Decoder = NULL;
}


// ###### Add decoder to repository #########################################
bool AudioDecoderRepository::addDecoder(AudioDecoderInterface* decoder)
{
   std::multimap<const card16,AudioDecoderInterface*>::iterator decoderIterator =
      Repository.find(decoder->getTypeID());
   if(decoderIterator == Repository.end()) {
      Repository.insert(std::pair<const card16,AudioDecoderInterface*>
                           (decoder->getTypeID(),decoder));
      if(Decoder == NULL) {
         Decoder = decoder;
      }
      return(true);
   }
   return(false);
}


// ###### Remove decoder from repository ####################################
void AudioDecoderRepository::removeDecoder(AudioDecoderInterface* decoder)
{
   std::multimap<const card16,AudioDecoderInterface*>::iterator decoderIterator =
      Repository.find(decoder->getTypeID());
   if(decoderIterator != Repository.end()) {
      Repository.erase(decoderIterator);
      decoder->deactivate();
      if(Decoder == decoder) {
         std::multimap<const card16,AudioDecoderInterface*>::iterator firstDecoder =
            Repository.begin();
         if(firstDecoder != Repository.end()) {
            Decoder = firstDecoder->second;
         }
         else {
            Decoder = NULL;
         }
      }
      else {
         Decoder = NULL;
      }
   }
}


// ###### Select decoder for given type ID ##################################
bool AudioDecoderRepository::selectDecoderForTypeID(const card16 typeID)
{
   if((Decoder == NULL) || (typeID != Decoder->getTypeID())) {
      std::multimap<const card16,AudioDecoderInterface*>::iterator decoderIterator =
         Repository.find(typeID);
      if(decoderIterator != Repository.end()) {
          AudioDecoderInterface* decoder = decoderIterator->second;
          const AudioQuality wantedQuality = Decoder->getWantedQuality();
          Decoder->deactivate();

          Decoder = decoder;

          Decoder->activate();
          Decoder->setWantedQuality(wantedQuality);
          return(true);
      }
      return(false);
   }
   return(true);
}


// ###### Get pointer to current decoder ####################################
DecoderInterface* AudioDecoderRepository::getCurrentDecoder() const
{
   return((DecoderInterface*)Decoder);
}


// ###### Get pointer to current audio decoder ##############################
AudioDecoderInterface* AudioDecoderRepository::getCurrentAudioDecoder() const
{
   return(Decoder);
}


/*
#############################################################################
###### State Pattern: Method calls                                     ######
#############################################################################
*/

const card16 AudioDecoderRepository::getTypeID() const
   { return(Decoder->getTypeID()); }

const char* AudioDecoderRepository::getTypeName() const
   { return(Decoder->getTypeName()); }

void AudioDecoderRepository::getMediaInfo(MediaInfo& mediaInfo) const
   { Decoder->getMediaInfo(mediaInfo); }

card8 AudioDecoderRepository::getErrorCode() const
   { return(Decoder->getErrorCode()); }

void AudioDecoderRepository::activate()
   { Decoder->activate(); }

void AudioDecoderRepository::deactivate()
   { Decoder->deactivate(); }

void AudioDecoderRepository::reset()
   { Decoder->reset(); }

bool AudioDecoderRepository::checkNextPacket(DecoderPacket* decoderPacket)
   { return(Decoder->checkNextPacket(decoderPacket)); }

void AudioDecoderRepository::handleNextPacket(const DecoderPacket* decoderPacket)
   { Decoder->handleNextPacket(decoderPacket); }

card8 AudioDecoderRepository::getChannels() const
   { return(Decoder->getChannels()); }

card8 AudioDecoderRepository::getBits() const
   { return(Decoder->getBits()); }

card16 AudioDecoderRepository::getSamplingRate() const
   { return(Decoder->getSamplingRate()); }

card16 AudioDecoderRepository::getByteOrder() const
   { return(Decoder->getByteOrder()); }

cardinal AudioDecoderRepository::getBitsPerSample() const
   { return(Decoder->getBitsPerSample()); }

cardinal AudioDecoderRepository::getBytesPerSecond() const
   { return(Decoder->getBytesPerSecond()); }

AudioQuality AudioDecoderRepository::getWantedQuality() const
   { return(Decoder->getWantedQuality()); }

void AudioDecoderRepository::setWantedQuality(const AudioQualityInterface& wantedQuality)
   {   Decoder->setWantedQuality(wantedQuality); }

card64 AudioDecoderRepository::getPosition() const
   { return(Decoder->getPosition()); }

card64 AudioDecoderRepository::getMaxPosition() const
   { return(Decoder->getMaxPosition()); }
