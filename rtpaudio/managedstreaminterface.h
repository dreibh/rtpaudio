// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Managed Stream Interface                                         ####
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


#ifndef MANAGEDSTREAMINTERFACE_H
#define MANAGEDSTREAMINTERFACE_H


#include "tdsystem.h"
#include "abstractqosdescription.h"


/**
  * This is an interface for a bandwidth-managed stream.
  *
  * @short   Managed Stream Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
*/
class ManagedStreamInterface
{
   public:
   /**
     * Destructor.
     */
   virtual ~ManagedStreamInterface() { };

   /**
     * Get QoS description for current time + offset.
     *
     * @param offset Offset in microseconds.
     * @return QoS Description.
     */
   virtual AbstractQoSDescription* getQoSDescription(const card64 offset) = 0;

   /**
     * Update encoder's quality with changes made in QoS description returned b
     * getQoSDescription().
     *
     * @param aqd QoS Description.
     *
     * @see RTPSender#getQoSDescription
     */
   virtual void updateQuality(const AbstractQoSDescription* aqd) = 0;


   /**
     * Lock stream.
     */
   virtual void lock() = 0;

   /**
     * Unlock stream.
     */
   virtual void unlock() = 0;
};


#endif
