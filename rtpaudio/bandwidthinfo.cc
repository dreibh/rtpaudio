// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Bandwidth Info Implementation                                    ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
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
