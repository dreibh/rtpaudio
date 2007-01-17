// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Packet Implementation                                        ####
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
#include "rtppacket.h"


namespace Coral {


// ###### Constructor #######################################################
RTPPacket::RTPPacket()
{
  V    = RTPConstants::RTPVersion;
  P    = false;
  X    = false;
  CC   = 0;
  M    = false;
  PT   = 0;
  setSequenceNumber(0);
  setTimeStamp(0);
  setSSRC(0);
}


// ###### Output operator ###################################################
ostream& operator<<(ostream& os, const RTPPacket& packet)
{
  cout << "RTPPacket:: {\n";
  cout << "\tVersion      : " << packet.getVersion()        << "\n";
  cout << "\tPadding      : " << packet.getPadding()        << "\n";
  cout << "\tExtension    : " << packet.getExtension()      << "\n";
  cout << "\tCSRC Count   : " << packet.getCSRCCount()      << "\n";
  cout << "\tMarker       : " << packet.getMarker()         << "\n";
  cout << "\tPayload Type : " << packet.getPayloadType()    << "\n";
  cout << "\tSequence Num : " << packet.getSequenceNumber() << "\n";
  cout << "\tTimestamp    : " << packet.getTimeStamp()      << "\n";
  cout << "\tSSRC         : " << packet.getSSRC()           << "\n";
  cout << "\tCSRC         : ";
  for(card8 i = 0;i < packet.getCSRCCount();i++) {
     cout << packet.getCSRC(i) << "  ";
  }
  cout << "}" << endl;
  return(os);
}


}
