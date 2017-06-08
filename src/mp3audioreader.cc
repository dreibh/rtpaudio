// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### MP3 Audio Reader Implementation                                  ####
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
#include "mp3audioreader.h"


#include <stdarg.h>


// Debug mode: Print MPEGSound debug messages
// #define DEBUG


// ###### Constructor #######################################################
MP3AudioReader::MP3AudioReader(const char* name)
   : AudioQuality(0,0,0,BYTE_ORDER)
{
   BufferPos   = 0;
   BufferSize  = 0;

   FramesPerSecond = 1.0;
   Position        = 0;
   MaxPosition     = 0;

   MP3Decoder      = NULL;
   MP3Source       = NULL;
   Error           = ME_NoMedia;

   if(name != NULL) {
      openMedia(name);
   }
}


// ###### Destructor ########################################################
MP3AudioReader::~MP3AudioReader()
{
   closeMedia();
}


// ###### Open input ########################################################
bool MP3AudioReader::openMedia(const char* name)
{
   char fileName[strlen(name) + 1];

   closeMedia();
   Error = ME_BadMedia;
   strncpy((char*)&fileName, name, sizeof(fileName));
   if(name == NULL) {
      return(false);
   }

   // ====== Create Soundinputstreamfromfile object and open file ===========
   MP3Source = new Soundinputstreamfromfile();
   if(MP3Source == NULL) {
      closeMedia();
      return(false);
   }
   bool ok = MP3Source->open((char*)&fileName);
   if(ok == false) {
      closeMedia();
      return(false);
   }

   // ====== Initialize decoder =============================================
   MP3Decoder = new Mpegtoraw(MP3Source,this);
   if(MP3Decoder == NULL) {
      closeMedia();
      return(false);
   }
   if(MP3Decoder->initialize((const char*)&fileName) == false) {
      closeMedia();
      return(false);
   }
   if(!MP3Decoder->run(-1)) {
      closeMedia();
      return(false);
   }

   // ====== Initialize position ============================================
   FramesPerSecond = MP3Decoder->getfrequency() /
                        MP3Decoder->getpcmperframe();
   Position    = 0;
   MaxPosition = (card64)floor(((double)(MP3Decoder->gettotalframe() - 1) * (double)PositionStepsPerSecond) /
                               FramesPerSecond);
   Error = ME_NoError;

   return(true);
}


// ###### Close input #######################################################
void MP3AudioReader::closeMedia()
{
   if(MP3Decoder) {
      delete MP3Decoder;
      MP3Decoder = NULL;
   }
   if(MP3Source) {
      delete MP3Source;
      MP3Source = NULL;
   }

   FramesPerSecond = 1.0;
   Position        = 0;
   MaxPosition     = 0;

   BufferPos       = 0;
   BufferSize      = 0;
   Error           = ME_NoMedia;
}


// ###### Get status ########################################################
bool MP3AudioReader::ready() const
{
   return((Error == ME_NoError));
}


// ###### Get media info ###################################################
void MP3AudioReader::getMediaInfo(MediaInfo& mediaInfo) const
{
   mediaInfo.reset();
   if(MP3Decoder != NULL) {
      mediaInfo.StartTimeStamp = 0;
      mediaInfo.EndTimeStamp   = MaxPosition;

      char name[32];
      strncpy((char*)&name,MP3Decoder->getname(),30);
      if(name[0] == 0x00) {
         strcpy((char*)&name,"Untitled");
      }
      else {
         name[30] = 0x00;
      }

      char artist[32];
      strncpy((char*)&artist,MP3Decoder->getartist(),30);
      if(artist[0] == 0x00) {
         strcpy((char*)&artist,"Unknown");
      }
      else {
         artist[30] = 0x00;
      }

      char year[8];
      strncpy((char*)&year,MP3Decoder->getyear(),4);
      year[4] = 0x00;

      char comment[32];
      strncpy((char*)&comment,MP3Decoder->getcomment(),30);
      if(comment[0] == 0x00) {
         strcpy((char*)&comment,"MP3 Audio File");
      }
      else {
         comment[30] = 0x00;
      }

      snprintf((char*)&mediaInfo.Title,sizeof(mediaInfo.Title),"%s",name);
      if(year[0] != 0x00) {
         snprintf((char*)&mediaInfo.Artist,sizeof(mediaInfo.Artist),
                  "%s, %s",artist,year);
      }
      else {
         strcpy((char*)&mediaInfo.Artist,artist);
      }
      strcpy((char*)&mediaInfo.Comment,comment);
   }
}


