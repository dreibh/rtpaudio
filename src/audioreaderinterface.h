// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Reader Interface                                           ####
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


#ifndef AUDIOREADERINTERFACE_H
#define AUDIOREADERINTERFACE_H


#include "tdsystem.h"
#include "audioqualityinterface.h"
#include "mediainfo.h"


/**
  * This class is the interface for an audio reader.
  *
  * @short   Audio Reader Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioReaderInterface : virtual public AudioQualityInterface
{
   public:
   /**
     * Destructor.
     *
     */
   virtual ~AudioReaderInterface();   
   
   /**
     * Open media.
     *
     * @param name Name of media, e.g. a file name.
     * @return true, if AudioReader is ready for reading; false otherwise.
     */
   virtual bool openMedia(const char* name) = 0;

   /**
     * Close media, if opened.
     */
   virtual void closeMedia() = 0;

   /**
     * Check, if AudioReader is ready for reading.
     *
     * @return true, if AudioReader is ready; false otherwise.
     */
   virtual bool ready() const = 0;

   /**
     * Get MediaInfo.
     *
     * @param mediaInfo Reference to store media info.
     */
   virtual void getMediaInfo(MediaInfo& mediaInfo) const = 0;

   /**
     * Get error code.
     *
     * @return Error code.
     */
   virtual MediaError getErrorCode() const = 0;

   /**
     * Get current position.
     *
     * @return Position in nanoseconds.
     */
   virtual card64 getPosition() const = 0;

   /**
     * Get maximum position.
     *
     * @return maximum position in nanoseconds.
     */
   virtual card64 getMaxPosition() const = 0;

   /**
     * Get position.
     *
     * @param position Position in nanoseconds.
     */
   virtual void setPosition(const card64 position) = 0;

   /**
     * Read next block.
     * In case of an error, getNextBlock() should return 0 and set ready to false.
     *
     * @param buffer Buffer for block to read.
     * @param blockSize Size of block in bytes.
     * @return Number of bytes read.
     */
   virtual cardinal getNextBlock(void* buffer, const cardinal blockSize) = 0;
};


#endif
