// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Multi Audio Reader                                               ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2025 by Thomas Dreibholz            ####
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
#include "multiaudioreader.h"
#include "wavaudioreader.h"
#include "mp3audioreader.h"


// ###### Constructor #######################################################
MultiAudioReader::MultiAudioReader(const char* name, const cardinal level)
   : AudioQuality(0,0,0)
{
   Error       = ME_NoMedia;
   MaxPosition = 0;
   Position    = 0;
   Reader      = NULL;
   Level       = level;
   if(name != NULL) openMedia(name);
}


// ###### Destructor ########################################################
MultiAudioReader::~MultiAudioReader()
{
   closeMedia();
}


// ###### Close file ########################################################
void MultiAudioReader::closeMedia()
{
   // Delete AudioReaders
   while(ReaderSet.begin() != ReaderSet.end()) {
      ReaderIterator               = ReaderSet.begin();
      AudioReaderInterface* reader = ReaderIterator->second.Reader;
      reader->closeMedia();
      delete reader;
      ReaderSet.erase(ReaderIterator);
   }

   Reader      = NULL;
   Error       = ME_NoMedia;
   Position    = 0;
   MaxPosition = 0;
}


// ###### Open new file #####################################################
bool MultiAudioReader::openMedia(const char* name)
{
   // ====== Open file ======================================================
   closeMedia();
   Error = ME_BadMedia;
   FILE* inputFD = fopen(name,"r");
   if(inputFD == NULL) {
      std::cerr << "WARNING: Unable to open input file <" << name << ">!" << std::endl;
      return(false);
   }

   // ====== Read identification ============================================
   char str[256];
   char* result = fgets((char*)&str,256,inputFD);
   if(result == NULL) {
      return(false);
   }
   if(strncmp((char*)&str,"AudioList",9)) {
      return(false);
   }

   // ====== Read file names and initialize AudioReaders ====================
   bool   overwriteSettings = false;
   String title("");
   String artist("");
   String comment("");
   String dir("");

   result = fgets((char*)&str,256,inputFD);
   while(!feof(inputFD)) {
      const cardinal inputLength = strlen((char*)&str);
      if(inputLength > 1) {
         str[inputLength - 1] = 0x00;
         switch(str[0]) {
            // ====== Line is a comment =====================================
            case '#':
             break;

            // ====== Line is an option =====================================
            case '*': {
               const String input((char*)&str[1]);
               String name;
               String value;
               if(input.scanSetting(name,value)) {
                  if(name == "DIRECTORY") {
                     dir = value;
                  }
                  else if(name == "TITLE") {
                     title = value;
                     overwriteSettings = true;
                  }
                  else if((name == "ARTIST") || (name == "AUTHOR")) {
                     artist = value;
                     overwriteSettings = true;
                  }
                  else if(name == "COMMENT") {
                     comment = value;
                     overwriteSettings = true;
                  }
                  else {
                     std::cerr << "WARNING: MultiAudioReader::openMedia() - Unknown option <"
                               << name << " = " << value << ">!" << std::endl;
                  }
               }
             }
             break;

            // ====== Line is a file name ===================================
            default: {
               const String input = String((char*)&str).stripWhiteSpace();
               String name("");
               if(input[0] != '/') {
                  name = dir;
                  const cardinal l = name.length();
                  if((l > 0) && (name[l - 1] != '/')) {
                     name = name + "/";
                  }
               }
               name = name + input;
               AudioReaderInterface* reader = getAudioReader(name.getData(),Level);
               if(reader != NULL) {
                  ReaderEntry readerEntry;
                  readerEntry.Reader            = reader;
                  readerEntry.OverwriteSettings = overwriteSettings;
                  if(overwriteSettings) {
                     readerEntry.Title   = title;
                     readerEntry.Artist  = artist;
                     readerEntry.Comment = comment;
                  }
                  ReaderSet.insert(std::pair<const card64, ReaderEntry>
                                      (MaxPosition,readerEntry));
                  MaxPosition += reader->getMaxPosition();
               }
               else {
                  std::cerr << "WARNING: MultiAudioReader::openMedia() - Unable to load <"
                            << name << ">" << std::endl;
               }
               title   = "";
               artist  = "";
               comment = "";
               overwriteSettings = false;
            }
            break;
         }
      }
      result = fgets((char*)&str,256,inputFD);
   }
   fclose(inputFD);


   // ###### Initialize AudioReader #########################################
   if(ReaderSet.size() > 0) {
      std::multimap<const card64, ReaderEntry>::iterator firstReader =
         ReaderSet.begin();
      Reader = firstReader->second.Reader;

      setQuality(*Reader);

      Error = ME_NoError;
      return(true);
   }
   return(false);
}


