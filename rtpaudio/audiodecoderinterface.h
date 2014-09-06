// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Decoder Interface                                          ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
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


#ifndef AUDIODECODERINTERFACE_H
#define AUDIODECODERINTERFACE_H


#include "tdsystem.h"
#include "decoderinterface.h"
#include "audioquality.h"


/**
  * This class is the interface for an audio decoder.
  *
  * @short   Audio Decoder Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioDecoderInterface : virtual public DecoderInterface,
                              virtual public AudioQualityInterface
{
   public:
   /**
     * Destructor.
     *
     */
   virtual ~AudioDecoderInterface() = 0;   

   // ====== Wanted quality =================================================
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
