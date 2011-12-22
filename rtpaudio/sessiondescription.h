// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Session Description                                              ####
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


#ifndef SESSIONDESCRIPTION_H
#define SESSIONDESCRIPTION_H


#include "tdsystem.h"
#include "streamdescription.h"


#include <map>


struct StreamDescription;


/**
  * This structure contains a description of a session. It is used
  * for the bandwidth manager's remapping algorithm.
  *
  * @short   Session Description
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
struct SessionDescription
{
   /**
     * Session ID.
     */
   cardinal SessionID;

   /**
     * Number of streams.
     */
   cardinal Streams;

   /**
     * Stream description set.
     */
   std::multimap<ManagedStreamInterface*,StreamDescription*> StreamSet;

   /**
     * Minimum session bandwidth.
     */
   card64 MinWantedBandwidth;

   /**
     * Maximum session bandwidth.
     */
   card64 MaxWantedBandwidth;

   /**
     * Total allocated bandwidth.
     */
   card64 TotalAllocatedBandwidth;

   /**
     * Allocated bandwidth for each class.
     */
   card64 AllocatedBandwidthArray[TrafficClassValues::MaxValues];

   /**
     * Session priority.
     */
   int8 Priority;

   /**
     * True, if all following higher bandwidth allocations will fail (no more
     * bandwidth available to achieve higher quality -> no more allocation trials
     * necessary); false otherwise.
     */
   bool MaximumReached;
};


#endif
