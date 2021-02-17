// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Multi Audio Reader                                               ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2021 by Thomas Dreibholz            ####
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


#ifndef WAVREADER_H
#define WAVREADER_H


#include "tdsystem.h"
#include "audioreaderinterface.h"
#include "audioquality.h"
#include "string.h"

#include <map>


/**
  * This class is a reader for multiple audio files from a list.
  *
  * @short   Multi Audio Reader
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class MultiAudioReader : virtual public AudioReaderInterface,
                         public AudioQuality
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param name Name of AudioList file or NULL.
     * @param level Recursion level (normally 0).
     */
   MultiAudioReader(const char* name = NULL, const cardinal level = 0);

   /**
     * Destructor.
     */
   ~MultiAudioReader();

   // ====== Initialize =====================================================
   /**
     * openMedia() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#openMedia
     */
   bool openMedia(const char* name);

   /**
     * closeMedia() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#closeMedia
     */
   void closeMedia();

   /**
     * ready() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#ready
     */
   bool ready() const;


   // ====== Input functions ================================================
   /**
     * getMediaInfo() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#getMediaInfo
     */
   void getMediaInfo(MediaInfo& mediaInfo) const;

   /**
     * getErrorCode() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#getErrorCode
     */
   MediaError getErrorCode() const;

   /**
     * getPosition() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#getPosition
     */
   card64 getPosition() const;

   /**
     * getMaxPosition() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#getMaxPosition
     */
   card64 getMaxPosition() const;

   /**
     * setPosition() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#setPosition
     */
   void setPosition(card64 position);

   /**
     * getNextBlock() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#getNextBlock
     */
   cardinal getNextBlock(void* buffer, const cardinal blockSize);


   // ====== Static functions ===============================================
   /**
     * Get AudioReaderInterface for loading a given file.
     *
     * @param name File name.
     * @param level Recursion level (normally 0).
     * @return AudioReaderInterface, if load was successfull; NULL otherwise.
     */
   AudioReaderInterface* getAudioReader(const char* name, const cardinal level);


   // ====== Private data ===================================================
   private:
   struct ReaderEntry {
      AudioReaderInterface* Reader;
      bool                  OverwriteSettings;
      String                Title;
      String                Artist;
      String                Comment;
   };


   AudioReaderInterface*                              Reader;
   std::multimap<const card64, ReaderEntry>           ReaderSet;
   std::multimap<const card64, ReaderEntry>::iterator ReaderIterator;

   MediaError Error;
   card64     Position;
   card64     MaxPosition;
   cardinal   Level;
};


#endif
