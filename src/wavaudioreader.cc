// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### WAV Audio Reader                                                 ####
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
#include "wavaudioreader.h"


// ###### Constructor #######################################################
WavAudioReader::WavAudioReader(const char* name)
   : AudioQuality(0,0,0,LITTLE_ENDIAN)
{
   Error         = ME_NoMedia;
   StartPosition = 0;
   EndPosition   = 0;
   MaxPosition   = 0;
   Position      = 0;
   InputFD       = NULL;
   if(name != NULL) openMedia(name);
}


// ###### Destructor ########################################################
WavAudioReader::~WavAudioReader()
{
   closeMedia();
}


// ###### Close file ########################################################
void WavAudioReader::closeMedia()
{
   if(InputFD != NULL) {
      fclose(InputFD);
      InputFD = NULL;
   }
   Error         = ME_NoMedia;
   StartPosition = 0;
   EndPosition   = 0;
   MaxPosition   = 0;
   Position      = 0;
   setSamplingRate(0);
   setBits(0);
   setChannels(0);
}


// ###### Open new file #####################################################
bool WavAudioReader::openMedia(const char* name)
{
   // ###### Open file ######################################################
   closeMedia();
   Error = ME_BadMedia;
   InputFD = fopen(name,"r");
   if(InputFD == NULL) {
      std::cerr << "WARNING: Unable to open input file <" << name << ">!" << std::endl;
      return(false);
   }

   // ###### Read RIFF-WAVE-Header ##########################################
   RIFF_Header header;
   size_t result = fread((void *)&header,sizeof(RIFF_Header),1,InputFD);
   if((result != 1) || strncmp(header.RIFF,"RIFF",4) || strncmp(header.FormatID,"WAVE",4)) {
      return(false);
   }

   // ###### Read RIFF Chunks ###############################################
   RIFF_Chunk chunk;
   while(getChunk(chunk)) {
      if(!(strncmp(chunk.ID,"data",4))) {
         StartPosition = ftell(InputFD);
         EndPosition   = chunk.Length;
         MaxPosition   = EndPosition - StartPosition;

         setSamplingRate(Format.SamplesPerSec);
         setBits((8 * Format.AvgBytesPerSec) / (Format.SamplesPerSec * Format.Channels));
         setChannels(Format.Channels);

         if((getBits() != 8) && (getBits() != 16)) {
            std::cerr << "WavAudioReader::openMedia() - Bad format in file " << name
                 << "!" << std::endl;
            return(false);
         }

         Error = ME_NoError;
         return(true);
      }
      else if(!(strncmp(chunk.ID,"fmt ",4))) {
         result = fread((void*)&Format,sizeof(WAVE_Format),1,InputFD);
         if(result != 1) {
            return(false);
         }
         if(chunk.Length > sizeof(WAVE_Format)) {
            fseek(InputFD,chunk.Length - sizeof(WAVE_Format),SEEK_CUR);
         }
      }
      else {
         fseek(InputFD,chunk.Length,SEEK_CUR);
      }
   }
   return(false);
}


// ###### Read WAV chunk ####################################################
bool WavAudioReader::getChunk(RIFF_Chunk& chunk) {
   int32 result = fread((void *)&chunk,sizeof(RIFF_Chunk),1,InputFD);
   if(result != 1) {
      std::cerr << "WavAudioReader::getChunk() - read error!" << std::endl;
      return(false);
   }
   return(true);
}


// ###### Get WavAudioReader status #########################################
bool WavAudioReader::ready() const
{
   return((Error == ME_NoError));
}


// ###### Get maximum position ##############################################
card64 WavAudioReader::getMaxPosition() const
{
   const cardinal bps = getBytesPerSecond();
   if(bps > 0)
      return( ((MaxPosition * 1000) / (card64)bps) * (PositionStepsPerSecond / 1000) ) ;
   else
      return(0);
}


// ###### Get media info ###################################################
void WavAudioReader::getMediaInfo(MediaInfo& mediaInfo) const
{
   mediaInfo.reset();
   mediaInfo.StartTimeStamp = 0;
   mediaInfo.EndTimeStamp   = MaxPosition;
   strcpy((char*)&mediaInfo.Title,"Untitled");
   strcpy((char*)&mediaInfo.Artist,"Unknown");
   strcpy((char*)&mediaInfo.Comment,"RIFF WAV Audio File");
}


// ###### Get error code ###################################################
MediaError WavAudioReader::getErrorCode() const
{
   return(Error);
}


// ###### Get position ######################################################
card64 WavAudioReader::getPosition() const
{
   const cardinal bps = getBytesPerSecond();
   if(bps > 0)
      return( ((Position * 1000) / (card64)bps) * (PositionStepsPerSecond / 1000) ) ;
   else
      return(0);
}


// ###### Set position ######################################################
void WavAudioReader::setPosition(const card64 position)
{
   if((InputFD != NULL) && (Error < ME_UnrecoverableError)) {
      Position = ((position / (PositionStepsPerSecond / 1000)) * getBytesPerSecond()) / 1000;

      // Avoid misaligned position!
      Position -= (Position % 4);

      fseek(InputFD,Position + StartPosition,SEEK_SET);
   }
}


// ###### Read block from file ##############################################
cardinal WavAudioReader::getNextBlock(void* buffer, const cardinal blockSize)
{
   if((InputFD != NULL) && (Error < ME_UnrecoverableError)) {
      if(Position + blockSize <= MaxPosition) {
         if((blockSize % (getBitsPerSample() / 8)) != 0) {
            std::cerr << "WARNING: WavAudioReader::getNextBlock() - Unaligned blockSize value "
                 << blockSize << "!" << std::endl;
            return(0);
         }
         int32 result = fread((void *)buffer,blockSize,1,InputFD);
         if(result == 1) {
            Position += blockSize;
            return(blockSize);
         }
         std::cerr << "WARNING: WavAudioReader::getNextBlock() - Read error!" << std::endl;
      }
      else {
         Error = ME_EOF;
      }
   }
   return(0);
}
