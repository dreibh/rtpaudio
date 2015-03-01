// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Abstract Server Implementation                              ####
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
#include "string.h"
#include "rtcpabstractserver.h"


// ###### Constructor #######################################################
RTCPAbstractServer::RTCPAbstractServer()
   : TimedThread(1000000,"RTCPAbstractServer")
{
   DefaultTimeout = 8000000;
   setTimerCorrection(0);
   setFastStart(false);
}


// ###### Thread's stop() reimplementation ##################################
void* RTCPAbstractServer::stop()
{
   void* result = TimedThread::stop();

   synchronized();
   while(ClientSet.begin() != ClientSet.end()) {
      std::multimap<const cardinal,Client*>::iterator clientIterator = ClientSet.begin();
      Client* client = clientIterator->second;

      // Note: The client will be removed and deleted here. The iterator
      //       becomes invalid!
      receivedBye(client->ClientAddress, client->SSRC, DeleteReason_Shutdown);
   }
   unsynchronized();

   return(result);
}


// ###### Destructor ########################################################
RTCPAbstractServer::~RTCPAbstractServer()
{
   stop();
}


// ###### Out of memory warning #############################################
void RTCPAbstractServer::outOfMemoryWarning()
{
   std::cerr << "WARNING: RTCPAbstractServer - Out of memory!" << std::endl;
}


// ###### Handle received Source Description ################################
void RTCPAbstractServer::receivedSourceDescription(const InternetFlow flow,
                                                   const card32       source,
                                                   const card8        type,
                                                   char*              data,
                                                   const card8        length)
{
   synchronized();

   Client* client = findClient(source,flow);
   if(client == NULL) {
      // ====== Create new client ===========================================
      if(type == RTCP_SDES_CNAME) {
         const String cnameString(data,length);
         Client* client = new Client;
         if(client != NULL) {
            client->SSRC          = source;
            client->ClientAddress = flow;
            client->ClientAddress.setPrintFormat(InternetAddress::PF_Address);
            client->ClientAddress.setPort(client->ClientAddress.getPort() + 1);
            client->TimeStamp     = getMicroTime();
            client->Timeout       = DefaultTimeout;
            client->UserData      = newClient(client,cnameString.getData());
            if(client->UserData != NULL) {
               ClientSet.insert(std::pair<const cardinal,Client*>(source,client));
            }
         }
         else {
            outOfMemoryWarning();
         }
      }
   }

   // ====== Forward SDES message to client =================================
   if(client) {
      sdesMessage(client,type,data,length);
   }

   unsynchronized();
}


// ###### Handle received Sender Report #####################################
void RTCPAbstractServer::receivedSenderReport(
                            const InternetFlow        flow,
                            const card32              source,
                            RTCPReceptionReportBlock* report,
                            const cardinal            layer)
{
   std::cerr << "RTCPAbstractServer::receivedSenderReport() - Not implemented yet!"
        << std::endl;
}


// ###### Handle received RTCP Bye message ##################################
void RTCPAbstractServer::receivedBye(const InternetFlow flow,
                                     const card32       source,
                                     const DeleteReason reason)
{
   synchronized();
   std::multimap<const cardinal,Client*>::iterator found = ClientSet.find(source);
   if(found != ClientSet.end()) {
      Client* client = found->second;
      if((InternetAddress)client->ClientAddress == (InternetAddress)flow) {
         deleteClient(client,reason);
         ClientSet.erase(found);
         delete client;
      }
   }
   unsynchronized();
}


// ###### Handle received RTCP APP message ##################################
void RTCPAbstractServer::receivedApp(const InternetFlow flow,
                                     const card32       source,
                                     const char*        name,
                                     void*              data,
                                     const card32       dataLength)
{
   synchronized();
   Client* client = findClient(source,flow);
   if(client) {
      appMessage(client,name,data,dataLength);
      client->TimeStamp = getMicroTime();
   }
   unsynchronized();
}


// ###### Handle received RTCP Receiver Report ##############################
void RTCPAbstractServer::receivedReceiverReport(
                            const InternetFlow        flow,
                            const card32              source,
                            RTCPReceptionReportBlock* report,
                            const cardinal            layer)
{
   synchronized();
   Client* client = findClient(source,flow);
   if(client) {
      receiverReport(client,report,layer);
      client->TimeStamp = getMicroTime();
   }
   unsynchronized();

/*
   std::cout << "RTCP Receiver Report from " << source << std::endl;
   std::cout << "   SSRC            : " << report->getSSRC()         << std::endl;
   std::cout << "   Fraction Lost   : " << report->getFractionLost() << std::endl;
   std::cout << "   Packets Lost    : " << report->getPacketsLost()  << std::endl;
   std::cout << "   Last Seq Number : " << report->getLastSeqNum()   << std::endl;
   std::cout << "   Interar. Jitter : " << report->getJitter()       << std::endl;
   std::cout << "   LSR             : " << report->getLSR()          << std::endl;
   std::cout << "   DLSR            : " << report->getDLSR()         << std::endl;
   std::cout << "}" << std::endl;
*/
}


// ###### Find client #######################################################
RTCPAbstractServer::Client* RTCPAbstractServer::findClient(
                               const card32       source,
                               const InternetFlow flow)
{
   std::multimap<const cardinal,Client*>::iterator found = ClientSet.find(source);
   if(found != ClientSet.end()) {
      Client* client = found->second;
      if((InternetAddress)client->ClientAddress == (InternetAddress)flow) {
         return(client);
      }
      else {
         char str[32];
         snprintf((char*)&str,sizeof(str),"$%08x",source);
         std::cerr << "WARNING: SSRC " << str << " changed address from "
                   << (InternetAddress)client->ClientAddress << " to "
                   << (InternetAddress)flow << std::endl;
      }
   }
   return(NULL);
}


// ###### timerEvent() implementation for TimedThread #######################
void RTCPAbstractServer::timerEvent()
{
   synchronized();

   const card64 now = getMicroTime();

   std::multimap<const cardinal,Client*>::iterator clientIterator = ClientSet.begin();
   while(clientIterator != ClientSet.end()) {
      Client* client = clientIterator->second;

      if(client->TimeStamp + client->Timeout < now) {
         receivedBye(client->ClientAddress, client->SSRC, DeleteReason_Timeout);
         clientIterator = ClientSet.begin();
      }
      else if(checkClient(client) == false) {
         receivedBye(client->ClientAddress, client->SSRC, DeleteReason_Error);
         clientIterator = ClientSet.begin();
      }
      else {
         clientIterator++;
      }
   }

   unsynchronized();
}
