// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client App Packet Implementation                           ####
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
#include "audioclientapppacket.h"
#include "tools.h"


namespace Coral {


// ###### Constructor #######################################################
AudioClientAppPacket::AudioClientAppPacket()
{
}


// ###### Reset #############################################################
void AudioClientAppPacket::reset()
{
   FormatID        = AudioClientFormatID;
   PosChgSeqNumber = 0;
   SequenceNumber  = 0;
   Status          = ACAS_UnknownCommand;
   SamplingRate    = 0;
   Channels        = 0;
   Bits            = 0;
   Encoding        = 0;
   StartPosition   = (card64)-1;
   RestartPosition = 0;
   BandwidthLimit  = (card32)-1;
   for(cardinal i = 0;i < 128;i++) {
      MediaName[i] = 0x00;
   }
}


// ###### Translate byte order ##############################################
void AudioClientAppPacket::translate()
{
   FormatID        = translate32(FormatID);
   SequenceNumber  = translate16(SequenceNumber);
   PosChgSeqNumber = translate16(PosChgSeqNumber);
   Status          = translate16(Status);
   SamplingRate    = translate16(SamplingRate);
   Encoding        = translate16(Encoding);
   StartPosition   = translate64(StartPosition);
   RestartPosition = translate64(RestartPosition);
   BandwidthLimit  = translate32(BandwidthLimit);
}


}
