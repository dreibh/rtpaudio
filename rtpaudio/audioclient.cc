// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client Implementation                                      ####
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
#include "audiodevice.h"
#include "audiodebug.h"
#include "tdsocket.h"
#include "simpleaudiodecoder.h"
#include "advancedaudiodecoder.h"
#include "audiodecoderrepository.h"
#include "rtpreceiver.h"
#include "rtcppacket.h"
#include "rtcpsender.h"
#include "tools.h"
#include "strings.h"
#include "randomizer.h"
#include "ext_socket.h"


#include "audioclientapppacket.h"
#include "audioclient.h"

#include <stdio.h>


namespace Coral {


// Debug mode: Display some information on UDP connections.
// #define DEBUG


// ###### Constructor #######################################################
AudioClient::AudioClient(SocketAddress**       localAddressArray,
                         const cardinal        localAddresses,
                         AudioWriterInterface* audioOutput)
{
   // ====== Default Settings ===============================================
   Status.reset();
   Status.Bits         = AudioQuality::HighestBits;
   Status.Channels     = AudioQuality::HighestChannels;
   Status.SamplingRate = AudioQuality::HighestSamplingRate;
   LocalAddressArray   = localAddressArray;
   LocalAddresses      = localAddresses;
   OurPort             = 0;
   Sender              = NULL;
   Receiver            = NULL;
   AudioOutput         = audioOutput;
   OldPosition         = (card64)-1;
   ChangeTimeStamp     = 0;
   IsPlaying           = false;

   // ====== Initialize decoders ============================================
   SimpleAudioDecoder* simpleAudioDecoder     = new SimpleAudioDecoder(audioOutput);
   AdvancedAudioDecoder* advancedAudioDecoder = new AdvancedAudioDecoder(audioOutput);
   if((simpleAudioDecoder == NULL) || (advancedAudioDecoder == NULL)) {
      cerr << "ERROR: AudioClient::AudioClient() - Out of memory!" << endl;
      exit(1);
   }
   bool a1 = Decoders.addDecoder(advancedAudioDecoder);
   bool a2 = Decoders.addDecoder(simpleAudioDecoder);
   DecoderSet.insert(pair<const cardinal,AudioDecoderInterface*>(0,advancedAudioDecoder));
   DecoderSet.insert(pair<const cardinal,AudioDecoderInterface*>(1,simpleAudioDecoder));
   if((!a1) || (!a2)) {
      cerr << "ERROR: AudioClient::AudioClient() - Out of memory!" << endl;
      exit(1);
   }
   Decoders.activate();
   Status.Encoding = Decoders.getTypeID();
}


// ###### Destructor ########################################################
AudioClient::~AudioClient()
{
   stop();

   // ====== Delete decoders ================================================
   while(DecoderSet.begin() != DecoderSet.end()) {
      multimap<const cardinal,AudioDecoderInterface*>::iterator decoderIterator =
         DecoderSet.begin();
      AudioDecoderInterface* decoder = decoderIterator->second;
      DecoderSet.erase(decoderIterator);
      Decoders.removeDecoder(decoder);
      delete decoder;
   }
}


// ###### Change media ######################################################
void AudioClient::change(const char* mediaName)
{
   if(Sender != NULL) {
      strncpy((char*)&Status.MediaName,mediaName,sizeof(Status.MediaName));
      Status.StartPosition   = 0;
      Status.RestartPosition = 0;
      ChangeTimeStamp        = getMicroTime();
      sendCommand(false);
   }
   else {
      cerr << "ERROR: AudioClient::change() - No connection!" << endl;
   }
}


// ###### Start playing #####################################################
bool AudioClient::play(const char* server,
                       const char* mediaName,
                       const bool  useSCTP)
{
   if(Sender == NULL) {
      // ====== Set default settings ========================================
      Status.FormatID        = AudioClientAppPacket::AudioClientFormatID;
      Status.SequenceNumber  = 0;
      Status.PosChgSeqNumber = 0;
      Status.Status          = AudioClientAppPacket::ACAS_Play;
      Status.StartPosition   = (card64)-1;
      Status.RestartPosition = 0;
      strncpy((char*)&Status.MediaName,mediaName,sizeof(Status.MediaName));
      OldPosition     = (card64)-1;
      ChangeTimeStamp = 0;
      Randomizer random;
      OurSSRC = random.random32();
      Decoders.reset();


      // ====== Get server address ==========================================
      ServerAddress = InternetAddress(server);
      if(!ServerAddress.isValid()) {
         stop();
         return(false);
      }
      if(ServerAddress.getPort() == 0) {
         ServerAddress.setPort(AudioClientAppPacket::RTPAudioDefaultPort);
      }


      // ====== Create sockets ==============================================
      SenderSocket.create(Socket::IP,Socket::UDP,useSCTP ? Socket::SCTP : Socket::Default);
      if(!SenderSocket.ready()) {
         cerr << "ERROR: AudioClient::play() - "
                 "Unable to bind socket for RTCPSender!" << endl;
         stop();
         return(false);
      }
      ReceiverSocket.create(Socket::IP,Socket::UDP,useSCTP ? Socket::SCTP : Socket::Default);
      if(!ReceiverSocket.ready()) {
         cerr << "ERROR: AudioClient::play() - "
                 "Unable to bind socket for RTPReceiver!" << endl;
         stop();
         return(false);
      }
      if(!Socket::bindxSocketPair(SenderSocket,
                                  ReceiverSocket,
                                  (const SocketAddress**)LocalAddressArray,
                                  LocalAddresses,
                                  SCTP_BINDX_ADD_ADDR)) {
         cerr << "ERROR: AudioClient::play() - Unable to bind sockets!" << endl;
         stop();
         return(false);
      }


      // ====== SCTP Events =================================================
      if(useSCTP) {
         struct sctp_event_subscribe events;

         memset((char*)&events, 0 ,sizeof(events));
         if(SenderSocket.setSocketOption(IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events)) < 0) {
            cerr << "WARNING: AudioClient::play() - SCTP_EVENTS failed!" << endl;
         }
         sctp_initmsg init;
         init.sinit_num_ostreams   = 1;
         init.sinit_max_instreams  = 16;
         init.sinit_max_attempts   = 0;
         init.sinit_max_init_timeo = 60;
         if(SenderSocket.setSocketOption(IPPROTO_SCTP,SCTP_INITMSG,(char*)&init,sizeof(init)) < 0) {
            cerr << "WARNING: AudioClient::play() - Unable to set SCTP_INITMSG parameters!" << endl;
         }
      }


      // ====== Connect sender socket to server =============================
      Flow = SenderSocket.allocFlow(ServerAddress);
      if(Flow.getFlowLabel() != 0) {
         if(SenderSocket.connect(Flow,AudioClientDefaultTrafficClass) == 0) {
            if(SenderSocket.connect(ServerAddress,AudioClientDefaultTrafficClass) == false) {
               cerr << "ERROR: AudioClient::play() - Unable to connect socket for RTCPSender!" << endl;
               stop();
               return(false);
            }
         }
      }
      else {
         if(SenderSocket.connect(ServerAddress,AudioClientDefaultTrafficClass) == false) {
            cerr << "ERROR: AudioClient::play() - Unable to connect socket for RTCPSender!" << endl;
            stop();
            return(false);
         }
      }


      // ====== Create RTPReceiver ==========================================
      Receiver = new RTPReceiver(&Decoders,&ReceiverSocket);
      if(Receiver == NULL) {
         cerr << "ERROR: AudioClient::play() - Out of memory!" << endl;
         stop();
         return(false);
      }
      if(Receiver->start() == false) {
         cerr << "ERROR: AudioClient::play() - Unable to start RTP receiver thread!" << endl;
         stop();
         return(false);
      }


      // ====== Create RTCPSender ===========================================
      Sender = new RTCPSender(OurSSRC,&SenderSocket,Receiver,3000);
      if(Sender == NULL) {
         cerr << "ERROR: AudioClient::play() - Out of memory!" << endl;
         stop();
         return(false);
      }


      // ====== Add SDES items ==============================================
      char host[128];
      if(InternetAddress::getFullHostName((char*)&host,sizeof(host)) == false) {
         host[0] = 0x00;
      }
      char user[100];
      if(getUserName((char*)&user,sizeof(user)) == false) {
         strcpy((char*)&user,"nobody");
      }
      char cname[300];
      snprintf((char*)&cname,sizeof(cname),"%s@%s",user,host);
      if(Sender->addSDESItem(RTCP_SDES_CNAME,cname) == false) {
         cerr << "ERROR: AudioClient::play() - Out of memory!" << endl;
         stop();
         return(false);
      }

/*
      Sender->addSDESItem(RTCP_SDES_PHONE,"+49 1234 56789");
      Sender->addSDESItem(RTCP_SDES_LOC,"Wiehl-Forst, Germany");
      Sender->addSDESItem(RTCP_SDES_EMAIL,"email@pop3.domain.xy");
*/
      Sender->sendSDES();

      if(Sender->start() == false) {
         cerr << "ERROR: AudioClient::play() - Unable to start RTCP sender thread!" << endl;
         stop();
         return(false);
      }
      IsPlaying = true;
      sendCommand();

      // ====== Get client's local address =====================================
//
      // OurPort = receiverAddress.getPort();
      OurPort = 0;

      // ====== Print information ==============================================
#ifdef DEBUG
      cout << "Connecting to audio server at " << ServerAddress << "." << endl;
      char str[128];
      if(SenderSocket.getSendFlowLabel()) {
         snprintf((char*)&str,sizeof(str),"$%05x, traffic class $%02x.",
                 SenderSocket.getSendFlowLabel(),SenderSocket.getSendTrafficClass());
         cout << "   => IPv6 flow label is " << str << endl;
      }
      else if(SenderSocket.getSendTrafficClass()) {
         snprintf((char*)&str,sizeof(str),"$%02x.",SenderSocket.getSendTrafficClass());
         cout << "   => TOS is " << str << endl;
      }
#endif
   }
   return(IsPlaying);
}


