// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Packet Implementation                                        ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2021 by Thomas Dreibholz            ####
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
#include "rtppacket.h"


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
std::ostream& operator<<(std::ostream& os, const RTPPacket& packet)
{
  std::cout << "RTPPacket:: {\n";
  std::cout << "\tVersion      : " << packet.getVersion()        << "\n";
  std::cout << "\tPadding      : " << packet.getPadding()        << "\n";
  std::cout << "\tExtension    : " << packet.getExtension()      << "\n";
  std::cout << "\tCSRC Count   : " << packet.getCSRCCount()      << "\n";
  std::cout << "\tMarker       : " << packet.getMarker()         << "\n";
  std::cout << "\tPayload Type : " << packet.getPayloadType()    << "\n";
  std::cout << "\tSequence Num : " << packet.getSequenceNumber() << "\n";
  std::cout << "\tTimestamp    : " << packet.getTimeStamp()      << "\n";
  std::cout << "\tSSRC         : " << packet.getSSRC()           << "\n";
  std::cout << "\tCSRC         : ";
  for(card8 i = 0;i < packet.getCSRCCount();i++) {
     std::cout << packet.getCSRC(i) << "  ";
  }
  std::cout << "}" << std::endl;
  return(os);
}