// ###### Get error code ###################################################
MediaError MP3AudioReader::getErrorCode() const
{
   return(Error);
}


// ###### Get position ######################################################
card64 MP3AudioReader::getPosition() const
{
   return(Position);
}


// ###### Get maximum position ##############################################
card64 MP3AudioReader::getMaxPosition() const
{
   return(MaxPosition);
}


// ###### Set position ######################################################
void MP3AudioReader::setPosition(const card64 position)
{
   if(MP3Decoder != NULL) {
      if(position > MaxPosition)
         Position = MaxPosition;
      else
         Position = position;

      const cardinal frame =
         (cardinal)(floor(((double)(Position / (PositionStepsPerSecond / 1000)) * FramesPerSecond) / 1000.0));
      MP3Decoder->setframe(frame);

      // NOTE: It seems to be necessary to re-initialize the decoder after
      //       changing the position!
      MP3Decoder->run(-1);
   }
}


// ###### Read next frame from MP3 decoder ##################################
bool MP3AudioReader::readNextFrame()
{
   if(MP3Decoder == NULL)
      return(false);

   // Try to read frame
   BufferSize = 0;
   MP3Decoder->run(1);
   bool ok = (BufferSize > 0);

   // Check for error code
   int error = MP3Decoder->geterrorcode();
   if((error < -1) || (error > 0)) {
      std::cerr << "WARNING: Mpegtoraw errorcode #" << error << std::endl;
      return(false);
   }

   return(ok);
}


// ###### Read block from file ##############################################
cardinal MP3AudioReader::getNextBlock(void* buffer, const cardinal blockSize)
{
   if((MP3Decoder != NULL) && (Error < ME_UnrecoverableError)) {
      cardinal readLength = blockSize;
      char*    dest       = (char*)buffer;
      if((blockSize % (getBitsPerSample() / 8)) != 0) {
         std::cerr << "WARNING: MP3AudioReader::getNextBlock() - Unaligned blockSize value "
                   << blockSize << "!" << std::endl;
         return(0);
      }

      // Check, if there is not already data in input buffer -> read it
      bool ok = (BufferSize > 0);
      if(ok == false)
         ok = readNextFrame();

      // Read data into user's buffer
      while(ok == true) {
          const cardinal len = std::min(readLength,BufferSize - BufferPos);
          memcpy(dest,(void*)((long)&Buffer + (long)BufferPos),len);

          dest       += len;
          readLength -= len;
          BufferPos  += len;

          if(readLength <= 0)
             break;

          ok = readNextFrame();
      }

      // Update position
      Position += (PositionStepsPerSecond / 1000) *
                  (card64)(1000.0 * (double)(blockSize - readLength) /
                                    (double)getBytesPerSecond());
      if(readLength == 0)
         Error = ME_NoError;
      else
         Error = ME_EOF;

      return(blockSize - readLength);
   }
   return(0);
}


// ###### Soundplayer: initialize ###########################################
bool MP3AudioReader::initialize(char* filename)
{
   return(true);
}


// ###### Soundplayer: setsoundtype #########################################
bool MP3AudioReader::setsoundtype(int stereo, int samplesize, int speed)
{
   setSamplingRate(speed);
   setBits(samplesize);
   setChannels((stereo) ? 2 : 1);
   return(true);
}


// ###### Soundplayer: set8bitmode ##########################################
void MP3AudioReader::set8bitmode()
{
   std::cout << "MP3AudioReader::set8bitmode() - Implement me!" << std::endl;
}


// ###### Soundplayer: putblock - copy block into buffer ####################
bool MP3AudioReader::putblock(void* buffer, int size)
{

   memcpy((void*)&Buffer,buffer,size);
   BufferPos  = 0;
   BufferSize = size;
   return(true);
}


// ###### Soundplayer: putblock_nt - copy block into buffer #################
int MP3AudioReader::putblock_nt(void* buffer, int size)
{
   memcpy((void*)&Buffer,buffer,size);
   BufferPos  = 0;
   BufferSize = size;
   return(BufferSize);
}


// ###### Soundplayer: releasedevice() dummy function #######################
void MP3AudioReader::releasedevice()
{
}


// ###### Soundplayer: attachdevice() dummy function ########################
bool MP3AudioReader::attachdevice()
{
   return true;
};


// ###### Soundplayer: debug ################################################
void debug(const char *fmt, ... )
{
   char    buf[1024];
   va_list ap;

   va_start(ap, fmt);
   vsnprintf(buf,sizeof(buf),fmt,ap);
   va_end(ap);
}
