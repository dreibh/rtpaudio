// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Encoder Info                                                     ####
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
#include "tools.h"
#include "rtppacket.h"
#include "rtcppacket.h"
#include "audioencoderinterface.h"
#include "advancedaudiopacket.h"
#include "simpleaudiopacket.h"
#include "audioquality.h"
#include "abstractqosdescription.h"

#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>


class SimpleAudioQoSDescription : public AbstractQoSDescription
{
   public:
   SimpleAudioQoSDescription();
   ~SimpleAudioQoSDescription();

   void updateDescription(const cardinal pktHeaderSize,
                          const cardinal pktMaxSize);
   cardinal getLayers() const;
   AbstractLayerDescription* getLayer(const cardinal layer) const;
};

SimpleAudioQoSDescription::SimpleAudioQoSDescription()
{
}

SimpleAudioQoSDescription::~SimpleAudioQoSDescription()
{
}

void SimpleAudioQoSDescription::updateDescription(
                                   const cardinal pktHeaderSize,
                                   const cardinal pktMaxSize)
{
}

cardinal SimpleAudioQoSDescription::getLayers() const
{
   return(1);
}

AbstractLayerDescription* SimpleAudioQoSDescription::getLayer(const cardinal layer) const
{

}



// // ###### Print information on given quality #################################
// void printQualityInfoAAE(const card16   samplingRate,
//                          const card8    bits,
//                          const card8    channels,
//                          const cardinal headerSize,
//                          const cardinal maxPacketSize)
// {
//    AudioQuality quality(samplingRate,bits,channels);
//    TransportInfo ti;
//    AdvancedAudioPacket pkt;
//    pkt.getTransportInfo(ti,headerSize,maxPacketSize,quality,quality);
//    ti.TotalBytesPerSecondLimit   = (card64)-1;
//    ti.TotalPacketsPerSecondLimit = (card32)-1;
//    ti.TotalFramesPerSecondLimit  = (card32)-1;
//    std::cout << quality << ":" << std::endl;
//    std::cout << ti.CurrentSetting;
// }
//
//
// // ###### Print information on given quality #################################
// void printQualityInfoSAE(const card16   samplingRate,
//                          const card8    bits,
//                          const card8    channels,
//                          const cardinal headerSize,
//                          const cardinal maxPacketSize)
// {
//    AudioQuality quality(samplingRate,bits,channels);
//    TransportInfo ti;
//    SimpleAudioPacket pkt;
//    pkt.getTransportInfo(ti,headerSize,maxPacketSize,quality,quality);
//    ti.TotalBytesPerSecondLimit   = (card64)-1;
//    ti.TotalPacketsPerSecondLimit = (card32)-1;
//    ti.TotalFramesPerSecondLimit  = (card32)-1;
//    std::cout << ti.CurrentSetting;
// }


// ###### Quality information printing ######################################
void printQualities(const cardinal maxPacketSize)
{
   const cardinal headerSize    = IPv6HeaderSize + UDPHeaderSize + RTPConstants::RTPDefaultHeaderSize;

   std::cout << "IPv4 Header             = " << sizeof(iphdr) << std::endl;
   std::cout << "IPv6 Header             = " << sizeof(ip6_hdr) << std::endl;
   std::cout << "UDP Header              = " << sizeof(udphdr) << std::endl;
   std::cout << "RTPDefaultHeaderSize    = " << RTPConstants::RTPDefaultHeaderSize << std::endl;
   std::cout << "MaxPacketSize           = " << maxPacketSize << std::endl;
   std::cout << "SimpleAudioHeaderSize   = " << sizeof(SimpleAudioPacket) << std::endl;
   std::cout << "AdvancedAudioHeaderSize = " << sizeof(AdvancedAudioPacket) << std::endl;
   std::cout << "RTCPReceiverReport      = " << sizeof(RTCPReceiverReport) << std::endl;
   std::cout << "RTCPbye                 = " << sizeof(RTCPBye) << std::endl;
   std::cout << std::endl << "Simple Audio Encoding:" << std::endl << std::endl;

//    for(cardinal i = 0;i < AudioQuality::ValidRates;i++) {
//       const cardinal rate = AudioQuality::ValidRatesTable[i];
//       for(cardinal j = 0;j < AudioQuality::ValidBits;j++) {
//          cardinal bits = AudioQuality::ValidBitsTable[j];
//          printQualityInfoSAE(rate,bits,1,headerSize,maxPacketSize);
//          printQualityInfoSAE(rate,bits,2,headerSize,maxPacketSize);
//       }
//    }
//
//    std::cout << std::endl << std::endl << "Advanced Audio Encoding:" << std::endl << std::endl;
//    for(cardinal i = 0;i < AudioQuality::ValidRates;i++) {
//       const cardinal rate = AudioQuality::ValidRatesTable[i];
//       for(cardinal j = 0;j < AudioQuality::ValidBits;j++) {
//          cardinal bits = AudioQuality::ValidBitsTable[j];
//          printQualityInfoAAE(rate,bits,1,headerSize,maxPacketSize);
//          printQualityInfoAAE(rate,bits,2,headerSize,maxPacketSize);
//       }
//    }
}


