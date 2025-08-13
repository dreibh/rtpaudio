// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### WAV Audio Reader                                                 ####
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


#ifndef WAVAUDIOREADER_H
#define WAVAUDIOREADER_H


#include "tdsystem.h"
#include "audioreaderinterface.h"
#include "audioquality.h"


/**
  * This class is a reader for WAV audio files.
  *
  * @short   WAV Audio Reader
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
class WavAudioReader : virtual public AudioReaderInterface,
                       public AudioQuality
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param name Name of WAV file or NULL.
     */
   WavAudioReader(const char* name = NULL);

   /**
     * Destructor.
     */
   ~WavAudioReader();

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
   void setPosition(const card64 position);

   /**
     * getNextBlock() implementation of AudioReaderInterface.
     *
     * @see AudioReaderInterface#getNextBlock
     */
   cardinal getNextBlock(void* buffer, const cardinal blockSize);


   // ====== Private data ===================================================
   private:
   struct RIFF_Header {
      char   RIFF[4];
      card32 Length;
      char   FormatID[4];
   };
   struct RIFF_Chunk {
      char   ID[4];
      card32 Length;
   };
   struct WAVE_Format {
      card16 FormatTag;
      card16 Channels;
      card32 SamplesPerSec;
      card32 AvgBytesPerSec;
      card16 BlockAlign;
   };


   bool getChunk(RIFF_Chunk& chunk);


   MediaError  Error;
   FILE*       InputFD;
   WAVE_Format Format;
   card64      StartPosition;
   card64      EndPosition;
   card64      Position;
   card64      MaxPosition;
};


#endif
