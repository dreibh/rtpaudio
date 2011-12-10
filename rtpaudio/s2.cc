// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Server                                                     ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de.de                               ####
// ####    WWW:   http://www.iem.uni-due.de.de/~dreibh/rn                ####
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
#include "tdsocket.h"
#include "rtpsender.h"
#include "rtcppacket.h"
#include "rtcpreceiver.h"
#include "rtcpabstractserver.h"
#include "audioclientapppacket.h"
#include "tools.h"
#include "breakdetector.h"
#include "simpleaudioencoder.h"
#include "advancedaudioencoder.h"

#include "multiaudioreader.h"
#include "audioencoderrepository.h"

//#include "audioserver.h"
//#include "qosmanager.h"


#define SCTP_MAXADDRESSES 20


template<class T> class MessageQueue : public Condition
{
   public:
   MessageQueue();
   ~MessageQueue();

   void flush();
   bool push(T* request);
   T* pop();


//   friend ostream& operator<<(ostream& os, const MessageQueue<T>& queue);


   public:
   struct Message {
      Message* Next;
      T*       Content;
   };
   Message* FirstMessage;
   Message* LastMessage;
};

template<class T> ostream& operator<<(ostream& os, MessageQueue<T>& queue)
{
   queue.synchronized();

   MessageQueue<T>::Message* msg = queue.FirstMessage;
   os << "MessageQueue \"" << queue.getName() << "\":" << endl;
   while(msg != NULL) {
      char str[128];
      snprintf((char*)&str,sizeof(str),"$%08x: $%08x",(card32)msg,(card32)msg->Content);
      os << "   " << str << endl;
      msg = msg->Next;
   }
   os << "-----------------------" << endl;

   queue.unsynchronized();
   return(os);
}

template<class T> MessageQueue<T>::MessageQueue()
   : Condition("MessageQueue")
{
   FirstMessage = NULL;
   LastMessage  = NULL;
}

template<class T> MessageQueue<T>::~MessageQueue()
{
   flush();
}

template<class T> void MessageQueue<T>::flush()
{
   synchronized();
   Message* msg = FirstMessage;
   while(msg != NULL) {
      Message* next = msg->Next;
      delete msg->Content;
      delete msg;
      msg = next;
   }
   FirstMessage = NULL;
   LastMessage  = NULL;
   unsynchronized();
   broadcast();
}

template<class T> bool MessageQueue<T>::push(T* message)
{
   Message* msg = new Message;
   if(msg != NULL) {
      synchronized();
      msg->Next    = NULL;
      msg->Content = message;
      if(LastMessage != NULL) {
         LastMessage->Next = msg;
      }
      else {
         LastMessage = msg;
      }
      if(FirstMessage == NULL) {
         FirstMessage = msg;
      }
      unsynchronized();
      broadcast();
      return(true);
   }
   return(false);
}

template<class T> T* MessageQueue<T>::pop()
{
   T* message = NULL;
   synchronized();
   if(FirstMessage != NULL) {
      message       = FirstMessage->Content;
      Message* next = FirstMessage->Next;
      delete FirstMessage;
      if(FirstMessage == LastMessage) {
         LastMessage = NULL;
      }
      FirstMessage = next;
   }
   unsynchronized();
   return(message);
}




   enum DeleteReason {
      DeleteReason_UserBye  = 0,
      DeleteReason_Timeout  = 1,
      DeleteReason_Shutdown = 2,
      DeleteReason_Error    = 3,
   };

class AbstractMediaServentRequest
{
   public:
   card16           SequenceNumber;
   card16           PosChgSeqNumber;

   enum MediaServentMode {
      MSM_Stop  = 0,
      MSM_Play  = 1,
      MSM_Pause = 2,
   };
   MediaServentMode Mode;
   integer          Speed;
   cardinal         Encoding;
   card64           StartPosition;
   card64           RestartPosition;
   cardinal         BandwidthLimit;
   char             MediaName[128];
};

class AudioServentRequest : public AbstractMediaServentRequest
{
   public:
   /**
     * Audio sampling rate.
     */
   card16 SamplingRate;

   /**
     * Number of audio channels.
     */
   card8 Channels;

   /**
     * Number of audio bits.
     */
   card8 Bits;
};


   enum ShutdownReason {
      SR_NoShutdown     = 0,
      SR_UserShutdown   = 1,
      SR_ServerShutdown = 2,
      SR_Timeout        = 3,
      SR_Error          = 255
   };


const cardinal MaxMediaServentLayers = 16;

struct MediaServentLayerReport
{
   card64 LastUpdate;

   double FractionLost;
   double Jitter;
};

struct MediaServentReport
{
   cardinal                Layers;
   MediaServentLayerReport LayerReport[MaxMediaServentLayers];
};



   enum ServentQueueErrors {
      SQE_Okay       = 0,
      SQE_NotForMe   = 1,
      SQE_Invalid    = 2,
      SQE_AuthFailed = 3,
      SQE_NoMemory   = 255
   };








class AbstractMediaServer;


class MediaServent : public Thread
{
   public:
   MediaServent(AbstractMediaServer*  server,
                const String&         identifier,
                SocketAddress*        peerAddress,
                const integer         communicationDomain = Socket::IP,
                const integer         socketType          = Socket::UDP,
                const integer         socketProtocol      = Socket::Default,
                const SocketAddress** localAddressArray   = NULL,
                const cardinal        localAddresses      = 0);
   ~MediaServent();

   ServentQueueErrors queueRequest(AbstractMediaServentRequest* request);

   void updateReport(const MediaServentLayerReport& report,
                     const cardinal                 layer);


   void shutdown(const ShutdownReason reason);

   inline const String getIdentifier() const;
   inline void   setTimeout(const card64 timeout);
   inline card64 getTimeout() const;
   inline void   updateTimeStamp(const card64 timeStamp = getMicroTime());
   inline bool   hasTimedOut(const card64 now = getMicroTime()) const;


   virtual bool transmissionErrorOccured() = 0;


   protected:
   virtual void handleRequest(AbstractMediaServentRequest* request) = 0;


   protected:
   AbstractMediaServer*                      Server;
   String                                    Identifier;
   MessageQueue<AbstractMediaServentRequest> Queue;

   card64              TimeStamp;
   card64              Timeout;


   MediaServentReport  Report;

   ShutdownReason      ShutdownStatus;


   Socket                      SenderSocket;
   InternetFlow                Flow;

   card16 LastSequenceNumber;
   card16 PosChgSeqNumber;

   bool                        UserLimitPause;
   bool                        ManagerLimitPause;
   bool                        ClientPause;


/*
      InternetFlow                Flow;


      String                      MediaName;
      integer                     StreamIdentifier;
      card64                      BandwidthLimit;
      card16                      LastSequenceNumber;
      card16                      PosChgSeqNumber;

*/