// // ###### Print TransportInfos ##############################################
// void printTransportInfos(const cardinal maxPacketSize)
// {
//    const cardinal headerSize = IPv6HeaderSize + UDPHeaderSize +
//                                RTPConstants::RTPDefaultHeaderSize;
//
//    AudioQuality quality(44100,16,2);
//    TransportInfo       tia,tib;
//    AdvancedAudioPacket a;
//    SimpleAudioPacket   b;
//    b.getTransportInfo(tia,headerSize,maxPacketSize,quality,quality);
//    tia.TotalBytesPerSecondLimit   = (card64)-1;
//    tia.TotalPacketsPerSecondLimit = (card32)-1;
//    tia.TotalFramesPerSecondLimit  = (card32)-1;
//    a.getTransportInfo(tib,headerSize,maxPacketSize,quality,quality);
//    tib.TotalBytesPerSecondLimit   = (card64)-1;
//    tib.TotalPacketsPerSecondLimit = (card32)-1;
//    tib.TotalFramesPerSecondLimit  = (card32)-1;
//    std::cout << "Simple Audio Encoding:" << std::endl << std::endl << tia << std::endl << std::endl;
//    std::cout << "Advanced Audio Encoding:" << std::endl << std::endl << tib << std::endl;
// }


// ###### Print quality levels ##############################################
void printQualityLevels()
{
   AudioQuality q = AudioQuality::LowestQuality;
   for(int i = 0;i < 256;i++) {
      std::cout << "#" << i << ":   " << q << "   " << q.getBytesPerSecond() << " Bytes/s" << std::endl;
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
      std::cerr << "Usage: " << argv[0] << " [-ti|-quality|-levels] {-maxpktsize=bytes}" << std::endl;
      exit(1);
   }
   if(argc > 2) {
      if(!(strncasecmp(argv[2],"-maxpktsize=",12))) {
         maxPacketSize = (cardinal)atol(&argv[2][12]);
      }
      else {
         std::cerr << "ERROR: Unknown option!" << std::endl;
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
   std::cout << "RTP Audio Encoder Info - Copyright (C) 1999-2012 Thomas Dreibholz" << std::endl;
   std::cout << "-----------------------------------------------------------------" << std::endl;
   std::cout << std::endl;
   std::cout << "Max Packet Size: " << maxPacketSize << std::endl;
   std::cout << std::endl;


   // ====== Print encoder information ======================================
   if(!(strcasecmp(argv[1],"-quality")))
      printQualities(maxPacketSize);
//    else if(!(strcasecmp(argv[1],"-ti")))
//       printTransportInfos(maxPacketSize);
   else if(!(strcasecmp(argv[1],"-levels")))
      printQualityLevels();
   else {
      std::cerr << "Usage:" << argv[0] << " [-ti|-quality|-levels]" << std::endl;
      exit(1);
   }

   return(0);
}
