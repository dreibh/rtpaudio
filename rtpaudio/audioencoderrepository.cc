// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Encoder Repository Implementation                          ####
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
// $Id$


#include "tdsystem.h"
#include "audioencoderrepository.h"
#include "audioquality.h"


// ###### Constructor #######################################################
AudioEncoderRepository::AudioEncoderRepository()
{
   Encoder    = NULL;
   AutoDelete = true;
}


// ###### Destructor ########################################################
AudioEncoderRepository::~AudioEncoderRepository()
{
   if(Encoder) {
      Encoder->deactivate();
   }
   if(AutoDelete) {
      while(Repository.begin() != Repository.end()) {
         std::multimap<const card16,AudioEncoderInterface*>::iterator encoderIterator =
            Repository.begin();
         Encoder = encoderIterator->second;
         Repository.erase(encoderIterator);
         delete Encoder;
      }
   }
   Encoder = NULL;
}


// ###### Add encoder to repository #########################################
bool AudioEncoderRepository::addEncoder(AudioEncoderInterface* encoder)
{
   std::multimap<const card16,AudioEncoderInterface*>::iterator encoderIterator =
      Repository.find(encoder->getTypeID());
   if(encoderIterator == Repository.end()) {
      Repository.insert(std::pair<const card16,AudioEncoderInterface*>
                           (encoder->getTypeID(),encoder));
      if(Encoder == NULL) {
         Encoder = encoder;
      }
      return(true);
   }
   return(false);
}


// ###### Remove encoder from repository ####################################
void AudioEncoderRepository::removeEncoder(AudioEncoderInterface* encoder)
{
   std::multimap<const card16,AudioEncoderInterface*>::iterator encoderIterator =
      Repository.find(encoder->getTypeID());
   if(encoderIterator != Repository.end()) {
      Repository.erase(encoderIterator);
      encoder->deactivate();
      if(Encoder == encoder) {
         std::multimap<const card16,AudioEncoderInterface*>::iterator firstEncoder =
            Repository.begin();
         if(firstEncoder != Repository.end()) {
            Encoder = firstEncoder->second;
         }
         else {
            Encoder = NULL;
         }
      }
      else {
         Encoder = NULL;
      }
   }
}


// ###### Select encoder for given type ID ##################################
bool AudioEncoderRepository::selectEncoderForTypeID(const card16 typeID)
{
   if((Encoder == NULL) || (typeID != Encoder->getTypeID())) {
      std::multimap<const card16,AudioEncoderInterface*>::iterator encoderIterator =
         Repository.find(typeID);
      if(encoderIterator != Repository.end()) {
          AudioEncoderInterface* encoder = encoderIterator->second;
          Encoder->deactivate();
          const AudioQuality quality(*Encoder);

          Encoder = encoder;

          Encoder->setQuality(quality);
          Encoder->activate();
          return(true);
      }
      return(false);
   }
   return(true);
}


// ###### Get pointer to current encoder ####################################
EncoderInterface* AudioEncoderRepository::getCurrentEncoder() const
{
   return((EncoderInterface*)Encoder);
}


// ###### Get pointer to current audio encoder ##############################
AudioEncoderInterface* AudioEncoderRepository::getCurrentAudioEncoder() const
{
   return(Encoder);
}



/*
#############################################################################
###### State Pattern: Method calls                                     ######
#############################################################################
*/

const card16 AudioEncoderRepository::getTypeID() const
   { return(Encoder->getTypeID()); }

const char* AudioEncoderRepository::getTypeName() const
   { return(Encoder->getTypeName()); }

void AudioEncoderRepository::activate()
   { Encoder->activate(); }

void AudioEncoderRepository::deactivate()
   { Encoder->deactivate(); }

void AudioEncoderRepository::reset()
   { Encoder->reset(); }

double AudioEncoderRepository::getFrameRate() const
   { return(Encoder->getFrameRate()); }

AbstractQoSDescription* AudioEncoderRepository::getQoSDescription(
                           const cardinal pktHeaderSize,
                           const cardinal pktMaxSize,
                           const card64   offset)
   { return(Encoder->getQoSDescription(pktHeaderSize,pktMaxSize,offset)); }

void AudioEncoderRepository::updateQuality(const AbstractQoSDescription* aqd)
   { Encoder->updateQuality(aqd); }

bool AudioEncoderRepository::checkInterval(card64& time, bool& newRUList)
   { return(Encoder->checkInterval(time,newRUList)); }

bool AudioEncoderRepository::prepareNextFrame(const cardinal headerSize,
                                              const cardinal maxPacketSize,
                                              const cardinal flags)
   { return(Encoder->prepareNextFrame(headerSize,maxPacketSize,flags)); }

cardinal AudioEncoderRepository::getNextPacket(EncoderPacket* encoderPacket)
   { return(Encoder->getNextPacket(encoderPacket)); }

card8 AudioEncoderRepository::getChannels() const
   { return(Encoder->getChannels()); }

card8 AudioEncoderRepository::getBits() const
   { return(Encoder->getBits()); }

card16 AudioEncoderRepository::getSamplingRate() const
   { return(Encoder->getSamplingRate()); }

card16 AudioEncoderRepository::getByteOrder() const
   { return(Encoder->getByteOrder()); }

cardinal AudioEncoderRepository::getBytesPerSecond() const
   { return(Encoder->getBytesPerSecond()); }

cardinal AudioEncoderRepository::getBitsPerSample() const
   { return(Encoder->getBitsPerSample()); }

card8 AudioEncoderRepository::setChannels(const card8 channels)
   { return(Encoder->setChannels(channels)); }

card8 AudioEncoderRepository::setBits(const card8 bits)
   { return(Encoder->setBits(bits)); }

card16 AudioEncoderRepository::setSamplingRate(const card16 rate)
   { return(Encoder->setSamplingRate(rate)); }

card16 AudioEncoderRepository::setByteOrder(const card16 byteOrder)
   { return(Encoder->setByteOrder(byteOrder)); }
