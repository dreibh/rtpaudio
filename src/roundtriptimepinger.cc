// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Round Trip Time Pinger                                           ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2020 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "roundtriptimepinger.h"
#include "tdsocket.h"
#include "internetaddress.h"
#include "breakdetector.h"
#include "timedthread.h"
#include "rtppacket.h"
#include "strings.h"
#include "trafficclassvalues.h"


// Debug mode: Print raw round trip times
// #define DEBUG



#define ICMP_FILTER 1

struct icmp_filter {
   card32 data;
};


// ###### Constructor #######################################################
RoundTripTimePinger::RoundTripTimePinger(Socket*      ping4socket,
                                         Socket*      ping6socket,
                                         const card64 delay)
   : TimedThread(delay)
{
   // ====== Initialize =====================================================
   Ready              = false;
   Logger             = false;
   RoundTripTimeAlpha = 7.0 / 8.0;
   Ping4Socket        = ping4socket;
   Ping6Socket        = ping6socket;
   MaxPingDelay       = delay;
   setTimerCorrection(0);
   setFastStart(false);

   // ====== Set ICMPv4 filter ==============================================
   if(Ping4Socket != NULL) {
      struct icmp_filter filter;
      filter.data = ~(1 << ICMP_ECHOREPLY);
      if(Ping4Socket->setSocketOption(SOL_SOCKET, // SOL_RAW,
                                      ICMP_FILTER,
                                      (void*)&filter, sizeof(filter)) == -1) {
         std::cerr << "ERROR: Unable to set ICMPv4 filter!" << std::endl;
         return;
      }

      // ====== Set non-blocking mode =======================================
      Ping4Socket->setBlockingMode(false);

      // ====== Send a ping to IPv4 localhost ===============================
      // Send one ping to IPv4 localhost. The result is, that the first real ping
      // over this socket will have a normal RTT. Otherwise, the RTT of the
      // first ping will have a much longer RTT, due to kernel initialization.
      if(sendPing4(InternetAddress("127.0.0.1:0"),0x00,0) == 0) {
         std::cerr << "ERROR: Unable to send over ICMPv4!" << std::endl;
         return;
      }
   }

   // ====== Set ICMPv6 filter and checksum option ==========================
   if(Ping6Socket != NULL) {
      struct icmp6_filter filter;
      ICMP6_FILTER_SETBLOCKALL(&filter);
      ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY,&filter);
      if(Ping6Socket->setSocketOption(SOL_SOCKET, // SOL_ICMPV6,
                                      ICMP6_FILTER,
                                      (void*)&filter, sizeof(filter)) == -1) {
         std::cerr << "ERROR: Unable to set ICMPv6 filter!" << std::endl;
         return;
      }
      int opt = 2;  // Position of checksum field in ICMPv6 header!
      if(Ping6Socket->setSocketOption(SOL_SOCKET, // SOL_RAW,
                                      IPV6_CHECKSUM,
                                      &opt,sizeof(opt)) == -1) {
         std::cerr << "ERROR: Unable to set IPv6 checksum option!" << std::endl;
         return;
      }

      // ====== Set non-blocking mode =======================================
      Ping6Socket->setBlockingMode(false);

      // ====== Send a ping to IPv6 localhost ===============================
      // Send one ping to IPv4 localhost. The result is, that the first real ping
      // over this socket will have a normal RTT. Otherwise, the RTT of the
      // first ping will have a much longer RTT, due to kernel initialization.
      if(sendPing6(InternetAddress("::1:0"),0x00,0) == 0) {
         std::cerr << "ERROR: Unable to send over ICMPv6!" << std::endl;
         return;
      }
   }

   Ready = true;
}


// ###### Destructor ########################################################
RoundTripTimePinger::~RoundTripTimePinger()
{
   stop();
}