   void run();
};



MediaServent::MediaServent(AbstractMediaServer*  server,
                           const String&         identifier,
                           SocketAddress*        peerAddress,
                           const integer         communicationDomain,
                           const integer         socketType,
                           const integer         socketProtocol,
                           const SocketAddress** localAddressArray,
                           const cardinal        localAddresses)
{
puts("CCCREATE!!!!!!!!!!!!!!!!!!!");
   Server         = server;
   Identifier     = identifier;
   TimeStamp      = getMicroTime();
   Timeout        = 0;
   ShutdownStatus = SR_NoShutdown;

   LastSequenceNumber = 0xffff;
   PosChgSeqNumber    = 0xffff;

   SenderSocket.create(communicationDomain, socketType, socketProtocol);
   if(SenderSocket.ready()) {
      if(SenderSocket.bindx(localAddressArray,
                            localAddresses,
                            SCTP_BINDX_ADD_ADDR) == true) {
         if(socketProtocol == IPPROTO_SCTP) {
            int on = 0;
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_RECVDATAIOEVNT,&on,sizeof(on)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVDATAIOEVNT failed!" << endl;
            }
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_RECVASSOCEVNT,&on,sizeof(on)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVASSOCEVNT failed!" << endl;
            }
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_RECVPADDREVNT,&on,sizeof(on)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVPADDREVNT failed!" << endl;
            }
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_RECVSENDFAILEVNT,&on,sizeof(on)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVSENDFAILEVNT failed!" << endl;
            }
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_RECVPEERERR,&on,sizeof(on)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVPEERERR failed!" << endl;
            }
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_RECVSHUTDOWNEVNT,&on,sizeof(on)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVSHUTDOWNEVNT failed!" << endl;
            }
            sctp_initmsg init;
            init.sinit_num_ostreams   = MaxMediaServentLayers;
            init.sinit_max_instreams  = 1;
            init.sinit_max_attempts   = 0;
            init.sinit_max_init_timeo = 60;
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_INITMSG,(char*)&init,sizeof(init)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - Unable to set SCTP_INITMSG parameters!" << endl;
               SenderSocket.close();
               return;
            }
// ?????????????????????????????
/*
            sctp_ustreams unreliable;
            unreliable.sus_start = 0;
            unreliable.sus_stop  = 0;
            if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_UNRELIABLE,&unreliable,sizeof(unreliable)) < 0) {
               cerr << "WARNING: MediaServent::MediaServent() - SCTP_UNRELIABLE failed!" << endl;
            }
*/
         }
// ?????????????????????????????
/*
         Flow = SenderSocket.allocFlow(peerAddress);
         if(Flow.getFlowLabel() != 0) {
            if(SenderSocket.connect(Flow,AudioServerDefaultTrafficClass) == false) {
               if(SenderSocket.connect(peerAddress,AudioServerDefaultTrafficClass) == false) {
                  cerr << "ERROR: MediaServent::MediaServent() - Unable to flow-connect to client!" << endl;
                  SenderSocket.close();
                  return;
               }
            }
         }
         else {
*/
cout << "peer=" << *peerAddress << endl;
            if(SenderSocket.connect(*peerAddress,AudioServerDefaultTrafficClass) == false) {
               cerr << "ERROR: MediaServent::MediaServent() - Unable to connect to client!" << endl;
               SenderSocket.close();
               return;
            }
//            }
      }
      else {
         cerr << "WARNING: MediaServent::MediaServent() - Unable to bind socket!" << endl;
         SenderSocket.close();
      }
   }
   else {
      cerr << "WARNING: MediaServent::MediaServent() - Unable to create socket!" << endl;
      SenderSocket.close();
   }
}

MediaServent::~MediaServent()
{
puts("sd0");
   shutdown(SR_ServerShutdown);
puts("sd1");
   join();
puts("sd2b");
   SenderSocket.close();
puts("sd3");
}

void MediaServent::run()
{
   while(ShutdownStatus == SR_NoShutdown) {
puts("QWAIT0.......................................");
      Queue.wait();
puts("QWAIT1///////////////////////////////////////");
      AbstractMediaServentRequest* request = Queue.pop();
      while(request != NULL) {
         handleRequest(request);
         delete request;
         request = Queue.pop();
      }
   }
}


void MediaServent::shutdown(const ShutdownReason reason)
{
   ShutdownStatus = reason;
   if(ShutdownStatus != SR_NoShutdown) {
      Queue.flush();
   }
}


ServentQueueErrors MediaServent::queueRequest(AbstractMediaServentRequest* request)
{
   if(Queue.push(request)) {
      updateTimeStamp();
      return(SQE_Okay);
   }
   return(SQE_NoMemory);
}


void MediaServent::updateReport(const MediaServentLayerReport& report,
                                const cardinal                 layer)
{
   if(layer < MaxMediaServentLayers) {
      updateTimeStamp();
      Report.LayerReport[layer] = report;
   }
   else {
      cerr << "WARNING: MediaServent::updateReport() - Invalid layer number!" << endl;
   }
}
inline const String MediaServent::getIdentifier() const
{
   return(Identifier);
}


inline void MediaServent::setTimeout(const card64 timeout)
{
   Timeout = timeout;
}

inline card64 MediaServent::getTimeout() const
{
   return(Timeout);
}

inline void MediaServent::updateTimeStamp(const card64 timeStamp)
{
   TimeStamp = timeStamp;
}

inline bool MediaServent::hasTimedOut(const card64 now) const
{
   return(now >= TimeStamp + Timeout);
}









class AbstractMediaServer : public TimedThread
{
   public:
   AbstractMediaServer();
   ~AbstractMediaServer();

   protected:
   friend class RTPAdaptionLayer;

   MediaServent* createServent(const String& identifier,
                               SocketAddress* peerAddress);
   void deleteServent(const String&        identifier,
                      const ShutdownReason reason);

   MediaServent* findServent(const String& identifier);


   protected:
   virtual MediaServent* serventFactory(const String& identifier,
                                        SocketAddress* peerAddress) = 0;


   virtual AbstractMediaServentRequest* createRequest(
                                           void*        request,
                                           const size_t length) = 0;



   private:


//   friend class TestReceiver;
/*
   void receivedSenderReport(const String&                   identifier,
                             const RTCPReceptionReportBlock* report,
                             const cardinal                  layer);
   void receivedReceiverReport(const String&                   identifier,
                               const RTCPReceptionReportBlock* report,
                               const cardinal                  layer);
   void receivedSourceDescription(const String&                    identifier,
                                  const RTCPSourceDescriptionItem* item);
   void receivedApp(const String&      identifier,
                    const char*        name,
                    const void*        data,
                    const card32       dataLength);
   void receivedBye(const String&      identifier,
                    const DeleteReason reason);
*/

