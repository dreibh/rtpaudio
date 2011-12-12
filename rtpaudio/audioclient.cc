// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client Implementation                                      ####
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


// Debug mode: Display some information on UDP connections.
#define DEBUG


// ###### Constructor #######################################################
AudioClient::AudioClient(AudioWriterInterface* audioOutput)
{
   // ====== Default Settings ===============================================
   Status.reset();
   Status.Bits         = AudioQuality::HighestBits;
   Status.Channels     = AudioQuality::HighestChannels;
   Status.SamplingRate = AudioQuality::HighestSamplingRate;
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
      std::cerr << "ERROR: AudioClient::AudioClient() - Out of memory!" << std::endl;
      exit(1);
   }
   bool a1 = Decoders.addDecoder(advancedAudioDecoder);
   bool a2 = Decoders.addDecoder(simpleAudioDecoder);
   DecoderSet.insert(std::pair<const cardinal,AudioDecoderInterface*>(0,advancedAudioDecoder));
   DecoderSet.insert(std::pair<const cardinal,AudioDecoderInterface*>(1,simpleAudioDecoder));
   if((!a1) || (!a2)) {
      std::cerr << "ERROR: AudioClient::AudioClient() - Out of memory!" << std::endl;
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
      std::multimap<const cardinal,AudioDecoderInterface*>::iterator decoderIterator =
         DecoderSet.begin();
      AudioDecoderInterface* decoder = decoderIterator->second;
      DecoderSet.erase(decoderIterator);
      Decoders.removeDecoder(decoder);
      delete decoder;
   }
}


// ###### Change media ######################################################
bool AudioClient::change(const char* url)
{
   String protocol;
   String server;
   String mediaName;

   if(scanURL(url,protocol,server,mediaName) == true) {
      if(Sender != NULL) {
         strncpy((char*)&Status.MediaName,mediaName.getData(),sizeof(Status.MediaName));
         Status.StartPosition   = 0;
         Status.RestartPosition = 0;
         ChangeTimeStamp        = getMicroTime();
         sendCommand(false);
         return(true);
      }
      else {
         std::cerr << "ERROR: AudioClient::change() - No connection!" << std::endl;
      }
   }

   return(false);
}