// ###### Stop playing ######################################################
void AudioClient::stop()
{
   IsPlaying = false;
   if(Sender != NULL) {
      Sender->sendBye();
      Sender->stop();
      if(Flow.getFlowLabel() !=0) {
         SenderSocket.freeFlow(Flow);
      }
      Sender = NULL;
   }
   if(Receiver != NULL) {
      Receiver->stop();
      delete Receiver;
      Receiver = NULL;
   }
   SenderSocket.close();
   ReceiverSocket.close();
   ServerAddress.reset();
   Decoders.reset();
   OurPort = 0;
   AudioOutput->sync();
   OldPosition     = (card64)-1;
   ChangeTimeStamp = 0;
}


// ###### Begin/end pause mode ##############################################
void AudioClient::setPause(const bool on)
{
   if(on) {
      Status.Status = AudioClientAppPacket::ACAS_Pause;
   }
   else {
      Status.Status = AudioClientAppPacket::ACAS_Play;
   }
   if(IsPlaying) sendCommand();
}


// ###### Send command to server ############################################
void AudioClient::sendCommand(const bool updateRestartPosition)
{
   if(IsPlaying) {
      Status.SequenceNumber  = Status.SequenceNumber + 1;

      if(updateRestartPosition == true)
         getPosition(); // This will update RestartPosition!

      AudioClientAppPacket app;
      app = Status;
      app.translate();
      Sender->sendApp("HELO",(void *)&app,sizeof(AudioClientAppPacket));
      if(Sender->addSDESItem(RTCP_SDES_PRIV,(char*)&app,sizeof(AudioClientAppPacket)) == false) {
         cerr << "ERROR: Unable to add SDES - Out of memory!" << endl;
      }
   }
}