// ###### Add host ##########################################################
bool RoundTripTimePinger::addHost(const InternetAddress& address,
                                  const card8            trafficClass)
{
   if(!address.isValid()) {
      std::cerr << "WARNING: RoundTripTimePinger::addHost() - Invalid address" << std::endl;
      return(false);
   }
   bool isIPv6 = address.isIPv6();
   if((isIPv6) && (Ping6Socket == NULL)) {
      std::cerr << "WARNING: RoundTripTimePinger::addHost() - Address is IPv6, but no IPv6 socket!" << std::endl;
      return(false);
   }
   else if((!isIPv6) && (Ping4Socket == NULL)) {
      std::cerr << "WARNING: RoundTripTimePinger::addHost() - Address is IPv4, but no IPv4 socket!" << std::endl;
      return(false);
   }

   PingerHost host;
   host.Address             = address;
   host.Address.setPort(0);
   host.AddressString       = host.Address.getAddressString();
   host.TrafficClass        = trafficClass;
   host.RoundTripTime       = (cardinal)-1;
   host.MaxRawRoundTripTime = 0;
   host.SeqNum              = 1;
   host.IsIPv6              = isIPv6;
   host.LastPingTimeStamp   = 0;
   host.LastEchoTimeStamp   = 0;
   host.UserCount           = 1;

   synchronized();
   bool added = false;
   std::multiset<PingerHost>::iterator found = find(HostSet.begin(),HostSet.end(),host);
   if(found == HostSet.end()) {
      HostSet.insert(host);
      added = true;
   }
   else {
      PingerHost& host = (PingerHost&)*found;
      host.UserCount++;
   }
   deactivateLogger();
   unsynchronized();

   return(added);
}


// ###### Remove host #######################################################
void RoundTripTimePinger::removeHost(const InternetAddress& address,
                                     const card8            trafficClass)
{
   synchronized();

   PingerHost findHost;
   findHost.Address      = address;
   findHost.TrafficClass = trafficClass;

   std::multiset<PingerHost>::iterator found = find(HostSet.begin(),HostSet.end(),findHost);
   if(found != HostSet.end()) {
      PingerHost& host = (PingerHost&)*found;
      host.UserCount--;
      if(host.UserCount <= 0) {
         HostSet.erase(found);
      }
      deactivateLogger();
   }

   unsynchronized();
}


// ###### Get round trip time ###############################################
cardinal RoundTripTimePinger::getRoundTripTime(
                                 const InternetAddress& address,
                                 const card8            trafficClass)
{
   PingerHost findHost;
   findHost.Address      = address;
   findHost.TrafficClass = trafficClass;

   synchronized();
   cardinal rtt = (cardinal)-1;
   std::multiset<PingerHost>::iterator found = find(HostSet.begin(),HostSet.end(),findHost);
   if(found != HostSet.end()) {
      rtt = found->RoundTripTime;
   }
   unsynchronized();

   return(rtt);
}


// ###### Calculate round trip time #########################################
void RoundTripTimePinger::calculateRoundTripTime(
                             const InternetAddress& address,
                             const card8            trafficClass,
                             const card64           sendTime,
                             const card64           arrivalTime)
{
   // ====== Calculate round trip time ======================================
   card64 roundTripTime = arrivalTime - sendTime;
   if(roundTripTime > MaxRoundTripTime)
      roundTripTime = MaxRoundTripTime;

   // ====== Find PingerHost in list ========================================
   PingerHost findHost;
   findHost.Address      = address;
   findHost.TrafficClass = trafficClass;

   synchronized();
   std::multiset<PingerHost>::iterator found = find(HostSet.begin(),HostSet.end(),findHost);
   if(found != HostSet.end()) {
      PingerHost& host = (PingerHost&)*found;

      // ====== Check for outdated reply ====================================
      if((host.LastEchoTimeStamp < arrivalTime) && (sendTime <= getMicroTime())) {

#ifdef DEBUG
   cout << "RTT for " << address << " is " << roundTripTime << "." << std::endl;
#endif

         // ====== Update PingerHost data ===================================
         host.LastEchoTimeStamp   = arrivalTime;
         host.MaxRawRoundTripTime = std::max(host.MaxRawRoundTripTime,(cardinal)roundTripTime);

         // Set RoundTripTime to *calculated* value, if there is no valid
         // round trip time set (unreachable or update for the first time)
         if(host.RoundTripTime >= MaxRoundTripTime) {
            host.RoundTripTime = roundTripTime;
         }

         // Else, use update calculation RTT = alpha*oldRTT + (1 - alpha)*newRTT
         else {
            host.RoundTripTime = (cardinal)
               (RoundTripTimeAlpha * (double)host.RoundTripTime +
               (1.0 - RoundTripTimeAlpha) * (double)roundTripTime);
         }
      }
      else {
         std::cerr << "Outdated echo received!" << std::endl;
      }
   }
   unsynchronized();
}


