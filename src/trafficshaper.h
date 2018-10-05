// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Traffic Shaper                                                   ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2018 by Thomas Dreibholz            ####
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


#ifndef TRAFFICSHAPER_H
#define TRAFFICSHAPER_H


#include "tdsystem.h"
#include "tdsocket.h"
#include "internetaddress.h"
#include "timedthread.h"
#include "trafficclassvalues.h"


#include <set>
#include <deque>
#include <vector>


class TrafficShaper;


/**
  * This class is a singleton for the traffic shaper.
  *
  * @short   Traffic Shaper Singleton
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
*/
class TrafficShaperSingleton : public TimedThread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   TrafficShaperSingleton();

   /**
     * Destructor.
     */
   ~TrafficShaperSingleton();


   // ====== Add/remove traffic shapers =====================================
   /**
     * Add traffic shaper object.
     *
     * @param ts TrafficShaper.
     */
   void addTrafficShaper(TrafficShaper* ts);

   /**
     * Remove traffic shaper object.
     *
     * @param ts TrafficShaper.
     */
   void removeTrafficShaper(TrafficShaper* ts);


   // ====== Private data ===================================================
   private:
   void timerEvent();


   std::vector<TrafficShaper*> ShaperSet;
   cardinal                    UserCount;
};



/**
  * This class is a traffic shaper.
  *
  * @short   Traffic Shaper
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
*/
class TrafficShaper : public Synchronizable
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   TrafficShaper();

   /**
     * Constructor.
     */
   TrafficShaper(Socket* socket);

   /**
     * Destructor.
     */
   ~TrafficShaper();


   // ====== Initialize =====================================================
   /**
     * Initialize.
     *
     * @param socket Socket.
     */
   void init(Socket* socket);


   // ====== Set sender socket ==============================================
   /**
     * Set socket to send shaped traffic to.
     *
     * @param socket Socket.
     */
   inline void setSocket(Socket* socket);


   // ====== Settings =======================================================
   /**
     * Get bandwidth for following packets.
     *
     * @return Bandwidth.
     */
   inline card64 getBandwidth() const;

   /**
     * Set bandwidth for following packets.
     *
     * @param bandwidth Bandwidth.
     */
   inline void setBandwidth(const card64 bandwidth);

   /**
     * Get maximum buffer delay for following packets.
     *
     * @return Maximum buffer delay in microseconds.
     */
   inline double getBufferDelay() const;

   /**
     * Set maximum buffer delay for following packets.
     *
     * @param bufferDelay Maximum buffer delay in microseconds.
     */
   inline void setBufferDelay(const double bufferDelay);


   // ====== Buffer manipulation ============================================
   /**
     * Flush buffer.
     */
   void flush();

   /**
     * Adapt buffer's contents to changed bandwidth and delay settings.
     *
     * @param trafficClass Traffic class to remap packets to.
     * @param doRemapping true, to do traffic class remapping; false otherwise.
     * @return true, if buffer flush has been necessary; false otherwise.
     */
   bool refreshBuffer(const card8 trafficClass,
                      const bool doRemapping);

   /**
     * Get sequence number of last packet sent.
     *
     * @return Sequence number.
     */
   inline cardinal getLastSeqNum();


   // ====== I/O functions ==================================================
   /**
     * Wrapper for sendto().
     * sendto() will set the packet's traffic class, if trafficClass is not 0.
     *
     * @param buffer Buffer with data to send.
     * @param length Length of data to send.
     * @param seqNum Packet's sequence number (-1 for none).
     * @param flags Flags for sendto().
     * @param receiver Address of receiver.
     * @return Bytes sent or error code < 0.
     */
    ssize_t sendTo(const void*         buffer,
                   const size_t        length,
                   const cardinal      seqNum,
                   const cardinal      flags,
                   const InternetFlow& receiver,
                   const card8         trafficClass = 0);

   /**
     * Wrapper for send().
     * send() will set the packet's traffic class, if trafficClass is not 0.
     * In this case, the packet will be sent by sendto() to the destination
     * address, the socket is connected to!
     *
     * @param buffer Buffer with data to send.
     * @param length Length of data to send.
     * @param seqNum Packet's sequence number (-1 for none).
     * @param flags Flags for sendto().
     * @param trafficClass Traffic class for packet.
     * @return Bytes sent or error code < 0.
     */
    ssize_t send(const void*    buffer,
                 const size_t   length,
                 const cardinal seqNum,
                 const cardinal flags        = 0,
                 const card8    trafficClass = 0);

   /**
     * Wrapper for write().
     *
     * @param buffer Buffer with data to write
     * @param length Length of data to write
     * @param seqNum Packet's sequence number (-1 for none).
     * @return Bytes sent or error code < 0.
     */
   ssize_t write(const void*    buffer,
                 const size_t   length,
                 const cardinal seqNum);


   // ====== Private data ===================================================
   private:
   void sendAll();
   ssize_t addPacket(const void*    data,
                     const cardinal bytes,
                     const cardinal seqNum,
                     InternetFlow&  destination,
                     const cardinal flags,
                     const cardinal command);


   enum TrafficShaperCommand {
      TSC_Write  = 0,
      TSC_Send   = 1,
      TSC_SendTo = 2
   };

   struct TrafficShaperPacket {
      card64       SendTimeStamp;
      cardinal     HeaderSize;
      cardinal     PayloadSize;
      cardinal     Flags;
      cardinal     Command;
      InternetFlow Destination;
      char*        Data;
      cardinal     SeqNum;

      inline int operator<(const TrafficShaperPacket& packet) const {
         return(SendTimeStamp < packet.SendTimeStamp);
      }
   };

   friend class TrafficShaperSingleton;


   static TrafficShaperSingleton   Singleton;
   std::deque<TrafficShaperPacket> Queue;
   Socket*                         SenderSocket;
   card64                          SendTimeStamp;
   card64                          Bandwidth;
   double                          BufferDelay;
   cardinal                        LastSeqNum;
};


#include "trafficshaper.icc"


#endif