// ###### Start playing #####################################################
bool AudioClient::play(const char* url)
{
   String protocol;
   String server;
   String mediaName;

   if(scanURL(url,protocol,server,mediaName) == false) {
      return(false);
   }

   bool useSCTP;
   protocol = protocol.toLower();
   if(protocol == "rtpa") {
      useSCTP = false;
   }
   else if(protocol == "rtpa+udp") {
      useSCTP = false;
   }
   else if(protocol == "rtpa+sctp") {
      useSCTP = true;
   }
   else {
      return(false);
   }

   if(Sender == NULL) {
      // ====== Set default settings ========================================
      Status.FormatID        = AudioClientAppPacket::AudioClientFormatID;
      Status.SequenceNumber  = 0;
      Status.PosChgSeqNumber = 0;
      Status.Status          = AudioClientAppPacket::ACAS_Play;
      Status.StartPosition   = (card64)-1;
      Status.RestartPosition = 0;
      strncpy((char*)&Status.MediaName,mediaName.getData(),sizeof(Status.MediaName));
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
      SenderSocket.create(Socket::IP,
                          useSCTP ? Socket::Stream : Socket::Datagram,
                          useSCTP ? Socket::SCTP : Socket::Default);
      if(!SenderSocket.ready()) {
         std::cerr << "ERROR: AudioClient::play() - "
                 "Unable to bind socket for RTCPSender!" << std::endl;
         stop();
         return(false);
      }
      ReceiverSocket.create(Socket::IP,
                            useSCTP ? Socket::SeqPacket : Socket::Datagram,
                            useSCTP ? Socket::SCTP : Socket::Default);
      if(!ReceiverSocket.ready()) {
         std::cerr << "ERROR: AudioClient::play() - "
                 "Unable to bind socket for RTPReceiver!" << std::endl;
         stop();
         return(false);
      }
      InternetAddress localAddress(0);
      if(!Socket::bindSocketPair(SenderSocket, ReceiverSocket, localAddress)) {
         std::cerr << "ERROR: AudioClient::play() - Unable to bind sockets!" << std::endl;
         stop();
         return(false);
      }
      if(useSCTP) {
         ReceiverSocket.listen(1);
      }


      // ====== Connect sender socket to server =============================
      Flow = SenderSocket.allocFlow(ServerAddress);
      if(Flow.getFlowLabel() != 0) {
         if(SenderSocket.connect(Flow,AudioClientDefaultTrafficClass) == 0) {
            if(SenderSocket.connect(ServerAddress,AudioClientDefaultTrafficClass) == false) {
               std::cerr << "ERROR: AudioClient::play() - Unable to connect socket for RTCPSender!" << std::endl;
               stop();
               return(false);
            }
         }
      }
      else {
         if(SenderSocket.connect(ServerAddress,AudioClientDefaultTrafficClass) == false) {
            std::cerr << "ERROR: AudioClient::play() - Unable to connect socket for RTCPSender!" << std::endl;
            stop();
            return(false);
         }
      }


      // ====== Create RTPReceiver ==========================================
      Receiver = new RTPReceiver(&Decoders,&ReceiverSocket);
      if(Receiver == NULL) {
         std::cerr << "ERROR: AudioClient::play() - Out of memory!" << std::endl;
         stop();
         return(false);
      }
      if(Receiver->start() == false) {
         std::cerr << "ERROR: AudioClient::play() - Unable to start RTP receiver thread!" << std::endl;
         stop();
         return(false);
      }


      // ====== Create RTCPSender ===========================================
      Sender = new RTCPSender(OurSSRC,&SenderSocket,Receiver,3000);
      if(Sender == NULL) {
         std::cerr << "ERROR: AudioClient::play() - Out of memory!" << std::endl;
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
         std::cerr << "ERROR: AudioClient::play() - Out of memory!" << std::endl;
         stop();
         return(false);
      }

/*
      Sender->addSDESItem(RTCP_SDES_PHONE,"+49 1234 56789");
      Sender->addSDESItem(RTCP_SDES_LOC,"Essen, Germany");
      Sender->addSDESItem(RTCP_SDES_EMAIL,"email@domain.xy");
*/
      Sender->sendSDES();

      if(Sender->start() == false) {
         std::cerr << "ERROR: AudioClient::play() - Unable to start RTCP sender thread!" << std::endl;
         stop();
         return(false);
      }
      IsPlaying = true;
      sendCommand();

      // ====== Get client's local address =====================================
      // Get our local address from the server's view.
      SenderSocket.getSocketAddress(OurAddress);
      InternetAddress receiverAddress;
      ReceiverSocket.getSocketAddress(receiverAddress);
      OurAddress.setPort(receiverAddress.getPort());

      // ====== Print information ==============================================
#ifdef DEBUG
      std::cout << "Connecting to audio server at " << ServerAddress << "." << std::endl;
      char str[128];
      if(SenderSocket.getSendFlowLabel()) {
         snprintf((char*)&str,sizeof(str),"$%05x, traffic class $%02x.",
                 SenderSocket.getSendFlowLabel(),SenderSocket.getSendTrafficClass());
         std::cout << "   => IPv6 flow label is " << str << std::endl;
      }
      else if(SenderSocket.getSendTrafficClass()) {
         snprintf((char*)&str,sizeof(str),"$%02x.",SenderSocket.getSendTrafficClass());
         std::cout << "   => TOS is " << str << std::endl;
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
   OurAddress.reset();
   ServerAddress.reset();
   Decoders.reset();
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
      Sender->sendApp("HELO",(void*)&app,sizeof(AudioClientAppPacket));
      if(Sender->addSDESItem(RTCP_SDES_PRIV,(char*)&app,sizeof(AudioClientAppPacket)) == false) {
         std::cerr << "ERROR: Unable to add SDES - Out of memory!" << std::endl;
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
               std::cerr << "ERROR: Unable to add SDES - Out of memory!" << std::endl;
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
   std::multimap<const cardinal,AudioDecoderInterface*>::iterator decoderIterator =
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
   if(IsPlaying) {
      InternetAddress address = OurAddress;
      address.setPrintFormat(format);
      return(address.getAddressString());
   }
   else {

      return("N/A");
   }
}


// ###### Get encoding name #################################################
const char* AudioClient::getEncodingName(const cardinal index)
{
   std::multimap<const cardinal,AudioDecoderInterface*>::iterator decoderIterator =
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
   // ???
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