   private:
   void* stop();
   void timerEvent();

   card64                               DefaultTimeout;
   multimap<const String,MediaServent*> ServentSet;
};

AbstractMediaServer::AbstractMediaServer()
   : TimedThread(1000000,"AbstractMediaServer")
{
   DefaultTimeout = 8000000;
   setTimerCorrection(0);
   setFastStart(false);
}

AbstractMediaServer::~AbstractMediaServer()
{
puts("y0");
   stop();
puts("y1");
}

void* AbstractMediaServer::stop()
{
   void* result = TimedThread::stop();

   synchronized();
   while(ServentSet.begin() != ServentSet.end()) {
      multimap<const String,MediaServent*>::iterator serventIterator = ServentSet.begin();
      MediaServent* servent = serventIterator->second;

      // Note: The servent will be removed and deleted here. The iterator
      //       becomes invalid!
      deleteServent(servent->getIdentifier(), SR_ServerShutdown);
   }
   unsynchronized();

   return(result);
}



MediaServent* AbstractMediaServer::createServent(const String&  identifier,
                                                 SocketAddress* peerAddress)
{
   MediaServent* servent = serventFactory(identifier, peerAddress);
   if(servent != NULL) {
      // ====== Add servent to list =========================================
      synchronized();
      servent->setTimeout(DefaultTimeout);
      ServentSet.insert(pair<const String, MediaServent*>(identifier,servent));
      unsynchronized();

      // ====== Start servent thread ========================================
      servent->start();
   }
   return(servent);
}

void AbstractMediaServer::deleteServent(const String&        identifier,
                                        const ShutdownReason reason)
{
puts("SD1");
   synchronized();
   multimap<const String, MediaServent*>::iterator found = ServentSet.find(identifier);
   if(found != ServentSet.end()) {
      MediaServent* servent = found->second;
puts("SD2");
      ServentSet.erase(found);
      servent->shutdown(reason);
puts("SD3");
      delete servent;
puts("SD4");
   }
   unsynchronized();
}

MediaServent* AbstractMediaServer::findServent(const String& identifier)
{
   multimap<const String,MediaServent*>::iterator found = ServentSet.find(identifier);
   if(found != ServentSet.end()) {
      return(found->second);
   }
   return(NULL);
}



void AbstractMediaServer::timerEvent()
{
   synchronized();

   const card64 now = getMicroTime();

puts("CHECK...");
   multimap<const String,MediaServent*>::iterator serventIterator = ServentSet.begin();
   while(serventIterator != ServentSet.end()) {
      MediaServent* servent = serventIterator->second;
      if(servent->hasTimedOut(now)) {
puts("TO!");
         deleteServent(servent->getIdentifier(), SR_Timeout);
         // Leave loop, since iterator has been invalidated!
         break;
      }
      else if(servent->transmissionErrorOccured() == true) {
puts("ERR!");
         deleteServent(servent->getIdentifier(), SR_Error);
         // Leave loop, since iterator has been invalidated!
         break;
      }
      serventIterator++;
   }

   unsynchronized();
}




class AlphaServent : public MediaServent
{
   public:
   AlphaServent(AbstractMediaServer*  server,
                const String&         identifier,
                SocketAddress*        peerAddress,
                const integer         communicationDomain,
                const integer         socketType,
                const integer         socketProtocol,
                const SocketAddress** localAddressArray,
                const cardinal        localAddresses,
                const cardinal        maxPacketSize);
  ~AlphaServent();

   bool transmissionErrorOccured();

   protected:
   void handleRequest(AbstractMediaServentRequest* request);

   private:
   String                      MediaName;
   MultiAudioReader            MediaReader;
   AudioEncoderRepository      EncoderRepository;
   RTPSender                   Sender;

   card32                      OurSSRC;
   cardinal                    MaxPacketSize;
   card64                      BandwidthLimit;
};

AlphaServent::AlphaServent(AbstractMediaServer*  server,
                           const String&         identifier,
                           SocketAddress*        peerAddress,
                           const integer         communicationDomain,
                           const integer         socketType,
                           const integer         socketProtocol,
                           const SocketAddress** localAddressArray,
                           const cardinal        localAddresses,
                           const cardinal        maxPacketSize)
   : MediaServent(server,
                  identifier,
                  peerAddress,
                  communicationDomain,
                  socketType,
                  socketProtocol,
                  localAddressArray,
                  localAddresses)
{
   MaxPacketSize  = maxPacketSize;
   BandwidthLimit = (card64)-1;

   UserLimitPause      = false;
   ManagerLimitPause   = false;
   ClientPause         = false;

// ????????????????? INITIALISIERUNGSFEHLER ABFANGEN !!!!!!!!!!!!!!!!!!!!

   if(SenderSocket.ready()) {
       // ====== Create repository and encoders =================================
       EncoderRepository.setAutoDelete(true);
       const bool e1 = EncoderRepository.addEncoder(new AdvancedAudioEncoder(&MediaReader));
       const bool e2 = EncoderRepository.addEncoder(new SimpleAudioEncoder(&MediaReader));
       if((e1 == false) || (e2 == false)) {
          return;
       }

       // ====== Initialize RTPSender ===========================================
       Randomizer random;
       OurSSRC = random.random32();
       Sender.init(OurSSRC,&EncoderRepository,&SenderSocket,MaxPacketSize);
       Sender.start();
   }
}

AlphaServent::~AlphaServent()
{
   puts("a1");
   Sender.stop();
   puts("a2");
}

// ??????
#define VERBOSE

