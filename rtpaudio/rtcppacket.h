// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### RTCP Packet                                                      ####
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


#ifndef RTCPPACKET_H
#define RTCPPACKET_H


#include "tdsystem.h"
#include "rtppacket.h"


/**
  * Definition of RTCP message types.
  */
enum RTCP_Type
{
  RTCP_SR   = 200,
  RTCP_RR   = 201,
  RTCP_SDES = 202,
  RTCP_BYE  = 203,
  RTCP_APP  = 204
};


/**
  * Definition of RTCP SDES message types.
  */
enum RTCP_SDES_Type
{
  RTCP_SDES_END   = 0,
  RTCP_SDES_CNAME = 1,
  RTCP_SDES_NAME  = 2,
  RTCP_SDES_EMAIL = 3,
  RTCP_SDES_PHONE = 4,
  RTCP_SDES_LOC   = 5,
  RTCP_SDES_TOOL  = 6,
  RTCP_SDES_NOTE  = 7,
  RTCP_SDES_PRIV  = 8
};


/**
  * This struct manages a common RTCP header.
  *
  * @short   RTCP Common Header
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPCommonHeader
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPCommonHeader();


   // ====== Get methods ====================================================
   /**
     * Get version.
     *
     * @return RTCP version.
     */
   inline card8 getVersion() const;

   /**
     * Get padding.
     *
     * @return RTCP padding.
     */
   inline card8 getPadding() const;

   /**
     * Get count.
     *
     * @return RTCP count.
     */
   inline card8 getCount() const;

   /**
     * Get packet type.
     *
     * @return RTCP packet type.
     */
   inline card8 getPacketType() const;

   /**
     * Get length.
     *
     * @return RTCP Length.
     */
   inline card16 getLength() const;


   // ====== Set methods ====================================================
   /**
     * Set version.
     *
     * @param version RTCP version.
     */
   inline void setVersion(const card8 version);

   /**
     * Set padding.
     *
     * @param padding RTCP padding.
     */
   inline void setPadding(const card8 padding);

   /**
     * Set count.
     *
     * @param count RTCP count.
     */
   inline void setCount(const card8 count);

   /**
     * Set packetType.
     *
     * @param packetType RTCP packet Type.
     */
   inline void setPacketType(const card8 packetType);

   /**
     * Set length.
     *
     * @param length RTCP Length.
     */
   inline void setLength(const card16 length);


   // ====== Protected data =================================================
   protected:
#if __BYTE_ORDER == __BIG_ENDIAN
   card8 V:2;		                  // Protocol Version
   card8 P:1;		                  // Padding Flag
   card8 C:5;		                  // Count varies by Packet Type (RC/SC)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
   card8 C:5;		                  // Count varies by Packet Type (RC/SC)
   card8 P:1;		                  // Padding Flag
   card8 V:2;		                  // Protocol Version
#else
#error "Unknown CPU_BYTEORDER setting!"
#endif
   card8 PT:8;		                  // RTCP Packet Type
   card16 Length;                         // Packet length in words minus one
} __attribute__((packed));


/**
  * This struct manages a sender info block
  *
  * @short   RTCP Sender Info Block
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPSenderInfoBlock
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPSenderInfoBlock();


   // ====== Get methods ====================================================
   /**
     * Get NTP timestamp.
     *
     * @return NTP timestamp.
     */
   inline card64 getNTPTimeStamp() const;

   /**
     * Get RTP time stamp.
     *
     * @return RTP time stamp.
     */
   inline card32 getRTPTimeStamp() const;


   /**
     * Get packets sent.
     *
     * @return Packets sent.
     */
   inline card32 getPacketsSent() const;

   /**
     * Get octets sent.
     *
     * @return Octets sent.
     */
   inline card32 getOctetsSent() const;


   // ====== Set methods ====================================================
   /**
     * Set NTP timestamp.
     *
     * @param timeStamp NTP timestamp.
     */
   inline void setNTPTimeStamp(const card64 timeStamp);

   /**
     * Set RTP time stamp.
     *
     * @param timeStamp RTP timestamp.
     */
   inline void setRTPTimeStamp(const card32 timeStamp);

   /**
     * Set packets sent.
     *
     * @param packets Packets sent.
     */
   inline void setPacketsSent(const card32 packets);

   /**
     * Set octets sent.
     *
     * @param octets Octets sent.
     */
   inline void setOctetsSent(const card32 octets);


   // ====== Protected data =================================================
   protected:
   card32 NTP_MostSignificant;     // NTP Timestamp (most  significant word)
   card32 NTP_LeastSignificant;    // NTP Timestamp (least significant word)
   card32 RTPTimeStamp;            // RTP Timestamp
   card32 PacketsSent;             // Packets sent
   card32 OctetsSent;              // Octets sent
} __attribute__((packed));


