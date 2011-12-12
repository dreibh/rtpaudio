// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Traffic Class Values                                             ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de.de                               ####
// ####    WWW:   http://www.iem.uni-due.de.de/~dreibh/rn                ####
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


#include "tdsystem.h"
#include "trafficclassvalues.h"



// ###### Traffic class values ##############################################
const card8 TrafficClassValues::TCValues[TrafficClassValues::MaxValues] = {
   0xb8,
   0x28, 0x30, 0x38,
   0x48, 0x50, 0x58,
   0x68, 0x70, 0x78,
   0x88, 0x90, 0x98,
   0xa0, 0xa8,
   0x00
};


// ###### Traffic class names ###############################################
const char* TrafficClassValues::TCNames[TrafficClassValues::MaxValues] = {
   "EF",
   "AF11","AF12","AF13",
   "AF21","AF22","AF23",
   "AF31","AF32","AF33",
   "AF41","AF42","AF43",
   "TD1", "TD2",
   "BE"
};


// ###### Get index of traffic class ########################################
cardinal TrafficClassValues::getIndexForTrafficClass(const card8 trafficClass)
{
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      if(TCValues[i] == trafficClass) {
         return(i);
      }
   }
   return(MaxValues - 1);
}


// ###### Get name of traffic class #########################################
const char* TrafficClassValues::getNameForTrafficClass(const card8 trafficClass)
{
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      if(TCValues[i] == trafficClass) {
         return(TCNames[i]);
      }
   }
   return(NULL);
}


// ###### Get traffic class for name ########################################
const card16 TrafficClassValues::getTrafficClassForName(const char* name)
{
   for(cardinal i = 0;i < TrafficClassValues::MaxValues;i++) {
      if(!(strcasecmp(TCNames[i],name))) {
         return((card16)TCValues[i]);
      }
   }
   return(0xffff);
}
