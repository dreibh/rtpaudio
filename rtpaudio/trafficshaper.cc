// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Traffic Shaper                                                   ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2015 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "trafficshaper.h"
#include "tools.h"
#include <algorithm>


// Print information
// #define PRINT_EXCEEDS


// ###### TrafficShaper singleton object ####################################
TrafficShaperSingleton TrafficShaper::Singleton;


// ###### Constructor #######################################################
TrafficShaper::TrafficShaper()
{
   init(NULL);
}


// ###### Constructor #######################################################
TrafficShaper::TrafficShaper(Socket* socket)
{
   init(socket);
}


// ###### Initialize ########################################################
void TrafficShaper::init(Socket* socket)
{
   SenderSocket  = socket;
   SendTimeStamp = 0;
   Bandwidth     = 0;
   BufferDelay   = 50000.0;
   LastSeqNum    = (cardinal)-1;
   Singleton.addTrafficShaper(this);
}


// ###### Destructor ########################################################
TrafficShaper::~TrafficShaper()
{
   Singleton.removeTrafficShaper(this);
   flush();
}


// ###### Flush buffer ######################################################
void TrafficShaper::flush()
{
   synchronized();

   // ====== Flush buffer ===================================================
   std::deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
   while(iterator != Queue.end()) {
      const TrafficShaperPacket& packet = *iterator;
      delete packet.Data;
      Queue.erase(iterator);
      iterator = Queue.begin();
   }
   SendTimeStamp = getMicroTime();

   unsynchronized();
}



// ###### Send a packet #####################################################
ssize_t TrafficShaper::write(const void*    buffer,
                             const size_t   length,
                             const cardinal seqNum)
{
   InternetFlow destination;
   SenderSocket->getPeerAddress(destination);
   if(destination.isNull()) {
      std::cerr << "WARNING: TrafficShaper::write() - Peer address is null!" << std::endl;
      return(0);
   }
   return(addPacket(buffer,length,seqNum,destination,0,TSC_Write));
}


// ###### Send a packet #####################################################
ssize_t TrafficShaper::send(const void*    buffer,
                            const size_t   length,
                            const cardinal seqNum,
                            const cardinal flags,
                            const card8    trafficClass)
{
   InternetFlow destination;
   SenderSocket->getPeerAddress(destination);
   if(destination.isNull()) {
      std::cerr << "WARNING: TrafficShaper::send() - Peer address is null!" << std::endl;
      return(0);
   }
   if(trafficClass != 0x00) {
      destination.setTrafficClass(trafficClass);
   }
   else {
      destination.setTrafficClass(SenderSocket->getSendTrafficClass());
   }
   return(addPacket(buffer,length,seqNum,destination,flags,TSC_Send));
}


// ###### Send a packet #####################################################
ssize_t TrafficShaper::sendTo(const void*         buffer,
                              const size_t        length,
                              const cardinal      seqNum,
                              const cardinal      flags,
                              const InternetFlow& receiver,
                              const card8         trafficClass)
{
   InternetFlow destination = receiver;
   if(trafficClass != 0x00) {
      destination.setTrafficClass(trafficClass);
   }
   return(addPacket(buffer,length,seqNum,destination,flags,TSC_SendTo));
}


// ###### Write #############################################################
ssize_t TrafficShaper::addPacket(const void*    data,
                                 const cardinal bytes,
                                 const cardinal seqNum,
                                 InternetFlow&  destination,
                                 const cardinal flags,
                                 const cardinal command)
{
   // ====== Check for error ================================================
   if((bytes > 0) && (Bandwidth == 0)) {
      std::cerr << "ERROR: TrafficShaper::addPacket() - Bandwidth is zero!" << std::endl;
      std::cerr << bytes << " to " << destination << std::endl;
      ::abort();
   }

   // ====== Create new packet ==============================================
   TrafficShaperPacket packet;
   packet.Data = new char[bytes];
   if(packet.Data == NULL) {
      return(-1);
   }
   const card64 now = getMicroTime();
   if(SendTimeStamp < now) {
      SendTimeStamp = now;
   }
   packet.SendTimeStamp = SendTimeStamp;
   packet.HeaderSize    = destination.isIPv6() ? (40 + 8) : (20 + 8);
   packet.PayloadSize   = bytes;
   packet.Destination   = destination;
   packet.Flags         = flags;
   packet.SeqNum        = seqNum;
   packet.Command       = command;
   memcpy(packet.Data,data,bytes);


   // ====== Calculate transmission time for given bandwidth ================
   const card64 time = (card64)floor(
      1000000.0 * (double)(packet.HeaderSize + packet.PayloadSize) /
                     (double)Bandwidth);
   synchronized();

   // ====== Check for exceeded limits ======================================
   const card64 delay = packet.SendTimeStamp - now;
   if(delay > (card64)BufferDelay) {
#ifdef PRINT_EXCEEDS
      // ====== Print information ===========================================
      std::cerr << "WARNING: TrafficShaper::addPacket() - Delay limit exceeded!" << std::endl;
      std::cerr << "         Delay is " << delay << ", limit is " << BufferDelay << "." << std::endl;
      std::cerr << "Buffer contents:" << std::endl;
      std::deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
      while(iterator != Queue.end()) {
         const TrafficShaperPacket& packet = *iterator;
         std::cerr << "   => " << packet.SendTimeStamp << ", " << packet.PayloadSize << std::endl;
         iterator++;
      }
      std::cerr << "   Tried to add " << bytes << " bytes, "
              "required: " << time << " [s]" << "  "
              "avaiable: " << (delay - BufferDelay) << " [s]." << std::endl;
#endif

      // ====== Flush buffer ================================================
      flush();
      unsynchronized();

      delete packet.Data;
      return(-1);
   }

   // ====== Add packet to buffer ===========================================
   Queue.push_back(packet);
   SendTimeStamp += time;

   unsynchronized();
   return(bytes);
}


