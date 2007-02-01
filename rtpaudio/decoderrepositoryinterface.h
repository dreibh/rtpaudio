// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Decoder Repository Interface                                     ####
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


#ifndef DECODERREPOSITORYINTERFACE_H
#define DECODERREPOSITORYINTERFACE_H


#include "tdsystem.h"
#include "synchronizable.h"
#include "decoderinterface.h"


/**
  * This class is a repository for decoders.
  *
  * @short   Decoder Repository
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class DecoderRepositoryInterface : virtual public DecoderInterface
{
   // ====== Select another decoder =========================================
   public:
   /**
     * Select the decoder with the given TypeID to be the current decoder
     * of the repository.
     *
     * @param typeID Decoding's type ID.
     * @return true, if decoder for this TypeID was in the repository;
     * false otherwise.
     */
   virtual bool selectDecoderForTypeID(const card16 typeID) = 0;

   /**
     * Get DecoderInterface of the current decoder.
     *
     * @return Current decoder's DecoderInterface.
     */
   virtual DecoderInterface* getCurrentDecoder() const = 0;
};


#endif