void AlphaServent::handleRequest(AbstractMediaServentRequest* requestData)
{
   const AudioServentRequest* request = (AudioServentRequest*)requestData;

puts("Handle....");
   // ====== Check, if command has already been executed ====================
   const integer diff = (integer)request->SequenceNumber -
                           (integer)LastSequenceNumber;

printf("S%d Y=%d\n",request->SequenceNumber,LastSequenceNumber);

   if(!((diff > 0) || (diff < -30000))) {
puts("BAD SEQU!!!!!!!!!!");
printf("diff=%d\n",diff);
      return;
   }

puts("H1");
   LastSequenceNumber = request->SequenceNumber;
   Sender.synchronized();

puts("H2");
   // ====== Set quality, position, media name and bandwidth limit ==========
   if(EncoderRepository.selectEncoderForTypeID(request->Encoding) == true) {
      EncoderRepository.setSamplingRate(request->SamplingRate);
      EncoderRepository.setBits(request->Bits);
      EncoderRepository.setChannels(request->Channels);

      // ====== Select new media and set frame number =======================
      if(MediaReader.getErrorCode() == ME_NoMedia) {
#ifdef VERBOSE
         printTimeStamp();
         cout << Identifier << " loading media <" << (const char*)&request->MediaName
              << ">." << endl;
#endif
         if(MediaReader.openMedia((char*)&request->MediaName)) {
            MediaReader.setPosition(request->RestartPosition);
            PosChgSeqNumber = request->PosChgSeqNumber;
            MediaName       = String((const char*)&request->MediaName);
            Sender.leaveCorrectionLoop();
         }
      }

      else if((MediaName != "") && (MediaName != (char*)&request->MediaName)) {
#ifdef VERBOSE
         printTimeStamp();
         cout << Identifier << " changing media to <" << (const char*)&request->MediaName
              << ">." << endl;
#endif
         MediaReader.closeMedia();
         if(MediaReader.openMedia((char*)&request->MediaName)) {
            MediaReader.setPosition(request->RestartPosition);
            MediaName = String((const char*)&request->MediaName);
            Sender.leaveCorrectionLoop();
         }
      }

      // ====== Change position, if it has been updated =====================
      const integer diff = (integer)request->PosChgSeqNumber -
                              (integer)PosChgSeqNumber;
      if((diff > 0) || (diff < -30000)) {
         PosChgSeqNumber = request->PosChgSeqNumber;
         if(request->StartPosition != (card64)-1) {
            MediaReader.setPosition(request->StartPosition);
            Sender.leaveCorrectionLoop();
         }
      }

      // ====== Update client's bandwidth limit =============================
      BandwidthLimit = (card64)request->BandwidthLimit;
   }
   else {
      char str[32];
      snprintf((char*)&str,sizeof(str),"$%04x",request->Encoding);
      cerr << "WARNING: AudioServer::userCommand() - Unsupported Encoding #"
           << str << " requested!" << endl;
   }


   // ====== Execute command ================================================
   switch(request->Mode) {
      case AbstractMediaServentRequest::MSM_Pause:
        ClientPause = true;
        Sender.setPause(true);
       break;
      case AbstractMediaServentRequest::MSM_Play:
         {
/*
            ExtendedTransportInfo ti;
            Sender.getTransportInfo(ti,false);

            if(BandwidthLimit >= ti.getTotalMinWantedBytesPerSecond()) {
               if(UserLimitPause == true) {
                  UserLimitPause = false;
                  if(ManagerLimitPause == false) {
#ifdef VERBOSE
                     printTimeStamp();
                     char str[128];
                     snprintf((char*)&str,sizeof(str),"$%08x resumed due to increased user bandwidth!",0);
                     cout << str << endl;
#endif
                     Sender.setPause(false);
                  }
               }
            }
            else {
               if(UserLimitPause == false) {
                  UserLimitPause = true;
                  if(ManagerLimitPause == false) {
#ifdef VERBOSE
                     printTimeStamp();
                     char str[128];
                     snprintf((char*)&str,sizeof(str),"$%08x paused due to user bandwidth limit!",0);
                     cout << str << endl;
#endif
                     Sender.setPause(true);
                  }
               }
            }
*/
            if((ClientPause == true) && (UserLimitPause == false) && (ManagerLimitPause == false)) {
               ClientPause = false;
               Sender.setPause(false);
            }
         }
        break;
       default:
        break;
   }


   Sender.unsynchronized();
}


bool AlphaServent::transmissionErrorOccured()
{
   return(Sender.transmissionErrorDetected());
}


class AlphaServer : public AbstractMediaServer
{
   public:
   AlphaServer(SocketAddress** localAddressArray = NULL,
               const cardinal  localAddresses    = 0);
   ~AlphaServer();

   MediaServent* serventFactory(const String&  identifier,
                                SocketAddress* peerAddress);

   AbstractMediaServentRequest* createRequest(
                                   void*        request,
                                   const size_t length);


   private:
   SocketAddress** LocalAddressArray;
   cardinal        LocalAddresses;
   integer         CommunicationDomain;
   integer         SocketType;
   integer         SocketProtocol;
};

AlphaServer::AlphaServer(SocketAddress** localAddressArray,
                         const cardinal  localAddresses)
{
   CommunicationDomain = Socket::IP,
   SocketType          = SOCK_DGRAM;
   SocketProtocol      = IPPROTO_UDP;
   LocalAddressArray   = localAddressArray;
   LocalAddresses      = localAddresses;
}

AlphaServer::~AlphaServer()
{
   puts("ASERVER...");
}


MediaServent* AlphaServer::serventFactory(const String&  identifier,
                                         SocketAddress* peerAddress)
{
   return(new AlphaServent(this,
                           identifier,
                           peerAddress,
                           CommunicationDomain,
                           SocketType,
                           SocketProtocol,
                           (const SocketAddress**)LocalAddressArray,
                           LocalAddresses,
                           1400 /* ????????????? */));
}


AbstractMediaServentRequest* AlphaServer::createRequest(void*        appPacket,
                                                        const size_t length)
{
   AudioClientAppPacket* app = (AudioClientAppPacket*)appPacket;
   if((length >= sizeof(AudioClientAppPacket)) &&
      (translate32(app->FormatID) == AudioClientAppPacket::AudioClientFormatID)) {
      app->translate();

      AudioServentRequest* request = new AudioServentRequest;
      if(request != NULL) {
         request->SequenceNumber  = app->SequenceNumber;
         request->PosChgSeqNumber = app->PosChgSeqNumber;
         switch(app->Status) {
            case AudioClientAppPacket::ACAS_Pause:
               request->Mode = AbstractMediaServentRequest::MSM_Pause;
             break;
            case AudioClientAppPacket::ACAS_Play:
               request->Mode = AbstractMediaServentRequest::MSM_Play;
             break;
            default:
               request->Mode = AbstractMediaServentRequest::MSM_Stop;
             break;
         }
         request->Speed           = 1;
         request->Encoding        = app->Encoding;
         request->StartPosition   = app->StartPosition;
         request->RestartPosition = app->RestartPosition;
         request->BandwidthLimit  = app->BandwidthLimit;
         memcpy((char*)request->MediaName,
                app->MediaName,
                min(sizeof(request->MediaName),
                    sizeof(app->MediaName)));
         request->MediaName[sizeof(request->MediaName) - 1] = 0x00;

         request->SamplingRate = app->SamplingRate;
         request->Bits         = app->Bits;
         request->Channels     = app->Channels;
      }
      return(request);
   }
   else {
      cerr << "WARNING: AbstractMediaServentRequest* AlphaServer::createRequest() - Bad request received!" << endl;
   }
   return(NULL);
}




class RTPAdaptionLayer
{
   public:
   RTPAdaptionLayer(AbstractMediaServer* server);

   ~RTPAdaptionLayer();

      enum DeleteReason {
      DeleteReason_UserBye  = 0,
      DeleteReason_Timeout  = 1,
      DeleteReason_Shutdown = 2,
      DeleteReason_Error    = 3,
   };

