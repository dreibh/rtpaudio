// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Null                                                       ####
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


#ifndef AUDIONULL_H
#define AUDIONULL_H


#include "tdsystem.h"
#include "audiowriterinterface.h"
#include "audioquality.h"


/**
  * This class implements a dummy AudioWriterInterface.
  *
  * @short   Audio Null
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class AudioNull : virtual public AudioWriterInterface,
                  public AudioQuality
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   AudioNull();

   /**
     * Destructor.
     */
   ~AudioNull();


   // ====== AudioInterface implementation ==================================
   /**
     * ready() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#ready
     */
   bool ready() const;

   /**
     * sync() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#sync
     */
   void sync();

   /**
     * write() implementation of AudioWriterInterface
     *
     * @see AudioWriterInterface#write
     */
   bool write(const void* data, const size_t length);
};


#endif
