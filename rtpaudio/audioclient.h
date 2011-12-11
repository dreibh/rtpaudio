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


#ifndef AUDIOCLIENT_H
#define AUDIOCLIENT_H


#include "audiowriterinterface.h"
#include "audiodecoderinterface.h"
#include "audiodecoderrepository.h"
#include "mediainfo.h"
#include "rtcpsender.h"
#include "rtpreceiver.h"
#include "internetaddress.h"
#include "tdsocket.h"
#include "strings.h"

#include "audioclientapppacket.h"

#include <map>


/**
  * This class is an audio client.
  *
  * @short   Audio Client
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioClient : virtual public AdjustableAudioQualityInterface
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor for a new audio client.
     *
     * @param audioOutput AudioWriter to write the output to.
     */
   AudioClient(AudioWriterInterface* audioOutput);

   /**
     * Destructor.
     */
   virtual ~AudioClient();


   // ====== Player functions ===============================================
   public:
   /**
     * Start playing given media from given server.
     *
     * @param server Server address (e.g. gaffel:7500).
     * @param mediaName Media name (e.g. ../AudioFiles/Test1.list)
     * @param useSCTP true to use SCTP instead of UDP; false otherwise.
     * @return true, if play request has been sent to server.
     */
   bool play(const char* server,
             const char* mediaName,
             const bool  useSCTP = false);

   /**
     * Change media of an established connection.
     *
     * @param mediaName New media name (e.g. ../AudioFiles/Test2.list)
     *
     * @see play
     */
   void change(const char* mediaName);

   /**
     * Stop playing.
     */
   void stop();


   // ====== Settings =======================================================
   /**
     * Get current media position. This will automatically the
     * RestartPosition value in the next AudioClientAppPacket. The server
     * will restart from the current position, if the server is restarted.
     *
     * @return Position in nanoseconds.
     */
   card64 getPosition();

   /**
     * Get maximum media position.
     *
     * @return Maximum position in nanoseconds.
     */
   inline card64 getMaxPosition() const;


   /**
     * getSamplingRate() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getSamplingRate
     */
   card16 getSamplingRate() const;

   /**
     * getBits() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getBits
     */
   card8 getBits() const;

   /**
     * getChannels() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getChannels
     */
   card8 getChannels() const;


   /**
     * getByteOrder() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getByteOrder
     */
   card16 getByteOrder() const;


   /**
     * getBytesPerSecond() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getBytesPerSecond
     */
   cardinal getBytesPerSecond() const;

   /**
     * getBitsPerSample() Implementation of AudioQualityInterface.
     *
     * @see AudioQualityInterface#getBitsPerSample
     */
   cardinal getBitsPerSample() const;


   /**
     * Get number of raw bytes (incl. IPv6/UDP/RTP/RTPAudio headers) per second.
     *
     * @return Number of raw bytes per second.
     */
   cardinal getRawBytesPerSecond();


   /**
     * Get MediaInfo.
     *
     * @return MediaInfo.
     */
   MediaInfo getMediaInfo() const;


   /**
     * Get error code.
     *
     * @return Error code.
     */
   inline card8 getErrorCode() const;

   /**
     * Get encoding name.
     *
     * @return Encoding name.
     */
   inline const char* getEncoding() const;

   /**
     * Get bandwidth limit.
     *
     * @return Bandwidth limit.
     */
   inline card32 getBandwidthLimit() const;

   /**
     * Get IP version.
     *
     * @return IP Version.
     */
   card8 getIPVersion() const;

   /**
     * Check, if audio client is playing.
     *
     * @return true, if client is playing; false otherwise.
     */
   inline bool playing() const;


   /**
     * Get server address string.
     *
     * @param format Print format.
     * @return Server address.
     *
     * @see InternetAddress#PrintFormat
     */
   String getServerAddressString(const InternetAddress::PrintFormat format = InternetAddress::PF_Address) const;

   /**
     * Get client address string.
     *
     * @param format Print format.
     * @return Client address.
     *
     * @see InternetAddress#PrintFormat
     */
   String getOurAddressString(const InternetAddress::PrintFormat format = InternetAddress::PF_Address) const;


   /**
     * Get number of layers in last transmission.
     */
   inline cardinal getLayers() const;

   /**
     * Get number of bytes received.
     *
     * @param layer Layer number or (cardinal)-1 for sum of all layers.
     * @return Number of bytes received
     */
   inline card64 getBytesReceived(const cardinal layer = 0) const;

   /**
     * Get number of packets received in given layer.
     *
     * @param layer Layer number or (cardinal)-1 for sum of all layers.
     * @return Number of packets received
     *
     * @see getLayers
     */
   inline card64 getPacketsReceived(const cardinal layer = 0) const;

   /**
     * Get InternetFlow of last received packet in given layer.
     *
     * @param layer Layer number.
     * @return InternetFlow.
     *
     * @see getLayers
     */
   inline InternetFlow getInternetFlow(const cardinal layer = 0) const;

   /**
     * Get flow label of last received packet in given layer.
     *
     * @param layer Layer number.
     * @return Flow label.
     *
     * @see getLayers
     */
   inline card32 getFlowLabel(const cardinal layer = 0) const;

   /**
     * Get traffic class of last received packet in given layer.
     *
     * @param layer Layer number.
     * @return Traffic class.
     *
     * @see getLayers
     */
   inline card8 getTrafficClass(const cardinal layer = 0) const;

   /**
     * Get server SSRC for given layer.
     *
     * @param layer Layer number.
     * @return Server SSRC.
     *
     * @see getLayers
     */
   card32 getServerSSRC(const cardinal layer = 0) const;

   /**
     * Get client SSRC.
     *
     * @return Client SSRC.
     */
   inline card32 getOurSSRC() const;

   /**
     * Get number of packets lost for given layer.
     *
     * @param layer Layer number.
     * @return Number of packets lost.
     *
     * @see getLayers
     */
   card64 getPacketsLost(const cardinal layer = 0) const;

   /**
     * Get fraction of packets lost for given layer.
     *
     * @param layer Layer number.
     * @return Fraction of packets lost.
     *
     * @see getLayers
     */
   double getFractionLost(const cardinal layer = 0) const;

   /**
     * Get jitter for given layer.
     *
     * @param layer Layer number.
     * @return Jitter.
     *
     * @see getLayers
     */
   double getJitter(const cardinal layer = 0) const;


   /**
     * Get encoding name for a given index of the client's decoder repository.
     *
     * @param index Repository index.
     * @return Encoding name or NULL, if index is too high.
     */
   const char* getEncodingName(const cardinal index);


   /**
     * Set media position.
     *
     * @param position New media position in nanoseconds.
     */
   inline void setPosition(const card64 position);

   /**
     * Set pause.
     *
     * @param on true for pause on; false for pause off.
     */
   void setPause(const bool on);

   /**
     * Set audio sampling rate.
     *
     * @param rate New audio sampling rate.
     */
   card16 setSamplingRate(const card16 rate);

   /**
     * Set number of audio channels
     *
     * @param channels New number of audio channels.
     */
   card8 setChannels(const card8 channels);

   /**
     * Set number of audio bits.
     *
     * @param bits New number of audio bits.
     */
   card8 setBits(const card8 bits);

   /**
     * Set audio byte order.
     *
     * @param rate New audio byte order.
     */
   card16 setByteOrder(const card16 byteOrder);

   /**
     * Set encoding by index in client's decoder repository.
     *
     * @param index Index in decoder repository.
     */
   void setEncoding(const cardinal index);

   /**
     * Set bandwidth limit.
     *
     * @param bandwidthLimit Bandwidth limit.
     */
   inline void setBandwidthLimit(const card32 bandwidthLimit);


   // ====== Private data ===================================================
   private:
   void sendCommand(const bool updateRestartPosition = true);


   // Update of RestartPosition has to be delayed after change() call to
   // ensure that there are no more packets of the old file on the network
   // which change the restart position back to the old value.
   static const card64 RestartPositionUpdateDelay = 5000000;


   AudioWriterInterface*                                AudioOutput;
   RTPReceiver*                                         Receiver;
   RTCPSender*                                          Sender;
   Socket                                               SenderSocket;
   Socket                                               ReceiverSocket;
   InternetFlow                                         Flow;
   InternetAddress                                      ServerAddress;
   card16                                               OurPort;
   card32                                               OurSSRC;

   std::multimap<const cardinal,AudioDecoderInterface*> DecoderSet;
   AudioDecoderRepository                               Decoders;

   AudioClientAppPacket                                 Status;
   card64                                               OldPosition;
   card64                                               ChangeTimeStamp;
   bool                                                 IsPlaying;
};


#include "audioclient.icc"


#endif