/**
  * This struct manages a reception report block
  *
  * @short   RTCP Reception Report Block
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPReceptionReportBlock
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPReceptionReportBlock();

   /**
     * Constructor.
     *
     * @param ssrc SSRC.
     */
   RTCPReceptionReportBlock(const card32 ssrc);


   /**
     * Initialize.
     *
     * @param ssrc SSRC.
     */
   void init(const card32 ssrc);


   // ====== Get methods ====================================================
   /**
     * Get SSRC.
     *
     * @return SSRC.
     */
   inline card32 getSSRC() const;

   /**
     * Get fraction lost.
     *
     * @return Fraction lost.
     */
   inline double getFractionLost() const;

   /**
     * Get packets lost.
     *
     * @return Packets lost.
     */
   inline card32 getPacketsLost() const;

   /**
     * Get last sequence number.
     *
     * @return Last sequence number.
     */
   inline card32 getLastSeqNum() const;

   /**
     * Get jitter.
     *
     * @return Jitter.
     */
   inline card32 getJitter() const;

   /**
     * Get LSR.
     *
     * @return LSR.
     */
   inline card32 getLSR() const;

   /**
     * Get DLSR.
     *
     * @return DLSR.
     */
   inline card32 getDLSR() const;


   // ====== Set methods ====================================================
   /**
     * Set SSRC.
     *
     * @param ssrc SSRC.
     */
   inline void setSSRC(card32 ssrc);

   /**
     * Set fraction lost.
     *
     * @param fraction Fraction lost.
     */
   inline void setFractionLost(const double fraction);

   /**
     * Set packets lost.
     *
     * @param packetsLost Packets lost.
     */
   inline void setPacketsLost(const card32 packetsLost);

   /**
     * Set last sequence number.
     *
     * @param lastSeq Last sequence number.
     */
   inline void setLastSeqNum(const card32 lastSeq);

   /**
     * Set jitter.
     *
     * @return jitter Jitter.
     */
   inline void setJitter(const card32 jitter);

   /**
     * Set LSR.
     *
     * @param lsr LSR.
     */
   inline void setLSR(const card32 lsr);

   /**
     * Set DLSR.
     *
     * @param dlsr DLSR.
     */
   inline void setDLSR(const card32 dlsr);


   // ====== Protected data =================================================
   protected:
   card32 SSRC;                           // Data Source being reported
   card32 Fraction:8;                     // Fraction lost since last SR/RR
   card32 Lost:24;                        // Cumulative no. of Packets lost (signed!)
   card32 LastSeq;                        // Extended last sequence no. received
   card32 Jitter;                         // Interarrival Jitter
   card32 LSR;                            // Last SR Packet from this source
   card32 DLSR;                           // Delay since last SR Packet
} __attribute__((packed));


/**
  * This struct manages an RTCP report.
  *
  * @short   RTCP Report
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPReport : public RTCPCommonHeader
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPReport();


   // ====== Get methods ====================================================
   /**
     * Get SSRC.
     *
     * @return SSRC.
     */
   inline card32 getSSRC() const;


   // ====== Set methods ====================================================
   /**
     * Set SSRC.
     *
     * @param ssrc SSRC.
     */
   inline void setSSRC(const card32 ssrc);


   // ====== Protected data =================================================
   protected:
   card32 SSRC;                  // Sender or Receiver generating this report
} __attribute__((packed));


/**
  * This struct manages an RTCP sender report
  *
  * @short   RTCP Sender Report
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPSenderReport : public RTCPReport, public RTCPSenderInfoBlock
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPSenderReport();

   /**
     * Constructor.
     *
     * @param syncSource SSRC.
     * @param count Count.
     */
   RTCPSenderReport(const card32 syncSource, const card8 count = 0);


   /**
     * Initialize.
     *
     * @param syncSource SSRC.
     * @param count Count.
     */
   void init(const card32 syncSource, const card8 count = 0);


   /**
     * Array of RTCPReceptionReportBlocks
     */
   RTCPReceptionReportBlock rr[];      // Variable length RR list
} __attribute__((packed));


