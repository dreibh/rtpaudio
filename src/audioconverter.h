// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Converter                                                  ####
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


#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H


#include "tdsystem.h"
#include "audioquality.h"


/**
  * Audio quality converter. Convert quality from a given value to a given
  * value. Note: The "from" value must be greater than or equal to the "to"
  * value, that is from-sampling rate >= to-sampling rate, from-bits >= to-bits,
  * from-channels >= to-channels.
  *
  * @param from Quality to convert from.
  * @param to Quality to convert to.
  * @param inputBuffer Input buffer.
  * @param outputBuffer Output buffer.
  * @param inputLength Length of the audio data in input buffer.
  * @param outputLength Length of the output buffer.
  * @return Length after conversion.
  */
cardinal AudioConverter(const AudioQualityInterface& from,
                        const AudioQualityInterface& to,
                        const card8*                 inputBuffer,
                        card8*                       outputBuffer,
                        const cardinal               inputLength,
                        const cardinal               outputLength);


/**
  * Get aligned output length for a conversion from given input quality and
  * input length to output quality.
  * Example: 12 Bit/Stereo has a 6-byte alignment: L1L1R1R2 = 48 bits = 6 Bytes.
  *
  * @param inputQuality Input quality.
  * @param outputQuality Output quality.
  * @param inputLength Input length.
  * @return Aligned length.
  */
cardinal getAlignedLength(const AudioQualityInterface& inputQuality,
                          const AudioQualityInterface& outputQuality,
                          const cardinal               inputLength);


/**
  * Get parameters for audio conversion.
  * New sampling rate = (a * OldSamplingRate) / b;
  *
  * @param in Old sampling rate.
  * @param out New sampling rate.
  * @param a Reference to store a.
  * @param b Reference to store b.
  * @param c Reference to store float in / out.
  * @return true, if a and b have been found; false, if there are no such numbers for b out of the set {1,2,...,20}
  */
bool getConvParams(const     cardinal in,
                   const     cardinal out,
                   cardinal& a,
                   cardinal& b,
                   float&    c);


#endif