   void receivedSenderReport(const InternetFlow              flow,
                             const card32                    source,
                             const RTCPReceptionReportBlock* report,
                             const cardinal                  layer);
   void receivedReceiverReport(const InternetFlow              flow,
                               const card32                    source,
                               const RTCPReceptionReportBlock* report,
                               const cardinal                  layer);
   void receivedSourceDescription(const InternetFlow flow,
                                  const card32       source,
                                  const card8        type,
                                  const char*        data,
                                  const card8        length);
   void receivedApp(const InternetFlow flow,
                    const card32       source,
                    const char*        name,
                    const void*        data,
                    const card32       dataLength);
   void receivedBye(const InternetFlow flow,
                    const card32       source,
                    const DeleteReason reason);

   String getIdentifier(const InternetFlow flow,
                        const card32       source);


   private:
   AbstractMediaServer* Server;
};

String RTPAdaptionLayer::getIdentifier(const InternetFlow flow,
                                const card32       source)
{
   InternetAddress a = flow;
   return(a.getAddressString(SocketAddress::PF_Address) + "/" + String(source));
}

void RTPAdaptionLayer::receivedSenderReport(const InternetFlow              flow,
                          const card32                    source,
                          const RTCPReceptionReportBlock* report,
                          const cardinal                  layer)
{
}

void RTPAdaptionLayer::receivedReceiverReport(const InternetFlow              flow,
                            const card32                    source,
                            const RTCPReceptionReportBlock* report,
                            const cardinal                  layer)
{
}

void RTPAdaptionLayer::receivedSourceDescription(const InternetFlow flow,
                               const card32       source,
                               const card8        type,
                               const char*        data,
                               const card8        length)
{
   String id = getIdentifier(flow,source);
   cout << "id=" << id << endl;

   if(type == RTCP_SDES_CNAME) {
      if(Server->findServent(id) == NULL) {

         SocketAddress* peerAddress = flow.duplicate();
         if(peerAddress != NULL) {
            peerAddress->setPort(peerAddress->getPort() + 1);

            MediaServent* servent = Server->createServent(id,peerAddress);

            printf("CREATE: %x\n",(card32)servent);
            delete peerAddress;
         }
      }
   }
   else if(type == RTCP_SDES_PRIV) {
      puts("SDES-PRIV!!!!");
      receivedApp(flow,source,"PRIV",data,length);
   }
   puts("de ok!");
}

void RTPAdaptionLayer::receivedApp(const InternetFlow flow,
                 const card32       source,
                 const char*        name,
                 const void*        data,
                 const card32       dataLength)
{
   String id = getIdentifier(flow,source);
   cout << "app id=" << id << endl;

   MediaServent* servent = Server->findServent(id);
   if(servent != NULL) {
puts("APP!");
/// const discard!!!!
      AbstractMediaServentRequest* request = Server->createRequest((void*)data,dataLength);
      if(request != NULL) {
         servent->queueRequest(request);
         puts("queued!");
      }
   }
}

void RTPAdaptionLayer::receivedBye(const InternetFlow flow,
                 const card32       source,
                 const DeleteReason reason)
{
}



RTPAdaptionLayer::RTPAdaptionLayer(AbstractMediaServer* server)
{
   Server = server;
}

RTPAdaptionLayer::~RTPAdaptionLayer()
{
}






// Debug mode: Print corrupted RTCP packets
#define DEBUG


/**
  * This class implements an RTCP receiver based on Thread.
  *
  * @short   RTCP Receiver
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class TestReceiver : public Thread
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Default constructor.
     * You have to initialize TestReceiver by calling init(...) later!
     *
     * @see init
     */
   TestReceiver();

   /**
     * Constructor for new TestReceiver. The new receiver's thread has to be
     * started by calling start()!
     *
     * @param server RTPAdaptionLayer.
     * @param receiverSocket Socket to receive RTCP packets from.
     */
   TestReceiver(RTPAdaptionLayer* server,
                Socket*             receiverSocket);

   /**
     * Destructor.
     */
   ~TestReceiver();


   // ====== Initialize =====================================================
   /**
     * Initialize new TestReceiver. The new receiver's thread has to be
     * started by calling start()!
     *
     * @param server RTPAdaptionLayer.
     * @param receiverSocket Socket to receive RTCP packets from.
     */
   void init(RTPAdaptionLayer* server,
             Socket*             receiverSocket);


   // ====== Private data ===================================================
   private:
   void  run();


   Socket*             ReceiverSocket;
   RTPAdaptionLayer* Server;
   double              AverageRTCPSize; // Average compound RTCP packet size
};


// ###### Constructor #######################################################
TestReceiver::TestReceiver()
   : Thread("TestReceiver")
{
   Server         = NULL;
   ReceiverSocket = NULL;
}


// ###### Constructor #######################################################
TestReceiver::TestReceiver(RTPAdaptionLayer* server, Socket* receiverSocket)
   : Thread("TestReceiver")
{
   init(server,receiverSocket);
}


// ###### Initialize ########################################################
void TestReceiver::init(RTPAdaptionLayer* server, Socket* receiverSocket)
{
   Server         = server;
   ReceiverSocket = receiverSocket;
}


// ###### Destructor ########################################################
TestReceiver::~TestReceiver()
{
   stop();
}