/**
  * This struct manages an RTCP receiver report
  *
  * @short   RTCP Sender Report
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPReceiverReport : public RTCPReport
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPReceiverReport();

   /**
     * Constructor.
     *
     * @param syncSource SSRC.
     * @param count Count.
     */
   RTCPReceiverReport(const card32 syncSource, const card8 count = 0);


   /**
     * Initialize.
     *
     * @param syncSource SSRC.
     * @param count Count.
     */
   void init(const card32 syncSource, const card8 count = 0);


   /**
     * Array of RTCPReceptionReportBlocks
     */
   RTCPReceptionReportBlock rr[];      // Variable length RR list
} __attribute__((packed));


/**
  * This struct manages an RTCP source description item
  *
  * @short   RTCP Source Description Item
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPSourceDescriptionItem
{
   public:
   /**
     * Item type (RTCP_SDES_...).
     */
   card8 Type;

   /**
     * Length in bytes.
     */
   card8 Length;

   /**
     * Item data.
     */
   char Data[];
} __attribute__((packed));


/**
  * This struct manages an RTCP source description chunk
  *
  * @short   RTCP Source Description Chunk
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPSourceDescriptionChunk
{
   public:
   /**
     * SSRC/CSRC Identifier of source.
     */
   card32 SRC;

   /**
     * Array of SDES items.
     */
   RTCPSourceDescriptionItem Item[1];
} __attribute__((packed));


/**
  * This struct manages an RTCP source description (SDES)
  *
  * @short   RTCP Source Description (SDES)
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPSourceDescription : public RTCPCommonHeader
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPSourceDescription();

   /**
     * Constructor.
     *
     * @param Count count.
     */
   RTCPSourceDescription(const card8 count);


   /**
     * Initialize.
     *
     * @param Count count.
     */
   void init(const card8 count);


   /**
     * Array of SDES chunks.
     */
   RTCPSourceDescriptionChunk Chunk[1];
} __attribute__((packed));


/**
  * This struct manages an RTCP BYE message
  *
  * @short   RTCP BYE Message
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPBye : public RTCPCommonHeader
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPBye();

   /**
     * Constructor.
     *
     * @param Count count.
     */
   RTCPBye(const card8 count);


   /**
     * Initialize.
     *
     * @param Count count.
     */
   void init(const card8 count);


   // ====== Get methods ====================================================
   /**
     * Get source at given index.
     *
     * @param index Index.
     * @return Source.
     */
   inline card32 getSource(const cardinal index) const;


   // ====== Set methods ====================================================
   /**
     * Set source at given index.
     *
     * @param index Index.
     * @param source Source.
     */
   inline void setSource(const cardinal index, const card32 source);


   // ====== Private data ===================================================
   private:
   card32 Source[];
} __attribute__((packed));


/**
  * This struct manages an RTCP APP message
  *
  * @short   RTCP APP Message
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  *
  * @see RTCPSender
  * @see RTCPReceiver
  * @see RTCPAbstractServer
  */
struct RTCPApp : public RTCPCommonHeader
{
   // ====== Constructor ====================================================
   public:
   /**
     * Constructor.
     */
   RTCPApp();

   /**
     * Constructor.
     * @param subtype RTCP APP subtype.
     */
   RTCPApp(const card8 subtype);


   /*
     * Initialize.
     * @param subtype RTCP APP subtype.
     */
   void init(const card8 subtype);


   // ====== Get methods ====================================================
   /**
     * Get source.
     *
     * @return Source.
     */
   inline card32 getSource() const;

   /**
     * Get pointer to name field.
     *
     * @return Pointer to name field.
     */
   inline char* getName();

   /**
     * Get pointer to data field.
     *
     * @return Pointer to data field.
     */
   inline char* getData();


   // ====== Set methods ====================================================
   /**
     * Set source.
     *
     * @param source Source.
     */
   inline void setSource(const card32 source);


   // ====== Private data ===================================================
   private:
   card32 Source;
   char   Name[4];
   char   Data[];
} __attribute__((packed));


#include "rtcppacket.icc"


#endif
