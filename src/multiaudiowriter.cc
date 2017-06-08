// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Multi Audio Writer                                               ####
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
#include "multiaudiowriter.h"
#include "tools.h"

#include <iostream>
#include <sys/time.h>


// ###### Constructor #######################################################
MultiAudioWriter::MultiAudioWriter()
   : Synchronizable("MultiAudioWriter")
{
   AudioSamplingRate = 44100;
   AudioBits         = 16;
   AudioChannels     = 2;
   AudioByteOrder    = BYTE_ORDER;
}


// ###### Destructor ########################################################
MultiAudioWriter::~MultiAudioWriter()
{
}


// ###### Add writer to writer set ##########################################
bool MultiAudioWriter::addWriter(AudioWriterInterface* writer)
{
   synchronized();
   WriterSet.insert(writer);
   writer->setQuality(*this);
   unsynchronized();
   return(true);
}


// ###### Remove writer from writer set #####################################
void MultiAudioWriter::removeWriter(AudioWriterInterface* writer)
{
   synchronized();
   std::multiset<AudioWriterInterface*>::iterator found = WriterSet.find(writer);
   if(found != WriterSet.end()) {
      WriterSet.erase(found);
   }
   unsynchronized();
}


// ###### Get number of channels ############################################
card8 MultiAudioWriter::getChannels() const
{
   return(AudioChannels);
}


// ###### Get number of bits ################################################
card8 MultiAudioWriter::getBits() const
{
   return(AudioBits);
}


// ###### Get sampling rate #################################################
card16 MultiAudioWriter::getSamplingRate() const
{
   return(AudioSamplingRate);
}


// ###### Get byte order ####################################################
card16 MultiAudioWriter::getByteOrder() const
{
   return(AudioByteOrder);
}


// ###### Set number of bits ################################################
card8 MultiAudioWriter::setBits(const card8 bits) {
   synchronized();
   AudioBits = bits;

   std::multiset<AudioWriterInterface*>::iterator writerIterator = WriterSet.begin();
   while(writerIterator != WriterSet.end()) {
      (*writerIterator)->setBits(AudioBits);
      writerIterator++;
   }

   unsynchronized();
   return(AudioBits);
}


// ###### Set number of channels ############################################
card8 MultiAudioWriter::setChannels(const card8 channels) {
   synchronized();
   AudioChannels = channels;

   std::multiset<AudioWriterInterface*>::iterator writerIterator = WriterSet.begin();
   while(writerIterator != WriterSet.end()) {
      (*writerIterator)->setChannels(AudioChannels);
      writerIterator++;
   }

   unsynchronized();
   return(AudioChannels);
}


// ###### Set sampling rate #################################################
card16 MultiAudioWriter::setSamplingRate(const card16 rate) {
   synchronized();
   AudioSamplingRate = rate;

   std::multiset<AudioWriterInterface*>::iterator writerIterator = WriterSet.begin();
   while(writerIterator != WriterSet.end()) {
      (*writerIterator)->setSamplingRate(AudioSamplingRate);
      writerIterator++;
   }

   unsynchronized();
   return(AudioSamplingRate);
}


// ###### Set byte order ####################################################
card16 MultiAudioWriter::setByteOrder(const card16 byteOrder)
{
   synchronized();
   AudioByteOrder = byteOrder;

   std::multiset<AudioWriterInterface*>::iterator writerIterator = WriterSet.begin();
   while(writerIterator != WriterSet.end()) {
      (*writerIterator)->setByteOrder(AudioByteOrder);
      writerIterator++;
   }

   unsynchronized();
   return(AudioByteOrder);
}


// ###### Get bytes per second ##############################################
cardinal MultiAudioWriter::getBytesPerSecond() const
{
   return((AudioSamplingRate * AudioChannels * AudioBits) / 8);
}


// ###### Get bits per sample ###############################################
cardinal MultiAudioWriter::getBitsPerSample() const
{
   return(AudioChannels * AudioBits);
}


// ###### Check, if spectrum analyzer is ready ##############################
bool MultiAudioWriter::ready() const
{
   return(true);
}


// ###### Reset #############################################################
void MultiAudioWriter::sync()
{
   synchronized();

   std::multiset<AudioWriterInterface*>::iterator writerIterator = WriterSet.begin();
   while(writerIterator != WriterSet.end()) {
      (*writerIterator)->sync();
      writerIterator++;
   }

   unsynchronized();
}


// ###### Write data to spectrum analyzer ###################################
bool MultiAudioWriter::write(const void* data, const size_t length)
{
   bool success = true;
   synchronized();

   std::multiset<AudioWriterInterface*>::iterator writerIterator = WriterSet.begin();
   while(writerIterator != WriterSet.end()) {
      if((*writerIterator)->write(data,length) == false) {
         success = false;
      }
      writerIterator++;
   }

   unsynchronized();
   return(success);
}
