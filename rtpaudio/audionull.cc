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


#include "tdsystem.h"
#include "audionull.h"
#include "tools.h"

#include <iostream>
#include <sys/time.h>


namespace Coral {


// ###### Constructor #######################################################
AudioNull::AudioNull()
   : AudioQuality(44100,16,2)
{
}


// ###### Destructor ########################################################
AudioNull::~AudioNull()
{
}


// ###### Check, if spectrum analyzer is ready ##############################
bool AudioNull::ready() const
{
   return(true);
}


// ###### Reset #############################################################
void AudioNull::sync()
{
}


// ###### Write data to spectrum analyzer ###################################
bool AudioNull::write(const void* data, const size_t length)
{
   return(true);
}


}
