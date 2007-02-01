// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Traffic Shaper                                                   ####
// ####                                                                  ####
// #### Version 1.50  --  August 01, 2001                                ####
// ####                                                                  ####
// ####            Copyright (C) 1999-2001 by Thomas Dreibholz           ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@exp-math.uni-essen.de                           ####
// ####    WWW:   http://www.exp-math.uni-essen.de/~dreibh/rtpaudio      ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software; you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License      ####
// #### as published by the Free Software Foundation; either version 2   ####
// #### of the License, or (at your option) any later version.           ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// ##########################################################################


#include "tdsystem.h"
#include "trafficshaper.h"
#include "tools.h"


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
   deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
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
      cerr << "WARNING: TrafficShaper::write() - Peer address is null!" << endl;
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
      cerr << "WARNING: TrafficShaper::send() - Peer address is null!" << endl;
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
      cerr << "ERROR: TrafficShaper::addPacket() - Bandwidth is zero!" << endl;
      cerr << bytes << " to " << destination << endl;
      exit(1);
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
      cerr << "WARNING: TrafficShaper::addPacket() - Delay limit exceeded!" << endl;
      cerr << "         Delay is " << delay << ", limit is " << BufferDelay << "." << endl;
      cerr << "Buffer contents:" << endl;
      deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
      while(iterator != Queue.end()) {
         const TrafficShaperPacket& packet = *iterator;
         cerr << "   => " << packet.SendTimeStamp << ", " << packet.PayloadSize << endl;
         iterator++;
      }
      cerr << "   Tried to add " << bytes << " bytes, "
              "required: " << time << " [s]" << "  "
              "avaiable: " << (delay - BufferDelay) << " [s]." << endl;
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
   deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
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
         cerr << "WARNING: TrafficShaper::refreshBuffer() - Flush necessary!"
              << endl;
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
   sort(Queue.begin(),Queue.end());

   // ====== Check all packets for reached send time ========================
   deque<TrafficShaperPacket>::iterator iterator = Queue.begin();
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
               cerr << "WARNING: TrafficShaper::sendAll() - Invalid TSC command?!" << endl;
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
   vector<TrafficShaper*>::iterator iterator = ShaperSet.begin();
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

   vector<TrafficShaper*>::iterator iterator = ShaperSet.begin();
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
   vector<TrafficShaper*>::iterator iterator = ShaperSet.begin();
   while(iterator != ShaperSet.end()) {
      (*iterator)->sendAll();
      iterator++;
   }
   unsynchronized();
}
