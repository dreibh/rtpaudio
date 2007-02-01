// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Source State Info Implementation                                 ####
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


#include "sourcestateinfo.h"


#define RTP_SEQ_MOD        (1 << 16)
#define RTP_MAX_DROPOUT    3000
#define RTP_MAX_MISORDER   100
#define RTP_MIN_SEQUENTIAL 2


// ###### Constructor #######################################################
SourceStateInfo::SourceStateInfo()
   : SeqNumValidator(RTP_MIN_SEQUENTIAL,RTP_MAX_MISORDER,RTP_MAX_DROPOUT,RTP_SEQ_MOD),
     Synchronizable("SourceStateInfo")
{
   reset();
}


// ###### Initialize table ##################################################
void SourceStateInfo::reset()
{
   LSR                = 0;
   LSRUpdateTimeStamp = 0;
   SSRC               = 0;
   SeqNumValidator::reset();
}


// ###### Calculate DLSR ####################################################
card32 SourceStateInfo::calculateDLSR() const
{
   if(LSRUpdateTimeStamp == 0) {
      return(0);
   }
   else {
      const double delay = (double)getMicroTime() - (double)LSRUpdateTimeStamp;
      const card32 dlsr = (card32)(delay * (65536.0 / 1000000.0));
      return(dlsr);
   }
}
