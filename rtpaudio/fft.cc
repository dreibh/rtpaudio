// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Fast Fourier Transformation implementation                       ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
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
#include "fft.h"
#include "tools.h"

#include <iostream>
#include <sys/time.h>


// ###### Constructor #######################################################
FastFourierTransformation::FastFourierTransformation(const integer fftlen)
{
   integer i;
   integer temp;
   integer mask;

   /*
    * FFT size is only half the number of data points
    * The full FFT output can be reconstructed from this FFT's output.
    * (This optimization can be made since the data is real.)
    */
   Points = fftlen;

   if((SinTable = new int16[Points]) == NULL) {
      std::cerr << "ERROR: FastFourierTransformation::FastFourierTransformation() - Out of memory!" << std::endl;
      ::abort();
   }
   if((BitReversed = new integer[Points / 2]) == NULL) {
      std::cerr << "ERROR: FastFourierTransformation::FastFourierTransformation() - Out of memory!" << std::endl;
      ::abort();
   }

   for(i = 0;i < Points / 2;i++) {
      temp=0;
      for(mask = Points / 4;mask > 0;mask >>= 1)
         temp=(temp >> 1) + (i & mask ? Points / 2 : 0);

      BitReversed[i] = temp;
   }

   for(i = 0;i < Points / 2;i++) {
      register double s,c;
      s = floor(-32768.0 * sin(2 * M_PI *i / (Points)) + 0.5);
      c = floor(-32768.0 * cos(2 * M_PI *i / (Points)) + 0.5);
      if(s > 32767.5) s = 32767;
      if(c > 32767.5) c = 32767;
      SinTable[BitReversed[i]  ] = (int16)s;
      SinTable[BitReversed[i]+1] = (int16)c;
   }
}


// ###### Destructor ########################################################
FastFourierTransformation::~FastFourierTransformation() {
   delete BitReversed;
   delete SinTable;
   Points = 0;
}


// ###### Do Fourier transformation #########################################
void FastFourierTransformation::fft(int16* buffer) {
   integer ButterfliesPerGroup = Points / 4;

   endptr1 = buffer + Points;

   /*
    *  Butterfly:
    *     Ain-----Aout
    *         \ /
    *         / \
    *     Bin-----Bout
    */

   while(ButterfliesPerGroup > 0)
   {
      A = buffer;
      B = buffer + ButterfliesPerGroup * 2;
      sptr = SinTable;

      while(A < endptr1)
      {
         register int16 sin = *sptr;
         register int16 cos = *(sptr + 1);
         endptr2 = B;
         while(A < endptr2)
         {
            long v1 = ((long) *B * cos + (long) *(B + 1) * sin) >> 15;
            long v2 = ((long) *B * sin - (long) *(B + 1) * cos) >> 15;
	    *B = (*A + v1) >> 1;
            *(A++)=*(B++) - v1;
	    *B = (*A - v2) >> 1;
            *(A++) = *(B++) + v2;
         }
         A = B;
         B += ButterfliesPerGroup * 2;
         sptr += 2;
      }
      ButterfliesPerGroup >>= 1;
   }
   br1 = BitReversed + 1;
   br2 = BitReversed + Points / 2 - 1;

   while(br1 <= br2)
   {
      register long temp1,temp2;
      int16 sin = SinTable[*br1];
      int16 cos = SinTable[*br1 + 1];
      A = buffer + *br1;
      B = buffer + *br2;
      HRplus = (HRminus = *A     - *B    ) + (*B << 1);
      HIplus = (HIminus = *(A + 1) - *(B + 1)) + (*(B + 1) << 1);
      temp1  = ((long)sin * HRminus - (long)cos * HIplus) >> 15;
      temp2  = ((long)cos * HRminus + (long)sin * HIplus) >> 15;
      *B       = (*A     = (HRplus  + temp1) >> 1) - temp1;
      *(B + 1) = (*(A + 1) = (HIminus + temp2) >> 1) - HIminus;

      br1++;
      br2--;
   }

   buffer[0] += buffer[1];
   buffer[1] = 0;
}


// ###### Get pointer to BitReversed array ##################################
integer* FastFourierTransformation::getBitReversed() {
  return BitReversed;
}