// ###### Get current frame number ##########################################
card64 AudioClient::getPosition()
{
   if(IsPlaying) {
      card64 position = Decoders.getPosition();

      if(position != OldPosition) {
         if((ChangeTimeStamp == 0) || (getMicroTime() - ChangeTimeStamp > RestartPositionUpdateDelay)) {
            OldPosition     = position;
            ChangeTimeStamp = 0;
            Status.RestartPosition   = position;
            AudioClientAppPacket app = Status;
            app.translate();
            if(Sender->addSDESItem(RTCP_SDES_PRIV,(char*)&app,sizeof(AudioClientAppPacket)) == false) {
               cerr << "ERROR: Unable to add SDES - Out of memory!" << endl;
            }
         }
      }
      return(position);
   }
   return(0);
}


// ###### Set encoder #######################################################
void AudioClient::setEncoding(const cardinal index)
{
   multimap<const cardinal,AudioDecoderInterface*>::iterator decoderIterator =
      DecoderSet.find(index);
   if(decoderIterator != DecoderSet.end()) {
      AudioDecoderInterface* decoder = decoderIterator->second;
      Status.Encoding = decoder->getTypeID();
      if(Receiver) Receiver->synchronized();
      Decoders.selectDecoderForTypeID(decoder->getTypeID());
      if(Receiver) Receiver->unsynchronized();
      sendCommand();
   }
}


