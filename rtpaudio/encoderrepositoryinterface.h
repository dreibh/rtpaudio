// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Encoder Repository Interface                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2014 by Thomas Dreibholz            ####
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


#ifndef ENCODERREPOSITORYINTERFACE_H
#define ENCODERREPOSITORYINTERFACE_H


#include "tdsystem.h"
#include "synchronizable.h"
#include "encoderinterface.h"


/**
  * This class is a repository for encoders.
  *
  * @short   Encoder Repository Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class EncoderRepositoryInterface : virtual public EncoderInterface
{
   // ====== Select another encoder =========================================
   public:
   /**
     * Select the encoder with the given TypeID to be the current encoder
     * of the repository.
     *
     * @param typeID Encoding's type ID.
     * @return true, if encoder for this TypeID was in the repository;
     * false otherwise.
     */
   virtual bool selectEncoderForTypeID(const card16 typeID) = 0;

   /**
     * Get EncoderInterface of the current encoder.
     *
     * @return Current encoder's EncoderInterface.
     */
   virtual EncoderInterface* getCurrentEncoder() const = 0;
};


#endif
