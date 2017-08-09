// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Randomizer implementation                                        ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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
#include "randomizer.h"
#include "tools.h"



// ###### Constructor #######################################################
Randomizer::Randomizer()
{
   setSeed();
}


// ###### Set randomizer seed ###############################################
void Randomizer::setSeed()
{
   Value = (card32)getMicroTime();
}


// ###### Set randomizer seed ###############################################
void Randomizer::setSeed(const cardinal seed)
{
   Value = seed;
}


// ###### Generate random cardinal number out of interval [a,b] #############
cardinal Randomizer::random(const cardinal a, const cardinal b)
{
   const cardinal c = b - a + 1;
   cardinal number;
   if(sizeof(cardinal) == 4)
      number = random32();
   else
      number = random64();

   if(c == 0)
      return(a);
   else
      return((number % c) + a);
}


// ###### Generate random double number out of interval [a,b] #############
double Randomizer::random(const double a, const double b)
{
   const double c = b - a;
   return((random() * c) + a);
}
