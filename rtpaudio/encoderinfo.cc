// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Encoder Info                                                     ####
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
#include "tools.h"
#include "rtppacket.h"
#include "rtcppacket.h"
#include "audioencoderinterface.h"
#include "advancedaudiopacket.h"
#include "simpleaudiopacket.h"
#include "audioquality.h"
#include "transportinfo.h"
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>


using namespace Coral;


// ###### Print information on given quality #################################
void printQualityInfoAAE(const card16   samplingRate,
                         const card8    bits,
                         const card8    channels,
                         const cardinal headerSize,
                         const cardinal maxPacketSize)
{
   AudioQuality quality(samplingRate,bits,channels);
   TransportInfo ti;
   AdvancedAudioPacket pkt;
   pkt.getTransportInfo(ti,headerSize,maxPacketSize,quality,quality);
   ti.TotalBytesPerSecondLimit   = (card64)-1;
   ti.TotalPacketsPerSecondLimit = (card32)-1;
   ti.TotalFramesPerSecondLimit  = (card32)-1;    
   cout << quality << ":" << endl;
   cout << ti.CurrentSetting;
}


// ###### Print information on given quality #################################
void printQualityInfoSAE(const card16   samplingRate,
                         const card8    bits,
                         const card8    channels,
                         const cardinal headerSize,
                         const cardinal maxPacketSize)
{
   AudioQuality quality(samplingRate,bits,channels);
   TransportInfo ti;
   SimpleAudioPacket pkt;
   pkt.getTransportInfo(ti,headerSize,maxPacketSize,quality,quality);
   ti.TotalBytesPerSecondLimit   = (card64)-1;
   ti.TotalPacketsPerSecondLimit = (card32)-1;
   ti.TotalFramesPerSecondLimit  = (card32)-1;    
   cout << ti.CurrentSetting;
}


// ###### Quality information printing ######################################
void printQualities(const cardinal maxPacketSize)
{
   const cardinal headerSize    = IPv6HeaderSize + UDPHeaderSize + RTPConstants::RTPDefaultHeaderSize;

   cout << "IPv4 Header             = " << sizeof(iphdr) << endl;
   cout << "IPv6 Header             = " << sizeof(ip6_hdr) << endl;
   cout << "UDP Header              = " << sizeof(udphdr) << endl;
   cout << "RTPDefaultHeaderSize    = " << RTPConstants::RTPDefaultHeaderSize << endl;
   cout << "MaxPacketSize           = " << maxPacketSize << endl;
   cout << "SimpleAudioHeaderSize   = " << sizeof(SimpleAudioPacket) << endl;
   cout << "AdvancedAudioHeaderSize = " << sizeof(AdvancedAudioPacket) << endl;
   cout << "RTCPReceiverReport      = " << sizeof(RTCPReceiverReport) << endl;
   cout << "RTCPbye                 = " << sizeof(RTCPBye) << endl;
   cout << endl << "Simple Audio Encoding:" << endl << endl;
   for(cardinal i = 0;i < AudioQuality::ValidRates;i++) {
      const cardinal rate = AudioQuality::ValidRatesTable[i];
      for(cardinal j = 0;j < AudioQuality::ValidBits;j++) {
         cardinal bits = AudioQuality::ValidBitsTable[j];
         printQualityInfoSAE(rate,bits,1,headerSize,maxPacketSize);
         printQualityInfoSAE(rate,bits,2,headerSize,maxPacketSize);
      }
   }

   cout << endl << endl << "Advanced Audio Encoding:" << endl << endl;
   for(cardinal i = 0;i < AudioQuality::ValidRates;i++) {
      const cardinal rate = AudioQuality::ValidRatesTable[i];
      for(cardinal j = 0;j < AudioQuality::ValidBits;j++) {
         cardinal bits = AudioQuality::ValidBitsTable[j];
         printQualityInfoAAE(rate,bits,1,headerSize,maxPacketSize);
         printQualityInfoAAE(rate,bits,2,headerSize,maxPacketSize);
      }
   }
}


// ###### Print TransportInfos ##############################################
void printTransportInfos(const cardinal maxPacketSize)
{
   const cardinal headerSize = IPv6HeaderSize + UDPHeaderSize +
                               RTPConstants::RTPDefaultHeaderSize;

   AudioQuality quality(44100,16,2);
   TransportInfo       tia,tib;
   AdvancedAudioPacket a;
   SimpleAudioPacket   b;
   b.getTransportInfo(tia,headerSize,maxPacketSize,quality,quality);
   tia.TotalBytesPerSecondLimit   = (card64)-1;
   tia.TotalPacketsPerSecondLimit = (card32)-1;
   tia.TotalFramesPerSecondLimit  = (card32)-1;    
   a.getTransportInfo(tib,headerSize,maxPacketSize,quality,quality);
   tib.TotalBytesPerSecondLimit   = (card64)-1;
   tib.TotalPacketsPerSecondLimit = (card32)-1;
   tib.TotalFramesPerSecondLimit  = (card32)-1;    
   cout << "Simple Audio Encoding:" << endl << endl << tia << endl << endl;
   cout << "Advanced Audio Encoding:" << endl << endl << tib << endl;
}


// ###### Print quality levels ##############################################
void printQualityLevels()
{
   AudioQuality q = AudioQuality::LowestQuality;
   for(int i = 0;i < 256;i++) {
      cout << "#" << i << ":   " << q << "   " << q.getBytesPerSecond() << " Bytes/s" << endl;
      if(q == AudioQuality::HighestQuality)
         break;
      q++;
   }
}



// ###### Main program ######################################################
int main(int argc, char** argv)
{
   // ====== Get arguments ==================================================
   cardinal maxPacketSize = 1500;
   if(argc < 2) {
      cerr << "Usage: " << argv[0] << " [-ti|-quality|-levels] {-maxpktsize=bytes}" << endl;
      exit(1);
   }
   if(argc > 2) {
      if(!(strncasecmp(argv[2],"-maxpktsize=",12))) {
         maxPacketSize = (cardinal)atol(&argv[2][12]);
      }
      else {
         cerr << "ERROR: Unknown option!" << endl;
         exit(1);
      }
   }
   if(maxPacketSize < 128) {
      maxPacketSize = 128;
   }
   else if(maxPacketSize > 1024 * 1024) {
      maxPacketSize = 1024 * 1024;
   }


   // ====== Print information ==============================================
   cout << "RTP Audio Encoder Info - Copyright (C) 1999-2001 Thomas Dreibholz" << endl;
   cout << "-----------------------------------------------------------------" << endl;
   cout << endl;
   cout << "Max Packet Size: " << maxPacketSize << endl;
   cout << endl;


   // ====== Print encoder information ======================================
   if(!(strcasecmp(argv[1],"-quality")))
      printQualities(maxPacketSize);
   else if(!(strcasecmp(argv[1],"-ti")))
      printTransportInfos(maxPacketSize);
   else if(!(strcasecmp(argv[1],"-levels")))
      printQualityLevels();
   else {
      cerr << "Usage:" << argv[0] << " [-ti|-quality|-levels]" << endl;
      exit(1);
   }

   return(0);
}
