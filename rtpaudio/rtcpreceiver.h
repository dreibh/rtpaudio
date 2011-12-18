// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Receiver                                                    ####
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


#ifndef RTCPRECEIVER_H
#define RTCPRECEIVER_H


#include "tdsystem.h"
#include "tdsocket.h"
#include "thread.h"
#include "rtcppacket.h"
#include "rtcpabstractserver.h"


/**
  * This class implements an RTCP receiver based on Thread.
  *
  * @short   RTCP Receiver
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RTCPReceiver : public Thread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Default constructor.
     * You have to initialize RTCPReceiver by calling init(...) later!
     *
     * @see init
     */
   RTCPReceiver();

   /**
     * Constructor for new RTCPReceiver. The new receiver's thread has to be
     * started by calling start()!
     *
     * @param server RTCPAbstractServer.
     * @param receiverSocket Socket to receive RTCP packets from.
     */
   RTCPReceiver(RTCPAbstractServer* server,
                Socket*             receiverSocket);

   /**
     * Destructor.
     */
   ~RTCPReceiver();


   // ====== Initialize =====================================================
   /**
     * Initialize new RTCPReceiver. The new receiver's thread has to be
     * started by calling start()!
     *
     * @param server RTCPAbstractServer.
     * @param receiverSocket Socket to receive RTCP packets from.
     */
   void init(RTCPAbstractServer* server,
             Socket*             receiverSocket);


   // ====== Private data ===================================================
   private:
   void  run();


   Socket*             ReceiverSocket;
   RTCPAbstractServer* Server;
   double              AverageRTCPSize; // Average compound RTCP packet size
};


#endif
