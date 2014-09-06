// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTP Packet                                                       ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2014 by Thomas Dreibholz            ####
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


#ifndef RTPPACKET_H
#define RTPPACKET_H


#include "tdsystem.h"
#include "tdsocket.h"


namespace RTPConstants {


/**
  * RTP maximum payload limit.
  *
  */
const cardinal RTPMaxPayloadLimit = 8192;

/**
  * RTP default maximum payload is 1376
  * = 1500 - (12 + 16 * 4) - 40 - 8
  * = Maximum ethernet data length - RTPPacket size - (16 * CSRC) - UDP header size - IPv6 header size.
  */
const cardinal RTPDefaultMaxPayload = 1376;

/**
  * Default RTP header size (CC = 0).
  */
const cardinal RTPDefaultHeaderSize = 12;

/**
  * Constant for RTP version.
  */
const card8 RTPVersion = 2;

/**
  * Constant for microseconds per RTP timestamp.
  */
const double RTPMicroSecondsPerTimeStamp = 1000.0 / 16.0;


#ifdef USE_TRANSPORTINFO

/**
  * Maximum number of layers in one stream.
  * Note: This is *not* a constant of RFC 1889 but a limit for the
  * RTP structs!
  */
const cardinal RTPMaxQualityLayers = 4;

#else

/**
  * Maximum number of layers in one stream.
  * Note: This is *not* a constant of RFC 1889 but a limit for the
  * RTP structs!
  */
const cardinal RTPMaxQualityLayers = 16;

#endif


}


/**
  * This struct manages an RTP packet
  *
  * @short   RTP Packet
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  *
  * @see RTPSender
  * @see RTPReceiver
  */
struct RTPPacket
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTPPacket();


   // ====== Get methods ====================================================
   /**
     * Get version.
     *
     * @return RTP Version.
     */
   inline card8 getVersion() const;

   /**
     * Get padding.
     *
     * @return RTP Padding.
     */
   inline card8 getPadding() const;

   /**
     * Get extension.
     *
     * @return RTP Extension.
     */
   inline card8 getExtension() const;

   /**
     * Get CSRC count.
     *
     * @return RTP CSRC count.
     */
   inline card8 getCSRCCount() const;

   /**
     * Get marker.
     *
     * @return RTP Marker.
     */
   inline bool getMarker() const;

   /**
     * Get payload type.
     *
     * @return RTP Payload type.
     */
   inline card8 getPayloadType() const;

   /**
     * Get sequence number.
     *
     * @return RTP Sequence number.
     */
   inline card16 getSequenceNumber() const;

   /**
     * Get time stamp.
     *
     * @return RTP Time stamp.
     */
   inline card32 getTimeStamp() const;

   /**
     * Get SSRC.
     *
     * @return RTP SSRC.
     */
   inline card32 getSSRC() const;

   /**
     * Get CSRC at given index.
     *
     * @param index Index.
     * @return RTP CSRC.
     */
   inline card32 getCSRC(cardinal index) const;


   /**
     * Calculate header size.
     *
     * @return Header size.
     */
   inline cardinal calculateHeaderSize() const;

   /**
     * Get pointer to payload data.
     *
     * @return pointer to payload data.
     */
   inline char* getPayloadData() const;

   /**
     * Get maximum payload size.
     *
     * @return Maximum payload size.
     */
   inline cardinal getMaxPayloadSize() const;


   // ====== Set methods ====================================================
   /**
     * Set version.
     *
     * @param version RTP Version.
     */
   inline void setVersion(const card8 version);

   /**
     * Set padding.
     *
     * @param padding RTP Padding.
     */
   inline void setPadding(const card8 padding);

   /**
     * Set extension.
     *
     * @param extension RTP Extension.
     */
   inline void setExtension(const card8 extension);

   /**
     * Set CSRC count.
     *
     * @param count RTP CSRC count.
     */
   inline void setCSRCCount(const card8 count);

   /**
     * Set marker.
     *
     * @param marker RTP Marker.
     */
   inline void setMarker(const bool marker);

   /**
     * Set payload type.
     *
     * @param payloadType RTP Payload type.
     */
   inline void setPayloadType(const card8 payloadType);

   /**
     * Set sequence number.
     *
     * @param sequenceNumber RTP Sequence number.
     */
   inline void setSequenceNumber(const card16 sequenceNumber);

   /**
     * Set time stamp.
     *
     * @param timeStamp RTP timeStamp.
     */
   inline void setTimeStamp(const card32 timeStamp);

   /**
     * Set SSRC.
     *
     * @param ssrc RTP SSRC.
     */
   inline void setSSRC(const card32 ssrc);

   /**
     * Set CSRC at given index.
     *
     * @param index Index.
     * @param csrc CSRC.
     */
   inline void setCSRC(const cardinal index, const card32 csrc);


   /**
     * Output operator.
     */
   friend std::ostream& operator<<(std::ostream& os, const RTPPacket& packet);


   // ====== Private data ===================================================
   private:
#if BYTE_ORDER == BIG_ENDIAN
   card8 V:2;                       // Version
   card8 P:1;                       // Padding
   card8 X:1;                       // Extension
   card8 CC:4;                      // CSRC Count
   card8 M:1;                       // Marker
   card8 PT:7;                      // Payload Type
#elif  BYTE_ORDER == LITTLE_ENDIAN
   card8 CC:4;                      // CSRC Count
   card8 X:1;                       // Extension
   card8 P:1;                       // Padding
   card8 V:2;                       // Version
   card8 PT:7;                      // Payload Type
   card8 M:1;                       // Marker
#else
#error "Unknown BYTE_ORDER setting!"
#endif
   card16 SequenceNumber;           // Sequence number
   card32 TimeStamp;                // TimeStamp
   card32 SSRC;                     // Synchronization Source (SSRC) identifier
   card32 CSRC[16];                 // Contributing Source (CSRC) identifiers
   char   Data[RTPConstants::RTPMaxPayloadLimit];   // Payload data
} __attribute__((packed));


#include "rtppacket.icc"


#endif
