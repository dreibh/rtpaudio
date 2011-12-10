// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client App Packet Implementation                           ####
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
#include "audioclientapppacket.h"
#include "tools.h"


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
