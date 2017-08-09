// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Bandwidth Info Implementation                                    ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   https://www.uni-due.de/~be0001/rtpaudio                ####
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


#include "tdsystem.h"
#include "bandwidthinfo.h"


// ###### Reset #############################################################
void BandwidthInfo::reset()
{
   BufferDelay      = 1;
   BytesPerSecond   = 0;
   PacketsPerSecond = 0;
   MaxTransferDelay = HUGE_VAL;
   MaxLossRate      = 1.0;
   MaxJitter        = HUGE_VAL;
}


// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, const BandwidthInfo& bi)
{
   os << "   BytesPerSecond   = " << bi.BytesPerSecond   << std::endl;
   os << "   PacketsPerSecond = " << bi.PacketsPerSecond << std::endl;
   os << "   BufferDelay      = " << bi.BufferDelay      << std::endl;
   os << "   MaxTransferDelay = " << bi.MaxTransferDelay / 1000.0 << " [ms]" << std::endl;
   os << "   MaxLossRate      = " << bi.MaxLossRate      << std::endl;
   os << "   MaxJitter        = " << bi.MaxJitter        << std::endl;
   return(os);
}