// ###### Refresh buffer ####################################################
bool TrafficShaper::refreshBuffer(const card8 trafficClass,
                                  const bool  doRemapping)
{
   bool flushed = false;
   synchronized();

   // ====== Reset time stamp ===============================================
   const card64 now = getMicroTime();
   SendTimeStamp = now;


   // ====== Refresh buffer =================================================
   std::deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
   while(iterator != Queue.end()) {

      // ====== Get packet data =============================================
      TrafficShaperPacket& packet = *iterator;
      if(doRemapping) {
         packet.Destination.setTrafficClass(trafficClass);
      }
      packet.SendTimeStamp = SendTimeStamp;

      // ====== Update transmission time for given bandwidth ================
      const card64 time = (card64)floor(
         1000000.0 * (double)(packet.HeaderSize + packet.PayloadSize) /
                        (double)Bandwidth);

      // ====== Check for exceeded limits ======================================
      const card64 delay = packet.SendTimeStamp - now;
      if(delay > (card64)BufferDelay) {
#ifdef PRINT_EXCEEDS
         std::cerr << "WARNING: TrafficShaper::refreshBuffer() - Flush necessary!"
              << std::endl;
#endif
         flush();
         flushed = true;
         break;
      }

      // ====== Add packet to buffer ===========================================
      SendTimeStamp += time;

      iterator++;
   }

   unsynchronized();
   return(flushed);
}


// ###### timerEvent() implementation #######################################
void TrafficShaper::sendAll()
{
   synchronized();

   // ====== Sort packets (necessary due to different classes!) =============
   std::sort(Queue.begin(),Queue.end());

   // ====== Check all packets for reached send time ========================
   std::deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
   while(iterator != Queue.end()) {
      const TrafficShaperPacket& packet = *iterator;

      // ====== Send packet, if send time is reached ========================
      if(packet.SendTimeStamp <= getMicroTime()) {

         switch(packet.Command) {
            case TSC_Write:
               SenderSocket->write(packet.Data, packet.PayloadSize);
             break;
            case TSC_Send:
               SenderSocket->send(packet.Data, packet.PayloadSize,
                                  packet.Flags,
                                  packet.Destination.getTrafficClass());
             break;
            case TSC_SendTo:
               SenderSocket->sendTo(packet.Data, packet.PayloadSize,
                                    packet.Flags, packet.Destination,
                                    packet.Destination.getTrafficClass());
             break;
            default:
               std::cerr << "WARNING: TrafficShaper::sendAll() - Invalid TSC command?!" << std::endl;
             break;
         }
         if(packet.SeqNum != (cardinal)-1) {
            LastSeqNum = packet.SeqNum;
         }

         delete packet.Data;
         Queue.pop_front();
      }
      else {
         break;
      }

      iterator = Queue.begin();
   }

   unsynchronized();
}


// ###### Constructor #######################################################
TrafficShaperSingleton::TrafficShaperSingleton()
   : TimedThread(1000000 / 100,"TrafficShaperSingleton")
{
   setTimerCorrection(false);
   UserCount = 0;
}


// ###### Destructor ########################################################
TrafficShaperSingleton::~TrafficShaperSingleton()
{
   stop();
   std::vector<TrafficShaper*>::iterator iterator = ShaperSet.begin();
   while(iterator != ShaperSet.end()) {
      ShaperSet.erase(iterator);
      iterator = ShaperSet.begin();
   }
}


// ###### Add socket ########################################################
void TrafficShaperSingleton::addTrafficShaper(TrafficShaper* ts)
{
   synchronized();
   ShaperSet.push_back(ts);
   unsynchronized();

   UserCount++;
   if(UserCount == 1) {
      start();
   }
}


// ###### Remove socket #####################################################
void TrafficShaperSingleton::removeTrafficShaper(TrafficShaper* ts)
{
   synchronized();

   std::vector<TrafficShaper*>::iterator iterator = ShaperSet.begin();
   while(iterator != ShaperSet.end()) {
      if(*iterator == ts) {
         ShaperSet.erase(iterator);
         UserCount--;
         break;
      }
      iterator++;
   }

   unsynchronized();

   if(UserCount <= 0) {
      stop();
   }
}


// ###### Main loop #########################################################
void TrafficShaperSingleton::timerEvent()
{
   synchronized();
   std::vector<TrafficShaper*>::iterator iterator = ShaperSet.begin();
   while(iterator != ShaperSet.end()) {
      (*iterator)->sendAll();
      iterator++;
   }
   unsynchronized();
}
