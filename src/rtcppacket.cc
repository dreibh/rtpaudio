// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Packet Implementation                                       ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2025 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "rtcppacket.h"


// ##########################################################################
// #### RTCP Common Header                                               ####
// ##########################################################################


// ###### Contructor ########################################################
RTCPCommonHeader::RTCPCommonHeader()
{ }


// ##########################################################################
// #### RTCP Report                                                      ####
// ##########################################################################


// ###### Contructor ########################################################
RTCPReport::RTCPReport()
{ }


// ##########################################################################
// #### RTCP Sender Report                                               ####
// ##########################################################################


// ###### Contructors #######################################################
RTCPSenderReport::RTCPSenderReport() { }
RTCPSenderReport::RTCPSenderReport(const card32 syncSource,
                                   const card8 count)
{
   init(syncSource,count);
}


// ###### Initialize ########################################################
void RTCPSenderReport::init(const card32 syncSource, const card8 count)
{
   V    = RTPConstants::RTPVersion;
   P    = false;
   C    = count;
   PT   = RTCP_SR;
   setLength(sizeof(*this));
   setSSRC(syncSource);
}


// ##########################################################################
// #### RTCP Receiver Report                                             ####
// ##########################################################################


// ###### Contructors #######################################################
RTCPReceiverReport::RTCPReceiverReport() { }
RTCPReceiverReport::RTCPReceiverReport(const card32 syncSource,
                                       const card8 count)
{
   init(syncSource,count);
}


// ###### Initialize ########################################################
void RTCPReceiverReport::init(const card32 syncSource, const card8 count)
{
  V    = RTPConstants::RTPVersion;
  P    = false;
  C    = count;
  PT   = RTCP_RR;
  setLength(sizeof(*this));
  setSSRC(syncSource);
}


// ##########################################################################
// #### RTCP Sender Info Block                                           ####
// ##########################################################################


// ###### Contructor ########################################################
RTCPSenderInfoBlock::RTCPSenderInfoBlock()
{ }


// ##########################################################################
// #### RTCP Reception Report Block                                      ####
// ##########################################################################


// ###### Contructors #######################################################
RTCPReceptionReportBlock::RTCPReceptionReportBlock() { }
RTCPReceptionReportBlock::RTCPReceptionReportBlock(const card32 ssrc)
{
   init(ssrc);
}


// ###### Initialize ########################################################
void RTCPReceptionReportBlock::init(const card32 ssrc)
{
   SSRC = translate32(ssrc);
}



// ##########################################################################
// #### RTCP Source Description                                          ####
// ##########################################################################


// ###### Contructors #######################################################
RTCPSourceDescription::RTCPSourceDescription() { }
RTCPSourceDescription::RTCPSourceDescription(const card8 count)
{
   init(count);
}


// ###### Initialize ########################################################
void RTCPSourceDescription::init(const card8 count)
{
  V  = RTPConstants::RTPVersion;
  P  = false;
  C  = count;
  PT = RTCP_SDES;
  setLength(sizeof(*this));
}


// ##########################################################################
// #### RTCP Bye                                                         ####
// ##########################################################################


// ###### Contructors #######################################################
RTCPBye::RTCPBye() { }
RTCPBye::RTCPBye(const card8 count)
{
   init(count);
}


// ###### Initialize ########################################################
void RTCPBye::init(const card8 count)
{
  V  = RTPConstants::RTPVersion;
  P  = false;
  C  = count;
  PT = RTCP_BYE;
  setLength(sizeof(*this));
}


// ##########################################################################
// #### RTCP App                                                         ####
// ##########################################################################


// ###### Contructors #######################################################
RTCPApp::RTCPApp() { }
RTCPApp::RTCPApp(const card8 subtype)
{
   init(subtype);
}


// ###### Initialize ########################################################
void RTCPApp::init(const card8 subtype)
{
  V  = RTPConstants::RTPVersion;
  P  = false;
  C  = subtype;
  PT = RTCP_APP;
  setLength(sizeof(*this));
}