// ###### Calculate ICMP checksum ###########################################
card16 RoundTripTimePinger::calculateChecksum(const card16*  addr,
                                              const cardinal length,
                                              card16         csum)
{
   cardinal      nleft = length;
   const card16* w     = addr;
   cardinal      sum   = csum;
   card16        answer;

   /*
    * Our algorithm is simple, using a 32 bit accumulator (sum),
    * we add sequential 16 bit words to it, and at the end, fold
    * back all the carry bits from the top 16 bits into the lower
    * 16 bits.
    */
   while (nleft > 1)  {
      sum += *w++;
      nleft -= 2;
   }

   /* mop up an odd byte, if necessary */
   if (nleft == 1)
      sum += htons(*(u_char *)w << 8);

   /*
    * add back carry outs from top 16 bits to low 16 bits
    */
   sum = (sum >> 16) + (sum & 0xffff);  /* add hi 16 to low 16 */
   sum += (sum >> 16);                  /* add carry */
   answer = ~sum;                       /* truncate to 16 bits */
   return (answer);
}



// ###### Sebd ICMPv4 ping ##################################################
card64 RoundTripTimePinger::sendPing4(const InternetAddress& destination,
                                      const card8            trafficClass,
                                      const card16           sequenceNumber)
{
   Ping4Packet packet;

   packet.Header.icmp_type  = ICMP_ECHO;
   packet.Header.icmp_code  = 0;
   packet.Header.icmp_cksum = 0;
   packet.Header.icmp_seq   = sequenceNumber;
   packet.Header.icmp_id    = 0x3300 | (card16)trafficClass;

   Thread::yield();
   packet.TimeStamp         = getMicroTime();
   packet.Header.icmp_cksum = calculateChecksum((card16*)&packet,sizeof(packet),0);
   ssize_t sent = Ping4Socket->sendTo((void*)&packet,sizeof(packet),0,
                                      destination,trafficClass);
   if(sent == sizeof(packet)) {
      return(packet.TimeStamp);
   }
   else {
      return(0);
   }
}


// ###### Sebd ICMPv6 ping ##################################################
card64 RoundTripTimePinger::sendPing6(const InternetAddress& destination,
                                      const card8            trafficClass,
                                      const card16           sequenceNumber)
{
   Ping6Packet packet;

   packet.Header.icmp6_type  = ICMP6_ECHO_REQUEST;
   packet.Header.icmp6_code  = 0;
   packet.Header.icmp6_cksum = 0;
   packet.Header.icmp6_seq   = sequenceNumber;
   packet.Header.icmp6_id    = 0x3300 | (card16)trafficClass;

   Thread::yield();
   packet.TimeStamp = getMicroTime();
   ssize_t sent = Ping6Socket->sendTo((void*)&packet,sizeof(packet),0,
                                      destination,trafficClass);

   if(sent == sizeof(packet)) {
      return(packet.TimeStamp);
   }
   else {
      return(0);
   }
}


// ###### Receive ICMPv4 echo ###############################################
bool RoundTripTimePinger::receiveEcho4()
{
   char buffer[sizeof(Ping4Packet) + IPv4HeaderSize];

   // ====== Get packet =====================================================
   InternetAddress source;
   int     flags  = 0;
   ssize_t length = Ping4Socket->receiveFrom(&buffer,sizeof(buffer),source,flags);
   if(length == sizeof(buffer)) {
      source.setPort(0);
      const Ping4Packet* packet = (Ping4Packet*)&buffer[IPv4HeaderSize];

      // ====== Verify packet ===============================================
      const card16 oldChecksum = packet->Header.icmp_cksum;
      const card16 newChecksum = calculateChecksum((card16*)packet,sizeof(Ping4Packet),-(packet->Header.icmp_cksum + 1));
      if((newChecksum == oldChecksum)   &&
         (packet->Header.icmp_type == ICMP_ECHOREPLY) &&
         ((packet->Header.icmp_id & 0xff00) == 0x3300)) {

         // ====== Get arrival time =========================================
#if (SYSTEM == OS_Linux)
         timeval socketTimeStamp;        
         if(Ping4Socket->ioctl(
#ifdef SIOCGSTAMP
            SIOCGSTAMP,
#else
            SIOCGSTAMP_OLD,
#endif            
            (void*)&socketTimeStamp) < 0) {
            return(false);
         }
         const card64 arrivalTime = ((card64)socketTimeStamp.tv_sec * (card64)1000000) +
                                       (card64)socketTimeStamp.tv_usec;
#else
         const card64 arrivalTime = getMicroTime();
#endif

         // ====== Calculate RoundTripTime ==================================
         const card64 sendTime = packet->TimeStamp;
         calculateRoundTripTime(source,(packet->Header.icmp_id & 0x00ff),
                                sendTime,arrivalTime);
         return(true);
      }
   }
   return(false);
}


