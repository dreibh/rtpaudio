// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Pinger Host                                                      ####
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


#ifndef PINGERHOST_H
#define PINGERHOST_H


#include "tdsystem.h"
#include "tdsocket.h"
#include "strings.h"
#include "internetaddress.h"
#include "timedthread.h"


/**
  * This structure contains internal information for RoundTripTimePinger.
  *
  * @short   PingerHost
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
struct PingerHost
{
   /**
     * InternetAddress to send ping to.
     */
   InternetAddress Address;

   /**
     * The ping address in text format.
     */
   String AddressString;

   /**
     * Timestamp of last sent ping.
     */
   card64 LastPingTimeStamp;

   /**
     * Timestamp of last received ping.
     */
   card64 LastEchoTimeStamp;

   /**
     * Round trip time.
     */
   cardinal RoundTripTime;

   /**
     * Maximum raw round trip time (directly calculated from packet).
     */
   cardinal MaxRawRoundTripTime;

   /**
     * User counter (number of addHost() calls for this destination).
     */
   cardinal UserCount;

   /**
     * Sequence number.
     */
   card16 SeqNum;

   /**
     * Traffic class.
     */
   card8 TrafficClass;

   /**
     * Does this address use IPv6?
     */
   bool IsIPv6;
};


/**
  * Operator "==".
  */
inline int operator==(const PingerHost& ph1, const PingerHost& ph2);

/**
  * Operator "!=".
  */
inline int operator==(const PingerHost& ph1, const PingerHost& ph2);

/**
  * Operator "<".
  */
inline int operator<(const PingerHost& ph1, const PingerHost& ph2);

/**
  * Operator ">".
  */
inline int operator>(const PingerHost& ph1, const PingerHost& ph2);


#include "pingerhost.icc"


#endif