// ###### Get MultiAudioReader status #######################################
bool MultiAudioReader::ready() const
{
   return((Error == ME_NoError));
}


// ###### Get maximum position ##############################################
card64 MultiAudioReader::getMaxPosition() const
{
   return(MaxPosition);
}


// ###### Get media info ###################################################
void MultiAudioReader::getMediaInfo(MediaInfo& mediaInfo) const
{
   if(Reader != NULL) {
      mediaInfo.StartTimeStamp = ReaderIterator->first;
      mediaInfo.EndTimeStamp   = mediaInfo.StartTimeStamp + Reader->getMaxPosition();
      if(!ReaderIterator->second.OverwriteSettings) {
         Reader->getMediaInfo(mediaInfo);
      }
      else {
         strcpy((char*)&mediaInfo.Title,ReaderIterator->second.Title.getData());
         strcpy((char*)&mediaInfo.Artist,ReaderIterator->second.Artist.getData());
         strcpy((char*)&mediaInfo.Comment,ReaderIterator->second.Comment.getData());
      }
   }
   else {
      mediaInfo.reset();
   }
}


// ###### Get error code ###################################################
MediaError MultiAudioReader::getErrorCode() const
{
   return(Error);
}


// ###### Get position ######################################################
card64 MultiAudioReader::getPosition() const
{
   if(Reader != NULL)
      return(Position + Reader->getPosition());
   return(0);
}


// ###### Set position ######################################################
void MultiAudioReader::setPosition(const card64 position)
{
   if((Reader != NULL) && (Error < ME_UnrecoverableError)) {
      Position = MaxPosition;

      ReaderIterator = ReaderSet.begin();
      bool ok = false;
      while(ReaderIterator != ReaderSet.end()) {
         Position = ReaderIterator->first;
         Reader   = ReaderIterator->second.Reader;

         // Search for AudioReader containing the given position
         if((position >= Position) && (position < Position + Reader->getMaxPosition())) {
            Reader->setPosition(position - Position);
            ok = true;
            break;
         }
         ReaderIterator++;
      }
      if(ok == false) {
         ReaderIterator = ReaderSet.end();
         ReaderIterator--;
         Position = ReaderIterator->first;
         Reader   = ReaderIterator->second.Reader;
         Reader->setPosition(Reader->getMaxPosition());
      }

      setQuality(*Reader);
   }
}


// ###### Read block from file ##############################################
cardinal MultiAudioReader::getNextBlock(void* buffer, const cardinal blockSize)
{
   if((Reader != NULL) && (Error < ME_UnrecoverableError)) {
      cardinal result = Reader->getNextBlock(buffer,blockSize);

      // ====== Move to next AudioReader ====================================
      if(result < blockSize) {
         ReaderIterator++;
         if(ReaderIterator != ReaderSet.end()) {
            Position = ReaderIterator->first;
            Reader   = ReaderIterator->second.Reader;

            // Start playing from position 0 of the new AudioReader
            Reader->setPosition(0);
            setQuality(*Reader);

            result = Reader->getNextBlock(buffer,blockSize);
         }
         else {
            // Restore old (=last) AudioReader
            ReaderIterator--;
         }
      }

      Error = Reader->getErrorCode();
      return(result);
   }
   return(0);
}


// ###### Get AudioReader for given file ####################################
AudioReaderInterface* MultiAudioReader::getAudioReader(const char*    name,
                                                       const cardinal level)
{
   // ====== Try to load file with WAV reader ===============================
   WavAudioReader* wavreader = new WavAudioReader(name);
   if(wavreader->ready()) {
      return(wavreader);
   }
   delete wavreader;


   // ====== Try to load file with MP3 reader ===============================
   MP3AudioReader* mp3reader = new MP3AudioReader(name);
   if(mp3reader->ready()) {
      return(mp3reader);
   }
   delete mp3reader;


   // ====== Try to load file with Multi reader =============================
   if(level < 4) {
      MultiAudioReader* multireader = new MultiAudioReader(name,level + 1);
      if(multireader->ready()) {
         return(multireader);
      }
      delete multireader;
   }
   else {
      std::cerr << "WARNING: MultiAudioReader::getAudioReader() - Recursion level too high!"
                << std::endl;
   }

   return(NULL);
}
