// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Writer Interface                                           ####
// ####                                                                  ####
// #### Version 1.50  --  August 01, 2001                                ####
// ####                                                                  ####
// ####            Copyright (C) 1999-2001 by Thomas Dreibholz           ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@exp-math.uni-essen.de                           ####
// ####    WWW:   http://www.exp-math.uni-essen.de/~dreibh/rtpaudio      ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software; you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License      ####
// #### as published by the Free Software Foundation; either version 2   ####
// #### of the License, or (at your option) any later version.           ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// ##########################################################################


#ifndef AUDIOWRITERINTERFACE_H
#define AUDIOWRITERINTERFACE_H


#include "tdsystem.h"
#include "audioqualityinterface.h"


/**
  * This class is the interface for an audio writer.
  *
  * @short   Audio Writer Interface
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class AudioWriterInterface : virtual public AdjustableAudioQualityInterface
{
   public:
   /**
     * Virtual destructor.
     */
   virtual ~AudioWriterInterface();

   /**
     * Check, if AudioWriter is ready for writing.
     *
     * @return true, if AudioWriter is ready; false otherwise.
     */
   virtual bool ready() const = 0;

   /**
     * Reset the writer. All data in the output buffer should be removed
     * without writing.
     * Usage example: AudioDevice sends ioctl SNDCTL_DSP_SYNC.
     */
   virtual void sync() = 0;

   /**
     * Write data.
     *
     * @param data Data to be written.
     * @return length Length of data in bytes.
     */
   virtual bool write(const void* data, const size_t length) = 0;

   /**
     * Get current capacity of audio output buffer.
     *
     * @return Capacity in bytes.
     */
   virtual cardinal getCurrentCapacity();
};


#endif