// ###### Receiver loop #####################################################
void TestReceiver::run()
{
   if(ReceiverSocket == NULL) {
      cerr << "ERROR: TestReceiver::run() - TestReceiver is uninitialized!"
           << endl;
      return;
   }

   InternetFlow flow;
   for(;;) {
      // ====== Read RTCP packet ============================================
      char packetData[8192];
      integer receivedPacketSize =
         ReceiverSocket->receiveFrom((char*)&packetData,sizeof(packetData),flow,0);
      if(receivedPacketSize < (ssize_t)sizeof(RTCPCommonHeader)) {
         if(receivedPacketSize > 0) {
            cerr << "WARNING: TestReceiver::run() - Received bad RTCP header" << endl;
         }
         continue;
      }


      // ====== Verify RTCP packet ==========================================
      RTCPCommonHeader* header  = (RTCPCommonHeader*)&packetData;
      const cardinal packetSize = header->getLength();

      cout << "RTCP Common Header\n";
      cout << "   Version     : " << (cardinal)header->getVersion()    << "\n";
      cout << "   Padding     : " << (cardinal)header->getPadding()    << "\n";
      cout << "   Count       : " << (cardinal)header->getCount()      << "\n";
      cout << "   Packet Type : " << (cardinal)header->getPacketType() << "\n";
      cout << "   Length      : " << (cardinal)header->getLength()     << endl;

      if(receivedPacketSize < (integer)packetSize) {
#ifdef DEBUG
         cerr << "RTCP packet: receivedPacketSize < packetSize: "
              << receivedPacketSize << " <=> " << packetSize << endl;
#endif
         continue;
      }

      if(header->getVersion() != RTPConstants::RTPVersion) {
#ifdef DEBUG
         cerr << "RTCP packet: Invalid RTP version: " << header->getVersion() << endl;
#endif
         continue;
      }

      RTCPCommonHeader* r    = (RTCPCommonHeader*)&packetData;
      RTCPCommonHeader* rend = (RTCPCommonHeader*)(long)r + packetSize;
      do {
         r = (RTCPCommonHeader*)((long*)r + (long)r->getLength());
      } while((r < rend) && (r->getVersion() == RTPConstants::RTPVersion));
      if(r != rend) {
#ifdef DEBUG
         cerr << "RTCP packet: Length check failed!" << endl;
#endif
         continue;
      }


      // ====== Get type and invoke server function =========================
      synchronized();
      switch(header->getPacketType()) {

         // ====== Packet is a Receiver Report ==============================
         case RTCP_RR:
            {
               RTCPReceiverReport* receiverReport = (RTCPReceiverReport*)&packetData;
               cardinal bytes = sizeof(RTCPReceiverReport);
               cardinal layer = 0;
               card32   ssrc  = 0;
               for(cardinal i = 0;i < receiverReport->getCount();i++) {
                  if((bytes + sizeof(RTCPReceptionReportBlock)) <= packetSize) {
                     if(receiverReport->rr[i].getSSRC() == ssrc) {
                        layer++;
                     }
                     else {
                        layer = 0;
                        ssrc  = receiverReport->rr[i].getSSRC();
                     }
                     Server->receivedReceiverReport(
                        flow, receiverReport->getSSRC(),
                        &receiverReport->rr[i], layer);
                  }
                  else {
#ifdef DEBUG
                     cerr << "RTCP packet: Invalid receiver report length!" << endl;
#endif
                     break;
                  }
                  bytes += sizeof(RTCPReceptionReportBlock);
               }
            }
          break;

         // ====== Packet is a Sender Report ================================
         case RTCP_SR:
            {
               RTCPSenderReport* senderReport = (RTCPSenderReport*)&packetData;
               cardinal bytes = (long)&senderReport->rr[0] - (long)senderReport;
               cardinal layer = 0;
               card32   ssrc  = 0;
               for(cardinal i = 0;i < senderReport->getCount();i++) {
                  if((bytes + sizeof(RTCPReceptionReportBlock)) <= packetSize) {
                     if(senderReport->rr[i].getSSRC() == ssrc) {
                        layer++;
                     }
                     else {
                        layer = 0;
                        ssrc  = senderReport->rr[i].getSSRC();
                     }
                     Server->receivedSenderReport(
                        flow, senderReport->getSSRC(),
                        &senderReport->rr[i], layer);
                  }
                  else {
#ifdef DEBUG
                     cerr << "RTCP packet: Invalid sender report length!" << endl;
#endif
                     break;
                  }
                  bytes += sizeof(RTCPReceptionReportBlock);
               }
            }
          break;

         // ====== Packet is a Source Description ===========================
         case RTCP_SDES:
            {
               RTCPSourceDescription* sdes          = (RTCPSourceDescription*)&packetData;
               const RTCPSourceDescriptionItem* end = (RTCPSourceDescriptionItem*)((long)sdes + sdes->getLength());
               RTCPSourceDescriptionChunk* sd       = &sdes->Chunk[0];
               RTCPSourceDescriptionItem* rsp;
               RTCPSourceDescriptionItem* rspn;
               integer count = sdes->getCount();
               while(--count >= 0) {
                  rsp = &sd->Item[0];
                  if(rsp >= end) {
                     break;
                  }
                  for ( ;rsp->Type;rsp = rspn) {
                     rspn = (RTCPSourceDescriptionItem*)((char*)rsp + rsp->Length + 2);
                     if(rspn >= end) {
                        rsp = rspn;
                        break;
                     }
                     Server->receivedSourceDescription(
                         flow, sd->SRC, rsp->Type, rsp->Data, rsp->Length);
                  }
                  rsp = (RTCPSourceDescriptionItem*)((long)sd + (((char*)rsp - (char*)sd) >> 2) + 1);
               }
            }
          break;

         // ====== Packet is a Bye message ==================================
         case RTCP_BYE:
            {
               RTCPBye* bye = (RTCPBye*)&packetData;
               for(cardinal i = 0;i < bye->getCount();i++) {
                  Server->receivedBye(flow, bye->getSource(i),
                                      RTPAdaptionLayer::DeleteReason_UserBye);
               }
            }
           break;

         // ====== Packet is an App message =================================
         case RTCP_APP:
            {
               RTCPApp* app = (RTCPApp*)&packetData;
               Server->receivedApp(flow,
                                   app->getSource(),
                                   app->getName(),
                                   (void*)app->getData(),
                                   packetSize - sizeof(RTCPCommonHeader) - 8);
            }
           break;

         // ====== Packet type is unknown ===================================
         default:
            receivedPacketSize = 0;
#ifdef DEBUG
            cerr << "RTCP packet: Unknown SDES type "
                 << header->getPacketType() << endl;
#endif
          break;

      }
      unsynchronized();
      AverageRTCPSize = (1.0/16.0) * receivedPacketSize + (15.0/16.0) * AverageRTCPSize;
   }
}







// Fast Break: Disable break detector to debug thread deadlocks
// #define FAST_BREAK


// Globals
Socket*                  rtcpServerSocket  = NULL;
TestReceiver*            rtcpReceiver      = NULL;
RTPAdaptionLayer*        adapt = NULL;
AlphaServer*             server            = NULL;
BandwidthManager*        qosManager        = NULL;


void cleanUp(const cardinal exitCode = 0);


// ###### Initialize ########################################################
void initAll(const char*              directory,
             SocketAddress**          localAddressArray,
             const cardinal           localAddresses,
             const card16             port,
             const card64             timeout,
             const cardinal           maxPacketSize,
             const bool               lossScalability,
             const bool               useSCTP)
{
   rtcpServerSocket = new Socket(Socket::IP,Socket::UDP,useSCTP ? Socket::SCTP : Socket::Default);
   if(rtcpServerSocket == NULL) {
      cerr << "ERROR: Server::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
   if(useSCTP) {
      int on = 0;
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_RECVDATAIOEVNT,&on,sizeof(on)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVDATAIOEVNT failed!" << endl;
      }
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_RECVASSOCEVNT,&on,sizeof(on)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVASSOCEVNT failed!" << endl;
      }
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_RECVPADDREVNT,&on,sizeof(on)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVPADDREVNT failed!" << endl;
      }
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_RECVSENDFAILEVNT,&on,sizeof(on)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVSENDFAILEVNT failed!" << endl;
      }
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_RECVPEERERR,&on,sizeof(on)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVPEERERR failed!" << endl;
      }
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_RECVSHUTDOWNEVNT,&on,sizeof(on)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_RECVSHUTDOWNEVNT failed!" << endl;
      }
      sctp_initmsg init;
      init.sinit_num_ostreams   = 1;
      init.sinit_max_instreams  = 1;
      init.sinit_max_attempts   = 0;
      init.sinit_max_init_timeo = 60;
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_INITMSG,(char*)&init,sizeof(init)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - Unable to set SCTP_INITMSG parameters!" << endl;
      }
