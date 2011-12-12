// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### MP3 Audio Reader                                                 ####
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


#ifndef MP3AUDIOREADER_H
#define MP3AUDIOREADER_H


#include "tdsystem.h"
#include "audioreaderinterface.h"
#include "audioquality.h"


// IMPORTANT: PTHREADEDMPEG *must* be defined, if libmpegsound.a is
//            compiled with PTHREADEDMPEG set!!! Otherwise initialization
//            of Mpegtoraw object will fail with segmentation fault!
#define PTHREADEDMPEG 1
#define HAVE_PTHREAD_H
#include "mpegsound.h"


/**
  * This class is a reader for MP3 audio files.
  *
  * @short   MP3 Audio Reader
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class MP3AudioReader : public Soundplayer,
                       public AudioReaderInterface,
                       public AudioQuality
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     *
     * @param name Name of MP3 file or NULL.
     */
   MP3AudioReader(const char* name = NULL);

   /**
     * Destructor.
     */
   ~MP3AudioReader();

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


   // ====== Soundplayer implementation =====================================
   private:
   bool initialize(char* filename);
   bool setsoundtype(int stereo, int samplesize, int speed);
   void set8bitmode();
   bool putblock(void* buffer, int size);
   int  putblock_nt(void* buffer, int size);
   void releasedevice();
   bool attachdevice();


   // ====== Private data ===================================================
   private:
   bool readNextFrame();

   Mpegtoraw*                MP3Decoder;
   Soundinputstreamfromfile* MP3Source;

   cardinal                  BufferPos;
   cardinal                  BufferSize;
   double                    FramesPerSecond;
   card64                    Position;
   card64                    MaxPosition;

   MediaError                Error;

   char                      Buffer[RAWDATASIZE * sizeof(short int)];
};


#endif
