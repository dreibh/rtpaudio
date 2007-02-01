// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Media Info Implementation                                        ####
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
#include "mediainfo.h"
#include "tools.h"


// ###### Constructor #######################################################
MediaInfo::MediaInfo()
{
}


// ###### Reset media info ##################################################
void MediaInfo::reset()
{
   StartTimeStamp = 0;
   EndTimeStamp   = 0;
   Title[0]       = 0x00;
   Artist[0]      = 0x00;
   Comment[0]     = 0x00;
}


// ###### Translate byte order ##############################################
void MediaInfo::translate() {
   StartTimeStamp = translate64(StartTimeStamp);
   EndTimeStamp   = translate64(EndTimeStamp);
}


// ###### Output operator ###################################################
ostream& operator<<(ostream& os, const MediaInfo& mi)
{
   cout << "StartTimeStamp = " << mi.StartTimeStamp << endl;
   cout << "EndTimeStamp   = " << mi.EndTimeStamp   << endl;
   cout << "Title          = \"" << mi.Title << "\"" << endl;
   cout << "Artist         = \"" << mi.Artist << "\"" << endl;
   cout << "Comment        = \"" << mi.Comment << "\"" << endl;
   return(os);
}
