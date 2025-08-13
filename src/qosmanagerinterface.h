// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### QoS Manager Interface                                            ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2025 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: thomas.dreibholz@gmail.com                             ####
// ####    WWW:   https://www.nntb.no/~dreibh/rtpaudio                   ####
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


#ifndef QOSMANAGERINTERFACE_H
#define QOSMANAGERINTERFACE_H


#include "tdsystem.h"
#include "abstractqosdescription.h"
#include "managedstreaminterface.h"
#include "rtcppacket.h"


class QoSManagerInterface
{
   // ====== Stream management ==============================================
   public:
   /**
     * Add stream to management.
     *
     * @param stream Stream to add.
     * @param session SessionID of session to add stream to (0 for no session).
     * @param name Stream name (only for log printing).
     */
   virtual void addStream(ManagedStreamInterface* stream,
                          const cardinal          sessionID = 0,
                          const char*             name      = NULL) = 0;

   /**
     * Remove stream from management.
     *
     * @param stream Stream to remove.
     */
   virtual void removeStream(ManagedStreamInterface* stream) = 0;

   /**
     * Update stream.
     *
     * @param stream Stream to be updated.
     */
   virtual void updateStream(ManagedStreamInterface* stream) = 0;


   // ====== Events =========================================================
   /**
     * Interval has changed.
     *
     * @param stream Stream with changed interval.
     * @param isNew true, if new interval has been reached; false otherwise.
     * @param when Microseconds to next interval.
     * @param newRUList true, if new resource/utiliztion list has been reached; false otherwise.
     */
   virtual void intervalChangeEvent(ManagedStreamInterface* stream,
                                    const bool              isNew,
                                    const card64            when,
                                    const bool              newRUList) = 0;

   /**
     * Report reception for given layer.
     *
     * @param stream Stream.
     * @param report Report.
     * @param layer Layer.
     */
   virtual void reportEvent(ManagedStreamInterface*         stream,
                            const RTCPReceptionReportBlock* report,
                            const cardinal                  layer) = 0;

   /**
     * Buffer flush for a given layer.
     *
     * @param stream Stream.
     */
   virtual void bufferFlushEvent(ManagedStreamInterface* stream,
                                 const cardinal          layer) = 0;
};


#endif
