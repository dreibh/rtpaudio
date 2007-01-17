// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Managed Stream Interface                                         ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#ifndef MANAGEDSTREAMINTERFACE_H
#define MANAGEDSTREAMINTERFACE_H


#include "tdsystem.h"
#include "abstractqosdescription.h"


namespace Coral {


/**
  * This is an interface for a bandwidth-managed stream.
  *
  * @short   Managed Stream Interface
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
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


}


#endif