// ###### Receive ICMPv6 echo ###############################################
bool RoundTripTimePinger::receiveEcho6()
{
   Ping6Packet packet;

   // ====== Get packet =====================================================
   InternetAddress source;
   int     flags  = 0;
   ssize_t length = Ping6Socket->receiveFrom(&packet,sizeof(packet),source,flags);
   if(length == sizeof(packet)) {
      source.setPort(0);

      // ====== Verify packet ===============================================
      if((packet.Header.icmp6_type == ICMP6_ECHO_REPLY) &&
         ((packet.Header.icmp6_id & 0xff00) == 0x3300)) {

         // ====== Get arrival time =========================================
#if (SYSTEM == OS_Linux)
         timeval socketTimeStamp;
         if(Ping6Socket->ioctl(
#ifdef SIOCGSTAMP
            SIOCGSTAMP,
#else
            SIOCGSTAMP_OLD,
#endif
            (void*)&socketTimeStamp) < 0) {
            return(false);
         }
         const card64 arrivalTime = ((card64)socketTimeStamp.tv_sec * (card64)1000000) +
                                       (card64)socketTimeStamp.tv_usec;
#else
         const card64 arrivalTime = getMicroTime();
#endif

         // ====== Calculate RoundTripTime ==================================
         const card64 sendTime = packet.TimeStamp;
         calculateRoundTripTime(source,(packet.Header.icmp6_id & 0x00ff),
                                sendTime,arrivalTime);
         return(true);
      }
   }
   return(false);
}


// ###### Check host for being unreachable ##################################
void RoundTripTimePinger::checkUnreachable(PingerHost& host)
{
   if((host.RoundTripTime >= MaxRoundTripTime) || (host.LastEchoTimeStamp == 0)) {
      // Host is unreachable.
   }
   else {
      // Assume current round trip time to be diff = now - host.LastEchoTimeStamp, if
      // diff > MinUnreachableAsumption (for OS delay) or
      // diff > UnreachableFactor * MaxRawRoundTripTime (for real network delay).
      const card64 now  = getMicroTime();
      const card64 diff = now - host.LastEchoTimeStamp;

      if((diff > MinUnreachableAsumption) &&
         (diff > (card64)((double)UnreachableFactor * (double)host.MaxRawRoundTripTime))) {

#ifdef DEBUG
   cout << "Assumed RTT for unreachable " << host.Address
        << " is " << diff << "." << std::endl;
#endif

         host.RoundTripTime = (cardinal)
            (RoundTripTimeAlpha * (double)host.RoundTripTime +
            (1.0 - RoundTripTimeAlpha) * (double)diff);
         if(host.RoundTripTime > MaxRoundTripTime)
            host.RoundTripTime = MaxRoundTripTime;
      }
   }
}


// ###### timerEvent() implementation #######################################
void RoundTripTimePinger::timerEvent()
{
   synchronized();

   std::multiset<PingerHost>::iterator hostIterator = HostSet.begin();
   while(hostIterator != HostSet.end()) {
      PingerHost& host = (PingerHost&)*hostIterator;

      // ====== Send pings ==================================================
      if(host.IsIPv6) {
         const card64 timeStamp = sendPing6(host.Address,host.TrafficClass,host.SeqNum++);
         if(timeStamp == 0) {
            std::cerr << "WARNING: Ping6 to " << host.Address << " failed!" << std::endl;
         }
         else {
            host.LastPingTimeStamp = timeStamp;
         }
      }
      else {
         const card64 timeStamp = sendPing4(host.Address,host.TrafficClass,host.SeqNum++);
         if(timeStamp == 0) {
            std::cerr << "WARNING: Ping4 to " << host.Address << " failed!" << std::endl;
         }
         else {
            host.LastPingTimeStamp = timeStamp;
         }
      }

      // ====== Receive echos ===============================================
      bool received4 = false;
      bool received6 = false;
      do {
         if(Ping6Socket != NULL) {
            received6 = receiveEcho6();
         }
         if(Ping4Socket != NULL) {
            received4 = receiveEcho4();
         }
      } while((received6 == true) || (received4 == true));


      // ====== Check, if host is unreachable ===============================
      checkUnreachable(host);

      hostIterator++;
   }

   // ====== Do logging =====================================================
   if(Logger) {
      writeGPData(*LoggerDataStream);
   }

   const card64 newInterval = Random.random64() % MaxPingDelay;
   setInterval(newInterval + 1);

   unsynchronized();
}


