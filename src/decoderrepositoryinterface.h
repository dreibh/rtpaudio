// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Decoder Repository Interface                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2023 by Thomas Dreibholz            ####
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


#ifndef DECODERREPOSITORYINTERFACE_H
#define DECODERREPOSITORYINTERFACE_H


#include "tdsystem.h"
#include "synchronizable.h"
#include "decoderinterface.h"


/**
  * This class is a repository for decoders.
  *
  * @short   Decoder Repository
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
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