// ###### Get media info ####################################################
MediaInfo AudioClient::getMediaInfo() const
{
   MediaInfo mediaInfo;
   if(IsPlaying) {
      Decoders.getMediaInfo(mediaInfo);
   }
   else {
      mediaInfo.reset();
   }
   return(mediaInfo);
}


// ###### Get sampling rate #################################################
card16 AudioClient::getSamplingRate() const
{
   if(IsPlaying) return(Decoders.getSamplingRate());
   return(0);
}


// ###### Get number of channels ############################################
card8 AudioClient::getChannels() const
{
   if(IsPlaying) return(Decoders.getChannels());
   return(0);
}


// ###### Get number of bits ################################################
card8 AudioClient::getBits() const
{
   if(IsPlaying) return(Decoders.getBits());
   return(0);
}


// ###### Get byte order ####################################################
card16 AudioClient::getByteOrder() const
{
   if(IsPlaying) return(Decoders.getByteOrder());
   return(BYTE_ORDER);
}


// ###### Get bytes per second ##############################################
cardinal AudioClient::getBytesPerSecond() const
{
   if(IsPlaying) return(Decoders.getBytesPerSecond());
   return(0);
}


// ###### Get bits per sample ###############################################
cardinal AudioClient::getBitsPerSample() const
{
   if(IsPlaying) return(Decoders.getBitsPerSample());
   return(0);
}


// ###### Get server SSRC ###################################################
card32 AudioClient::getServerSSRC(const cardinal layer) const
{
   if(IsPlaying) {
      SourceStateInfo ssi = Receiver->getSSI(layer);
      return(ssi.getSSRC());
   }
   return(0);
}


// ###### Get packets lost ##################################################
card64 AudioClient::getPacketsLost(const cardinal layer) const
{
   if(IsPlaying) {
      SourceStateInfo ssi = Receiver->getSSI(layer);
      return(ssi.getPacketsLost());
   }
   return(0);
}


// ###### Get fraction lost #################################################
double AudioClient::getFractionLost(const cardinal layer) const
{
   if(IsPlaying) {
      SourceStateInfo ssi = Receiver->getSSI(layer);
      return(ssi.getFractionLost());
   }
   return(0.0);
}


// ###### Get jitter ########################################################
double AudioClient::getJitter(const cardinal layer) const
{
   if(IsPlaying) {
      SourceStateInfo ssi = Receiver->getSSI(layer);
      return(ssi.getJitter());
   }
   return(0.0);
}


// ###### Get server address ################################################
String AudioClient::getServerAddressString(const InternetAddress::PrintFormat format) const
{
   if(IsPlaying) {
      InternetAddress address = ServerAddress;
      address.setPrintFormat(format);
      return(address.getAddressString());
   }
   else {
      return("N/A");
   }
}


// ###### Get client address ################################################
String AudioClient::getOurAddressString(const InternetAddress::PrintFormat format) const
{
/* ?????????
   if(IsPlaying) {
      InternetAddress address = OurAddress;
      address.setPrintFormat(format);
      return(address.getAddressString());
   }
   else {
*/
      return("N/A");
   //}
}


// ###### Get encoding name #################################################
const char* AudioClient::getEncodingName(const cardinal index)
{
   multimap<const cardinal,AudioDecoderInterface*>::iterator decoderIterator =
      DecoderSet.find(index);
   if(decoderIterator != DecoderSet.end()) {
      AudioDecoderInterface* decoder = decoderIterator->second;
      return(decoder->getTypeName());
   }
   return(NULL);
}


// ###### Get number of raw bytes per second ################################
cardinal AudioClient::getRawBytesPerSecond()
{
   // ??????????????????????????????
   return(0);
}


// ###### Set sampling rate #################################################
card16 AudioClient::setSamplingRate(const card16 rate)
{
   Status.SamplingRate = rate;
   sendCommand();
   return(rate);
}


// ###### Set number of channels ############################################
card8 AudioClient::setChannels(const card8 channels)
{
   Status.Channels = channels;
   sendCommand();
   return(channels);
}


// ###### Set number of bits ################################################
card8 AudioClient::setBits(const card8 bits)
{
   Status.Bits = bits;
   sendCommand();
   return(bits);
}


// ###### Set byte order ####################################################
card16 AudioClient::setByteOrder(const card16 byteOrder)
{
   return(BYTE_ORDER);
}


}
