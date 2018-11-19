// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Fast Fourier Transformation                                      ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2019 by Thomas Dreibholz            ####
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


#ifndef FFT_H
#define FFT_H


#include "tdsystem.h"
#include "audiowriterinterface.h"


/**
  * This class does fast fourier transformation.
  *
  * @short   Fast Fourier Transformation
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class FastFourierTransformation {
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   FastFourierTransformation(const integer fftlen);

   /**
     * Destructor.
     */
   ~FastFourierTransformation();


   // ====== FFT functions ==================================================
   /**
     * Do Fourier transformation.
     *
     * @param buffer Input buffer.
     */
   void fft(int16* buffer);

   /**
     * Get BitReversed array.
     */
   integer* getBitReversed();


   // ====== Private data ===================================================
   private:
   integer* BitReversed;
   int16*   SinTable;
   integer  Points;

   int16    *A,*B;
   int16    *sptr;
   int16    *endptr1,*endptr2;
   integer  *br1,*br2;
   integer  HRplus,HRminus,HIplus,HIminus;
};


#endif
