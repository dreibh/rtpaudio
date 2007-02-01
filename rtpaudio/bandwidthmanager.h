// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Bandwidth Manager                                                ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#ifndef BANDWIDTHMANAGER_H
#define BANDWIDTHMANAGER_H


#include "tdsystem.h"
#include "abstractqosdescription.h"
#include "managedstreaminterface.h"
#include "rtcppacket.h"


class BandwidthManager
{
   public:
   /**
     * Destructor.
     */
   virtual ~BandwidthManager() = 0;

   // ====== Stream management ==============================================
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
