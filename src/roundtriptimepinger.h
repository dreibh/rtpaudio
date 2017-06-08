// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Round Trip Time Pinger                                           ####
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


#ifndef ROUNDTRIPTIMEPINGER_H
#define ROUNDTRIPTIMEPINGER_H


#include "tdsystem.h"
#include "tdsocket.h"
#include "internetaddress.h"
#include "timedthread.h"
#include "rtppacket.h"
#include "pingerhost.h"
#include "randomizer.h"


#include <set>
#include <algorithm>
#include <fstream>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <sys/time.h>


/**
  * This class implements a round trip time pinger.
  *
  * @short   Round Trip Time Pinger
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RoundTripTimePinger : public TimedThread
{
   // ====== Constructor ===================================================
   public:
   /**
     * Constructor.
     *
     * @param ping4socket Socket for IPv4 pings.
     * @param ping6socket Socket for IPv6 pings.
     * @param delay Maximum delay between two pings in microseconds.
     */
   RoundTripTimePinger(Socket*      ping4socket,
                       Socket*      ping6socket,
                       const card64 delay = 1000000);

   /**
     * Destructor.
     */
   ~RoundTripTimePinger();


   // ====== Status functions ===============================================
   /**
     * Check, if RoundTripTimePinger is ready.
     *
     * @return true, if RoundTripTimePinger is ready; false otherwise.
     */
   inline bool ready() const;

   /**
     * Get number of hosts in RoundTripTimePinger.
     *
     * @return Number of hosts.
     */
   inline cardinal getHosts();


   /**
     * Get constant alpha: RTT = alpha * oldValue + (1 - alpha) * newValue.
     *
     * @return alpha.
     */
   inline double getAlpha();

   /**
     * Set constant alpha: RTT = alpha * oldValue + (1 - alpha) * newValue.
     *
     * @param alpha Alpha.
     */
   inline void setAlpha(const double alpha);


   /**
     * Get maximum delay between two pings in microseconds.
     *
     * @return Delay in microseconds.
     */
   inline card64 getMaxPingDelay();

   /**
     * Set maximum delay between two pings in microseconds.
     *
     * @param delay Delay in microseconds.
     */
   inline void setMaxPingDelay(const card64 delay);


   /**
     * Get round trip time for given host and traffic class.
     *
     * @param trafficClass Traffic class.
     * @return Round trip time in microseconds; -1 for hosts not in list or unreachable.
     */
   cardinal getRoundTripTime(const InternetAddress& address,
                             const card8            trafficClass = 0x00);


   // ====== Adding/removing hosts ==========================================
   /**
     * Add host to RoundTripTimePinger list.
     *
     * @param address Host address.
     * @param trafficClass Traffic class.
     * @return true, if host has been added; false otherwise (duplicate).
     */
   bool addHost(const InternetAddress& address,
                const card8            trafficClass = 0x00);

   /**
     * Remove host from RoundTripTimePinger list.
     *
     * @param address Host address.
     * @param trafficClass Traffic class.
     */
   void removeHost(const InternetAddress& address,
                   const card8            trafficClass = 0x00);

   // ====== GNUplot output generator =======================================
   /**
     * Activate logger. Very important: Logging will be deactivated by addHost()
     * and removeHost() calls!
     *
     * @param scriptStream Script output stream.
     * @param dataStream Data output stream.
     * @param dataName Data file name (for GNUplot's plot command).
     */
   void activateLogger(std::ostream* scriptStream,
                       std::ostream* dataStream,
                       const char*   dataName);

   /**
     * Deactivate logger.
     */
   void deactivateLogger();

   /**
     * Check, if logger is running.
     */
   inline bool isLogging() const;

   /**
     * Write GNUplot header. Very important: This header will become invalid
     * when calling addHost() or removeHost()!
     *
     * @param os Output stream.
     * @param dataName Name of data file.
     * @param lineStyle First GNUplot line style or 0 for using GNUplot's defaults.
     */
   void writeGPHeader(std::ostream&       os,
                      const char*    dataName,
                      const cardinal lineStyle = 1);

   /**
     * Write GNUplot data line.
     *
     * @param os Output stream.
     */
   void writeGPData(std::ostream& os);


   // ====== Output operator ================================================
   /**
     * Friend output operator.
     */
   friend std::ostream& operator<<(std::ostream& os, RoundTripTimePinger& pinger);


   // ====== Constants ======================================================
   /**
     * Maximum round trip time in microseconds.
     */
   static const cardinal MaxRoundTripTime = 180000000;

   /**
     * Unreachable Factor:
     * Assume current round trip time to be diff = now - host.LastEchoTimeStamp, if
     * diff > MinUnreachableAsumption (for OS delay) or
     * diff > UnreachableFactor * MaxRawRoundTripTime (for real network delay).
     */
   static constexpr double UnreachableFactor = 2.0;

   /**
     * MinUnreachableAsumption:
     * Assume current round trip time to be diff = now - host.LastEchoTimeStamp, if
     * diff > MinUnreachableAsumption (for OS delay) or
     * diff > UnreachableFactor * MaxRawRoundTripTime (for real network delay).
     */
   static const card64 MinUnreachableAsumption = 2500000;


   // ====== Private data ===================================================
   private:
   void timerEvent();
   void calculateRoundTripTime(const InternetAddress& address,
                               const card8            trafficClass,
                               const card64           sendTime,
                               const card64           arrivalTime);
   card16 calculateChecksum(const card16*  addr,
                            const cardinal length,
                            card16         csum);
   card64 sendPing4(const InternetAddress& destination,
                    const card8            trafficClass,
                    const card16           sequenceNumber);
   card64 sendPing6(const InternetAddress& destination,
                    const card8            trafficClass,
                    const card16           sequenceNumber);
   bool receiveEcho4();
   bool receiveEcho6();
   void checkUnreachable(PingerHost& host);


   struct Ping4Packet
   {
      icmp   Header;
      card64 TimeStamp;
   };

   struct Ping6Packet
   {
      icmp6_hdr Header;
      card64    TimeStamp;
   };


   Socket*                   Ping4Socket;
   Socket*                   Ping6Socket;
   double                    RoundTripTimeAlpha;
   std::multiset<PingerHost> HostSet;
   card64                    GPHeaderTimeStamp;
   bool                      Ready;
   bool                      Logger;
   std::ostream*             LoggerScriptStream;
   std::ostream*             LoggerDataStream;
   card64                    MaxPingDelay;
   Randomizer                Random;
};


#include "roundtriptimepinger.icc"


#endif