/*
      sctp_ustreams unreliable;
      unreliable.sus_start = 0;
      unreliable.sus_stop  = 0;
      if(rtcpServerSocket->setSocketOption(IPPROTO_SCTP,SCTP_UNRELIABLE,&unreliable,sizeof(unreliable)) < 0) {
         cerr << "WARNING: MediaServent::MediaServent() - SCTP_UNRELIABLE failed!" << endl;
      }
      ?????
*/
   }
   for(cardinal i = 0;i < localAddresses;i++) {
      localAddressArray[i]->setPort(port);
   }
   if(rtcpServerSocket->bindx((const SocketAddress**)localAddressArray,
                              localAddresses,
                              SCTP_BINDX_ADD_ADDR) == false) {
      cerr << "ERROR: Server::initAll() - Unable to bind socket!" << endl;
      cleanUp(1);
   }
   for(cardinal i = 0;i < localAddresses;i++) {
      localAddressArray[i]->setPort(0);
   }
   server = new AlphaServer(localAddressArray,localAddresses);
   if(server == NULL) {
      cerr << "ERROR: Server::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }
/* ????????????????
   server->setDefaultTimeout(timeout);
   server->setLossScalability(lossScalability);
*/
adapt = new RTPAdaptionLayer(server);
   rtcpReceiver = new TestReceiver(adapt,rtcpServerSocket);
   if(rtcpReceiver == NULL) {
      cerr << "ERROR: Server::initAll() - Out of memory!" << endl;
      cleanUp(1);
   }

   if(server->start() == false) {
      cerr << "ERROR: Server::initAll() - Unable to start server thread!" << endl;
      cleanUp(1);
   }

   if(rtcpReceiver->start() == false) {
      cerr << "ERROR: Server::initAll() - Unable to start RTCP receiver thread!" << endl;
      cleanUp(1);
   }

   // ====== Change directory ===============================================
   if(directory != NULL) {
      if(chdir(directory) != 0) {
         cerr << "ERROR: Server::initAll() - Unable to change directory!" << endl;
         cleanUp(1);
      }
   }
   else {
      directory = "./";
   }
}


// ###### Clean up ##########################################################
void cleanUp(const cardinal exitCode)
{
puts("X1");
   if(rtcpReceiver != NULL) {
      delete rtcpReceiver;
   }
puts("X2");
   if(server != NULL) {
      delete server;
   }
puts("X3");
   if(rtcpServerSocket != NULL) {
      delete rtcpServerSocket;
   }
puts("X4");
   /*
   if(qosManager != NULL) {
      delete qosManager; ????
   }
   */
puts("X5");
   if(exitCode == 0) {
      cout << "Terminated!" << endl;
   }
   exit(exitCode);
}


// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Initialize ======================================================
   bool   optForceIPv4               = false;
   bool   optUseSCTP                 = false;
   bool   lossScalability            = true;
   bool   disableQM                  = false;
   char*  manager                    = NULL;
   cardinal maxPacketSize            = 1500;
   card64 timeout                    = 10000000;
   card16 port                       = AudioClientAppPacket::RTPAudioDefaultPort;
   SocketAddress** localAddressArray = NULL;
   cardinal        localAddresses    = 0;
   String          directory;


   // ====== Read configuration from file ===================================
   FILE* inputFD = fopen("AudioServer.config","r");
   if(inputFD != NULL) {
      char str[256];
      char* result = fgets((char*)&str,256,inputFD);
      cardinal line = 0;
      while(!feof(inputFD)) {
         line++;
         const cardinal inputLength = strlen((char*)&str);
         if(inputLength > 1) {
            str[inputLength - 1] = 0x00;
            switch(str[0]) {
               // ====== Line is a comment =====================================
               case '#':
                break;

               // ====== Line is a setting =====================================
               default:
                  const String input(str);
                  String name;
                  String value;
                  if(input.scanSetting(name,value)) {
                     if(name == "PORT") {
                        int portNumber;
                        if(sscanf(value.getData(),"%d",&portNumber) != 1) {
                           cerr << "ERROR: Bad port setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Port = <number>" << endl;
                           exit(1);
                        }
                        port = (card16)portNumber;
                     }
                     else if(name == "TIMEOUT") {
                        int timeoutValue;
                        if(sscanf(value.getData(),"%d",&timeoutValue) != 1) {
                           cerr << "ERROR: Bad timeout setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Timeout = <seconds>" << endl;
                           exit(1);
                        }
                        timeout = 1000000 * (card64)timeoutValue;
                     }
                     else if(name == "MAX PACKET SIZE") {
                        int size;
                        if(sscanf(value.getData(),"%d",&size) != 1) {
                           cerr << "ERROR: Bad maximum packet size setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Max Packet Size = <Bytes>" << endl;
                           exit(1);
                        }
                        maxPacketSize = (cardinal)size;
                     }
                     else if(name == "DIRECTORY") {
                        directory = value;
                     }
                     else if(name == "DISABLE QOS MANAGER") {
                        int off;
                        if(sscanf(value.getData(),"%d",&off) != 1) {
                           cerr << "ERROR: Bad QoS manager setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Disable QoS Manager = <0|1>" << endl;
                           exit(1);
                        }
                        disableQM = (off != 0) ? true : false;
                     }
                     else if(name == "QOS MANAGER") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad QoS manager setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: QoS Manager = <0|1>" << endl;
                           exit(1);
                        }
                        disableQM = (on != 0) ? false : true;
                     }
                     else if(name == "LOSS SCALABILITY") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad loss scalability setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Loss Scalability = <0|1>" << endl;
                           exit(1);
                        }
                        lossScalability = (on != 0) ? true : false;
                     }
                     else if(name == "FORCE IPV4") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad force IPv4 setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: Force IPv4 = <0|1>" << endl;
                           exit(1);
                        }
                        optForceIPv4 = (on != 0) ? true : false;
                     }
                     else if(name == "SCTP") {
                        int on;
                        if(sscanf(value.getData(),"%d",&on) != 1) {
                           cerr << "ERROR: Bad SCTP setting, "
                                   "line " << line << "!" << endl;
                           cerr << "       Syntax: SCTP = <0|1>" << endl;
                           exit(1);
                        }
                        optUseSCTP = (on != 0) ? true : false;
                     }
                     else {
                        cerr << "ERROR: Unknown option <"
                             << name << " = " << value << ">, "
                                "line " << line << "!" << endl;
                        exit(1);
                     }
                  }
                break;
            }
         }
         result = fgets((char*)&str,256,inputFD);
      }
      fclose(inputFD);
   }


   // ===== Check arguments =================================================
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strcasecmp(argv[i],"-force-ipv4")))           optForceIPv4 = true;
      else if(!(strcasecmp(argv[i],"-use-ipv6")))        optForceIPv4 = false;
      else if(!(strcasecmp(argv[i],"-sctp")))            optUseSCTP   = true;
      else if(!(strcasecmp(argv[i],"-nosctp")))          optUseSCTP   = false;
      else if(!(strncasecmp(argv[i],"-local=",7))) {
         if(localAddresses < SCTP_MAXADDRESSES) {
            if(localAddressArray == NULL) {
               localAddressArray = SocketAddress::newAddressList(SCTP_MAXADDRESSES);
               if(localAddressArray == NULL) {
                  cerr << "ERROR: Out of memory!" << endl;
                  exit(1);
               }
            }
            localAddressArray[localAddresses] = SocketAddress::createSocketAddress(
                                                   SocketAddress::PF_HidePort,
                                                   &argv[i][7]);
            if(localAddressArray[localAddresses] == NULL) {
               cerr << "ERROR: Argument #" << i << " is an invalid address!" << endl;
               exit(1);
            }
            localAddresses++;
         }
         else {
            cerr << "ERROR: Too many local addresses!" << endl;
            exit(1);
         }
      }
      else if(!(strncasecmp(argv[i],"-port=",6)))        port      = (card16)atol(&argv[i][6]);
      else if(!(strncasecmp(argv[i],"-manager=",9)))     manager   = &argv[i][9];
      else if(!(strncasecmp(argv[i],"-timeout=",9)))     timeout   = 1000000 * (card64)atol(&argv[i][9]);
      else if(!(strncasecmp(argv[i],"-maxpktsize=",12))) maxPacketSize = (cardinal)atol(&argv[i][12]);
      else if(!(strcasecmp(argv[i],"-disable-qm")))      disableQM = true;
      else if(!(strcasecmp(argv[i],"-enable-qm")))       disableQM = false;
      else if(!(strcasecmp(argv[i],"-disable-ls")))      lossScalability = false;
      else if(!(strcasecmp(argv[i],"-enable-ls")))       lossScalability = true;
      else if(!(strncasecmp(argv[i],"-directory=",11)))  directory = String(&argv[i][11]);
      else {
         cerr << "Usage: " << argv[0] << " {-port=port} {-directory=path} {-manager=host:port} {-local=host} {-timeout=secs} {-maxpktsize=bytes} {-disable-qm|-enable-qm} {-disable-ls|-enable-ls} {-force-ipv4|-use-ipv6}" << endl;
         exit(1);
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         cerr << "NOTE: IPv6 support disabled!" << endl;
      }
   }
   if(port < 1024) {
      cerr << "ERROR: Invalid port number!" << endl;
      exit(1);
   }
   if(localAddressArray == NULL) {
      if(optUseSCTP == true) {
         if(!Socket::getLocalAddressList(
               localAddressArray,
               localAddresses,
               Socket::GLAF_HideBroadcast|Socket::GLAF_HideMulticast|Socket::GLAF_HideAnycast)) {
            cerr << "ERROR: Cannot obtain local addresses!" << endl;
            exit(1);
         }
         if(localAddresses < 1) {
            cerr << "ERROR: No valid local addresses have been found?!" << endl
                 << "       Check your network interface configuration!" << endl;
            exit(1);
         }
      }
      else {
         localAddressArray = SocketAddress::newAddressList(SCTP_MAXADDRESSES);
         if(localAddressArray == NULL) {
            cerr << "ERROR: Out of memory!" << endl;
            exit(1);
         }
         localAddressArray[0] = new InternetAddress(port);
         if(localAddressArray[0] == NULL) {
            cerr << "ERROR: Out of memory!" << endl;
            exit(1);
         }
         localAddresses = 1;
      }
   }
   if(timeout < 5000000) {
      timeout = 5000000;
   }
   else if(timeout > 1800000000) {
      timeout = 1800000000;
   }
   if(maxPacketSize < 256) {
      maxPacketSize = 256;
   }
   else if(maxPacketSize > 1024 * 1024) {
      maxPacketSize = 1024 * 1024;
   }


   // ====== Initialize QoS manager =========================================
   if(disableQM == false) {
/*
      qosManager = new QoSManager();
      if(qosManager == NULL) {
         cerr << "ERROR: Server::main() - Out of memory!" << endl;
         cleanUp(1);
      }
*/
   }


   // ====== Initialize =====================================================
   initAll(directory.getData(),
           localAddressArray,
           localAddresses,
           port,
           timeout, maxPacketSize, lossScalability,
           optUseSCTP);
#ifndef FAST_BREAK
   installBreakDetector();
#endif

   InternetAddress ourAddress;
   rtcpServerSocket->getSocketAddress(ourAddress);
   ourAddress.setPrintFormat(InternetAddress::PF_Address);


   // ====== Print status ===================================================
   cout << "RTP Audio Server - Copyright (C) 1999-2002 Thomas Dreibholz" << endl;
   cout << "-----------------------------------------------------------" << endl;
   cout << endl;
   cout << "Version:          " << __DATE__ << ", " << __TIME__ << endl;
   if(optUseSCTP) {
      cout << "SCTP:             on" << endl;
   }
   else {
      cout << "SCTP:             off" << endl;
   }
   localAddressArray[0]->setPrintFormat(SocketAddress::PF_Address|SocketAddress::PF_HidePort);
   cout << "Local Addresses:  " << *(localAddressArray[0]) << endl;
   for(cardinal i = 1;i < localAddresses;i++) {
      localAddressArray[i]->setPrintFormat(SocketAddress::PF_Address|SocketAddress::PF_HidePort);
      cout << "                  " << *(localAddressArray[i]) << endl;
   }
   cout << "Server Port:      " << port << endl;
/* ????????????
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",server->getOurSSRC());
   cout << "Server SSRC:      " << str << endl;
*/
   cout << "Client Timeout:   " << (timeout / 1000000) << " [s]" << endl;
   cout << "Input Directory:  " << directory << endl;
   cout << "Max Packet Size:  " << maxPacketSize << endl;
   cout << "Loss Scalability: " << (lossScalability ? "on" : "off") << endl;
   cout << endl;


   // ====== Main loop ======================================================
   for(;;) {
#ifndef FAST_BREAK
      if(breakDetected())
         break;
#endif
      Thread::delay(10000000,true);
   }

   // ====== Clean up =======================================================
   SocketAddress::deleteAddressList(localAddressArray);
   cleanUp(0);
}
