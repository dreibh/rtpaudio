// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Traffic Class Values                                             ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2014 by Thomas Dreibholz            ####
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


#ifndef TRAFFICCLASSVALUES_H
#define TRAFFICCLASSVALUES_H


#include "tdsystem.h"



/**
  * This class contains a set of values for the traffic class/TOS byte of
  * IP packets. This class contains only static methods and attributes.
  *
  * @short   Traffic Class Values
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class TrafficClassValues
{
   // ====== Values =========================================================
   public:
   /**
     * Number of values.
     */
   static const cardinal MaxValues = 16;


   /**
     * Get traffic class of given index.
     *
     * @param index Index.
     * @return Traffic class.
     */
   inline static card8 getTrafficClassForIndex(const cardinal index);

   /**
     * Get traffic class for name.
     *
     * @param name Name.
     * @return Traffic class or 0xffff, if name in unknown.
     */
   static const card16 getTrafficClassForName(const char* name);


   /**
     * Get name for given traffic class.
     *
     * @param trafficClass Traffic class.
     * @return Name.
     */
   static const char* getNameForTrafficClass(const card8 trafficClass);

   /**
     * Get name for index entry.
     *
     * @param index Index.
     * @return Name.
     */
   inline static const char* getNameForIndex(const cardinal index);


   /**
     * Get index for given traffic class.
     *
     * @param trafficClass Traffic class.
     * @return Index.
     */
   static cardinal getIndexForTrafficClass(const card8 trafficClass);


   // ====== Private data ===================================================
   private:
   static const card8 TCValues[MaxValues];
   static const char* TCNames[TrafficClassValues::MaxValues];
};


#include "trafficclassvalues.icc"


#endif
