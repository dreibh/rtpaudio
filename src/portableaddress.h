// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Portable Address implementation                                  ####
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


#ifndef PORTABLEADDRESS_H
#define PORTABLEADDRESS_H


#include "tdsystem.h"



/**
  * Binary representation for a socket address for sending the address over
  * a network. The difference between InternetAddress is that PortableAddress
  * does not contain hidden information on virtual function management, which
  * make network transfer of InternetAddress objects problematic.
  *
  * @short   Portable Internet Address
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class PortableAddress
{
   // ====== Comparision operators ==========================================
   public:
   /**
     * Implementation of == operator.
     */
   int operator==(const PortableAddress& address) const;

   /**
     * Implementation of == operator.
     */
   int operator!=(const PortableAddress& address) const;

   /**
     * Implementation of < operator.
     */
   int operator<(const PortableAddress& address) const;

   /**
     * Implementation of <= operator.
     */
   int operator<=(const PortableAddress& address) const;

   /**
     * Implementation of > operator.
     */
   int operator>(const PortableAddress& address) const;

   /**
     * Implementation of >= operator.
     */
   int operator>=(const PortableAddress& address) const;


   // ====== Reset ==========================================================
   /**
     * Reset portable address.
     */
   inline void reset();


   // ====== Address data ===================================================
   public:
   /**
     * Host address in network byte order. IPv4 addresses are converted to
     * IPv4-mapped IPv6 addresses.
     */
   card16 Host[8];

   /**
     * Port number.
     */
   card16 Port;
};


#include "portableaddress.icc"


#endif
