// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Converter Implementation                                   ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   http://www.iem.uni-due.de/~dreibh/rtpaudio             ####
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
#include "audioconverter.h"


// ###### Get aligned length for conversion's result ########################
cardinal getAlignedLength(const AudioQualityInterface& inputQuality,
                          const AudioQualityInterface& outputQuality,
                          const cardinal               inputLength)
{
   const cardinal length = (cardinal)floor(((double)inputLength * (double)outputQuality.getBytesPerSecond()) / (double)inputQuality.getBytesPerSecond());

   cardinal alignment = 1;
   if(outputQuality.getBits() == 4) {
      if(outputQuality.getChannels() > 1)
         alignment = 2;
   }
   else if(outputQuality.getBits() == 12) {
      if(outputQuality.getChannels() > 1)
         alignment = 6;
      else
         alignment = 3;
   }
   else if(outputQuality.getBits() == 16) {
      if(outputQuality.getChannels() > 1)
         alignment = 4;
      else
         alignment = 2;
   }

   return(length - (length % alignment));
}


// ###### Get conversion parameters #########################################
bool getConvParams(const cardinal in,
                   const cardinal out,
                   cardinal&      a,
                   cardinal&      b,
                   float&        c)
{
   for(cardinal y = 1;y < 20;y++) {
      cardinal rest = (y * in) % out;
      if(rest == 0) {
         b = y;
         a = (y * in) / out;
         c = (float)in / (float)out;
         return(true);
      }
   }
   return(false);
}


// ###### Get 12-bit audio values ###########################################
inline void get12(const card8* buffer, card16& a, card16& b)
{
   a = (((card16)buffer[0] << 4) | ((card16)buffer[2] >> 4)) << 4;
   b = (((card16)buffer[1] << 4) | ((card16)buffer[2] & 0x000f)) << 4;
}


// ###### Set 12-bit audio values ###########################################
inline void set12(card8* buffer, const card16 c, const card16 d)
{
   const card16 a = c >> 4;
   const card16 b = d >> 4;
   buffer[0] = (card8)(a >> 4);
   buffer[1] = (card8)(b >> 4);
   buffer[2] = ((card8)(a & 0x000f) << 4) | (b & 0x00f);
}