// ###### Activate logger ###################################################
void RoundTripTimePinger::activateLogger(std::ostream* scriptStream,
                                         std::ostream* dataStream,
                                         const char*   dataName)
{
   synchronized();
   deactivateLogger();
   LoggerScriptStream = scriptStream;
   LoggerDataStream   = dataStream;
   Logger = true;
   unsynchronized();
   writeGPHeader(*LoggerScriptStream,dataName);
}


// ###### Deactivate logger #################################################
void RoundTripTimePinger::deactivateLogger()
{
   synchronized();
   if(Logger == true) {
      LoggerScriptStream = NULL;
      LoggerDataStream   = NULL;
      Logger = false;
   }
   unsynchronized();
}


// ###### Write GNUplot header ##############################################
void RoundTripTimePinger::writeGPHeader(std::ostream&  os,
                                        const char*    dataName,
                                        const cardinal lineStyle)
{
   GPHeaderTimeStamp = getMicroTime();

   synchronized();
   std::multiset<PingerHost>::iterator hostIterator = HostSet.begin();
   cardinal number = 0;
   while(hostIterator != HostSet.end()) {
      PingerHost host = *hostIterator;
      char str[32];
      const char* tclass =
         TrafficClassValues::getNameForTrafficClass(host.TrafficClass);
      if(tclass == NULL) {
         snprintf((char*)&str,sizeof(str),"$%02x",host.TrafficClass);
         tclass = (const char*)&str;
      }
      if(number == 0) {
         os << "plot \"" << dataName << "\" using "
            << (2* number + 1) << ":" << (2*number + 2)
            << " smooth unique title \"" << host.AddressString << " / "
            << tclass << "\" with lines";
         if(lineStyle != 0) {
            os << " ls " << number + lineStyle << " ";
         }
      }
      else {
         os << ", \"" << dataName << "\" using "
            << (2* number + 1) << ":" << (2*number + 2)
            << " smooth unique title \"" << host.AddressString << " / "
            << tclass << "\" with lines";
            if(lineStyle != 0) {
               os << " ls " << number + lineStyle << " ";
            }
      }
      number++;
      hostIterator++;
   }
   unsynchronized();
   os << std::endl;
}


// ###### Write GNUplot data line ###########################################
void RoundTripTimePinger::writeGPData(std::ostream& os)
{
   synchronized();
   const card64 now = getMicroTime();
   std::multiset<PingerHost>::iterator hostIterator = HostSet.begin();
   while(hostIterator != HostSet.end()) {
      PingerHost host = *hostIterator;
      double rtt = (host.RoundTripTime) / 1000.0;
      if(rtt >= 5000.0)
         os << "0 0 ";
      else
         os << (now - GPHeaderTimeStamp) / 1000000.0 << " " << rtt << " ";
      hostIterator++;
   }
   os << std::endl;
   unsynchronized();
}


// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, RoundTripTimePinger& pinger)
{
   pinger.synchronized();

   cardinal number = 1;
   std::multiset<PingerHost>::iterator hostIterator = pinger.HostSet.begin();
   while(hostIterator != pinger.HostSet.end()) {
      PingerHost host = *hostIterator;
      String hostName = host.AddressString;

      char str[256];
      char tcString[32];
      const char* tc = TrafficClassValues::getNameForTrafficClass(host.TrafficClass);
      if(tc != NULL) {
         strcpy((char*)&tcString,tc);
      }
      else {
         snprintf((char*)&tcString,sizeof(tcString),"$%02x",host.TrafficClass);
      }

      snprintf((char*)&str,sizeof(str),"#%02d:  %4s  %8d  %-32s",
                          (int)number,
                          tcString,
                          (int)host.RoundTripTime,
                          hostName.getData());
      os << str << std::endl;

      number++;
      hostIterator++;
   }

   pinger.unsynchronized();
   return(os);
}
