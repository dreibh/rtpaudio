// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Decoder Interface                                          ####
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


#ifndef AUDIODECODERINTERFACE_H
#define AUDIODECODERINTERFACE_H


#include "tdsystem.h"
#include "decoderinterface.h"
#include "audioquality.h"


/**
  * This class is the interface for an audio decoder.
  *
  * @short   Audio Decoder Interface
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class AudioDecoderInterface : virtual public DecoderInterface,
                              virtual public AudioQualityInterface
{
   // ====== Wanted quality =================================================
   public:
   /**
     * Get wanted quality. This is the quality wanted in TransportInfo
     * from getTransportInfo().
     *
     * @return Wanted quality.
     *
     * @see DecoderInterface#getTransportInfo
     */
   virtual AudioQuality getWantedQuality() const = 0;

   /**
     * Set wanted quality. This is the quality wanted in TransportInfo
     * from getTransportInfo(). Note: This does *not* tell the sender to
     * modify the quality! This function only sets the wanted quality which
     * is reported by getTransportInfo().
     *
     * @return Wanted quality.
     *
     * @see DecoderInterface#getTransportInfo
     */
   virtual void setWantedQuality(const AudioQualityInterface& wantedQuality) = 0;
};


#endif