// ###### AudioConverter implementation #####################################
cardinal AudioConverter(const AudioQualityInterface& from,
                        const AudioQualityInterface& to,
                        const card8*                 inputBuffer,
                        card8*                       outputBuffer,
                        const cardinal               inputLength,
                        const cardinal               outputLength)
{
   // cout << "in: " << inputLength << "   " <<  from << " -> " << to << endl;

   // ====== Check for errors ===============================================
   const cardinal inputBytesPerSecond  = from.getBytesPerSecond();
   const cardinal outputBytesPerSecond = to.getBytesPerSecond();
   cardinal required = (cardinal)ceil(((double)inputLength * (double)outputBytesPerSecond) / (double)inputBytesPerSecond);
   if(outputLength < (cardinal)required) {
      std::cerr << "WARNING: AudioConverter() - Output buffer too small: "
                << outputLength << " < " << required << "!" << std::endl;
      return(0);
   }
   if(from == to) {
      memcpy((void*)outputBuffer,(void*)inputBuffer,inputLength);
      return(inputLength);
   }


   // ====== Initialize =====================================================
   const cardinal maximumBytesPerSecond = AudioQuality::HighestQuality.getBytesPerSecond();
   required = (cardinal)ceil(((double)inputLength * (double)maximumBytesPerSecond) / (double)inputBytesPerSecond);
   card8 workBuffer[(size_t)required];
   card16* workBuffer16    = (card16*)&workBuffer;
   card16*  inputBuffer16  = (card16*)inputBuffer;
   card16*  outputBuffer16 = (card16*)outputBuffer;
   card8    channels       = from.getChannels();
   card8    bits           = from.getBits();
   cardinal length         = inputLength;


   // ====== Convert to stereo/16 bits quality ==============================
   cardinal k = 0;
   if(bits == 4) {
      if(channels == 1) {
         for(cardinal i = 0;i < length;i++) {
            workBuffer16[k++] = ((card16)((inputBuffer[i] & 0xf0) - 127)) << 8;
            workBuffer16[k++] = ((card16)((inputBuffer[i] & 0xf0) - 127)) << 8;
            workBuffer16[k++] = ((card16)(((inputBuffer[i] & 0x0f) << 4) - 127)) << 8;
            workBuffer16[k++] = ((card16)(((inputBuffer[i] & 0x0f) << 4) - 127)) << 8;
         }
      }
      else {
         for(cardinal i = 0;i < length;i += 2) {
            workBuffer16[k++] = ((card16)((inputBuffer[i    ] & 0xf0) - 127)) << 8;
            workBuffer16[k++] = ((card16)((inputBuffer[i + 1] & 0xf0) - 127)) << 8;
            workBuffer16[k++] = ((card16)(((inputBuffer[i    ] & 0x0f) << 4) - 127)) << 8;
            workBuffer16[k++] = ((card16)(((inputBuffer[i + 1] & 0x0f) << 4) - 127)) << 8;
         }
      }
      length   = k << 1;
      bits     = 16;
      channels = 2;
   }
   else if(bits == 8) {
      if(channels == 1) {
         for(cardinal i = 0;i < length;i++) {
            workBuffer16[k++] = ((card16)(inputBuffer[i] - 127)) << 8;
            workBuffer16[k++] = ((card16)(inputBuffer[i] - 127)) << 8;
         }
      }
      else {
         for(cardinal i = 0;i < length;i++) {
            workBuffer16[k++] = ((card16)(inputBuffer[i] - 127)) << 8;
         }
      }
      length   = k << 1;
      bits     = 16;
      channels = 2;
   }
   else if(bits == 12) {
      if(channels == 1) {
         for(cardinal i = 0;i < length;i += 3) {
            card16 a,b;
            get12(&inputBuffer[i],a,b);

            workBuffer16[k++] = a;
            workBuffer16[k++] = a;
            workBuffer16[k++] = b;
            workBuffer16[k++] = b;
         }
      }
      else {
         for(cardinal i = 0;i < length;i += 6) {
            card16 al,bl,ar,br;
            get12(&inputBuffer[i    ],al,bl);
            get12(&inputBuffer[i + 3],ar,br);
            workBuffer16[k++] = al;
            workBuffer16[k++] = ar;
            workBuffer16[k++] = bl;
            workBuffer16[k++] = br;
         }
      }
      length   = k << 1;
      bits     = 16;
      channels = 2;
   }
   else if(bits == 16) {
      const cardinal maxIndex = length >> 1;
      if(from.getByteOrder() != BYTE_ORDER) {
         if(channels == 1) {
            for(cardinal i = 0;i < maxIndex;i++) {
               workBuffer16[k++] = translate16(inputBuffer16[i]);
               workBuffer16[k++] = translate16(inputBuffer16[i]);
            }
         }
         else {
            for(cardinal i = 0;i < maxIndex;i++) {
               workBuffer16[k++] = translate16(inputBuffer16[i]);
            }
         }
      }
      else {
         if(channels == 1) {
            for(cardinal i = 0;i < maxIndex;i++) {
               workBuffer16[k++] = inputBuffer16[i];
            workBuffer16[k++] = inputBuffer16[i];
            }
         }
         else {
            for(cardinal i = 0;i < maxIndex;i++) {
               workBuffer16[k++] = inputBuffer16[i];
            }
         }
      }
      length   = k << 1;
      channels = 2;
   }


   // ====== Get conversion parameters ======================================
   cardinal a,b;
   float    c;
   if((getConvParams(from.getSamplingRate(),to.getSamplingRate(),a,b,c)) == false) {
      std::cerr << "WARNING: AudioConverter: Unable to convert rate "
                << from.getSamplingRate() << " to " << to.getSamplingRate() << "!" << std::endl;
      return(length);
   }

   // std::cout << "a=" << a << " b=" << b << " c=" << c << std::endl;

   // ====== Convert sampling rate ==========================================
   if((a != 1) || (b != 1)) {
      // ====== Do cheap conversion, if possible ============================
      if((a - b) < 2) {
         k = 0;
         const cardinal maxIndex = length >> 1;
         for(cardinal i = 0;   ;i += 2*a) {
            for(cardinal j = 0;j < b;j++) {
               const cardinal l = i + (j * 2);
               const cardinal r = l + 1;
               if(r >= maxIndex) {
                  goto conversionComplete;
               }
               workBuffer16[k++] = workBuffer16[l];
               workBuffer16[k++] = workBuffer16[r];
            }
         }
      }
      // ====== Do expensive conversion using floats ========================
      else {
         k = 0;
         const cardinal maxIndex = length >> 1;
         for(float i = 0.0;   ;i += c) {
            const cardinal l = ((cardinal)i) << 1;
            const cardinal r = l + 1;
            if(r >= maxIndex) {
               goto conversionComplete;
            }
            workBuffer16[k++] = workBuffer16[l];
            workBuffer16[k++] = workBuffer16[r];
         }
      }
conversionComplete:
      length = k << 1;
   }

   // ====== Convert bits and channels to requested output values ===========
   if(to.getBits() == 4) {
      k = 0;
      const cardinal maxIndex = length >> 1;
      if(to.getChannels() == 1) {
         for(cardinal i = 0;i < maxIndex;i += 4) {
            const card8 a = 127 + (card8)(workBuffer16[i    ] >> 8);
            const card8 b = 127 + (card8)(workBuffer16[i + 2] >> 8);
            outputBuffer[k++] = (a & 0xf0) | ((b & 0xf0) >> 4);
         }
      }
      else {
         for(cardinal i = 0;i < maxIndex;i += 4) {
            const card8 al = 127 + (card8)(workBuffer16[i    ] >> 8);
            const card8 ar = 127 + (card8)(workBuffer16[i + 1] >> 8);
            const card8 bl = 127 + (card8)(workBuffer16[i + 2] >> 8);
            const card8 br = 127 + (card8)(workBuffer16[i + 3] >> 8);
            outputBuffer[k++] = (al & 0xf0) | ((bl & 0xf0) >> 4);
            outputBuffer[k++] = (ar & 0xf0) | ((br & 0xf0) >> 4);
         }
      }
      length = k;
   }
   else if(to.getBits() == 8) {
      k = 0;
      const cardinal maxIndex = length >> 1;
      if(to.getChannels() == 1) {
         for(cardinal i = 0;i < maxIndex;i += 2) {
            const card8 p = 127 + (card8)(workBuffer16[i] >> 8);
            outputBuffer[k++] = p;
         }
      }
      else {
         for(cardinal i = 0;i < maxIndex;i += 2) {
            const card8 l = 127 + (card8)(workBuffer16[i    ] >> 8);
            const card8 r = 127 + (card8)(workBuffer16[i + 1] >> 8);
            outputBuffer[k++] = l;
            outputBuffer[k++] = r;
         }
      }
      length = k;
   }
   else if(to.getBits() == 12) {
      k = 0;
      const cardinal maxIndex = length >> 1;
      if(to.getChannels() == 1) {
         for(cardinal i = 0;i < maxIndex;i += 4) {
            set12(&outputBuffer[k],workBuffer16[i],workBuffer16[i + 2]);
            k += 3;
         }
      }
      else {
         for(cardinal i = 0;i < maxIndex;i += 4) {
            const card16 al = workBuffer16[i    ];
            const card16 bl = workBuffer16[i + 2];
            const card16 ar = workBuffer16[i + 1];
            const card16 br = workBuffer16[i + 3];
            set12(&outputBuffer[k    ],al,bl);
            set12(&outputBuffer[k + 3],ar,br);
            k += 6;
         }
      }
      length = k;
   }
   else if(to.getBits() == 16) {
      k = 0;
      const cardinal maxIndex = length >> 1;
      if(to.getByteOrder() != BYTE_ORDER) {
         if(to.getChannels() == 1) {
            for(cardinal i = 0;i < maxIndex;i += 2) {
               outputBuffer16[k++] = translate16(workBuffer16[i]);
            }
            length = k << 1;
         }
         else {
            for(cardinal i = 0;i < maxIndex;i++) {
               outputBuffer16[k++] = translate16(workBuffer16[i]);
            }
         }
      }
      else {
         if(to.getChannels() == 1) {
            for(cardinal i = 0;i < maxIndex;i += 2) {
               outputBuffer16[k++] = workBuffer16[i];
            }
            length = k << 1;
         }
         else {
            for(cardinal i = 0;i < maxIndex;i++) {
               outputBuffer16[k++] = workBuffer16[i];
            }
         }
      }
   }

   // cout << "out: " << length <<  endl;
   return(length);
}
