// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Encoder Interface                                                ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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


#ifndef ENCODERINTERFACE_H
#define ENCODERINTERFACE_H


#include "tdsystem.h"
#include "abstractqosdescription.h"


/**
  * This structure contains packet information for getNextPacket() call.
  *
  * @short   EncoderPacket
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
struct EncoderPacket
{
   /**
     * Buffer to write packet payload into.
     */
   void* Buffer;

   /**
     * Maximum length of payload to be written into Buffer.
     */
   cardinal MaxLength;

   /**
     * The packet's layer number, to be set within getNextPacket().
     */
   cardinal Layer;

   /**
     * The packet's payload type, to be set within getNextPacket().
     */
   card8 PayloadType;

   /**
     * The packet's marker, to be set within getNextPacket().
     */
   bool Marker;

   /**
     * The packet's error code. If greater than ME_UnrecoverableError,
     * the packet may sent even in case of exceeded traffic shaper buffer!
     */
   card8 ErrorCode;
};



/**
  * This class is the interface for an encoder.
  *
  * @short   Encoder Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class EncoderInterface
{
   // ====== Encoder type ===================================================
   public:
   /**
     * Get the encoder's type ID.
     *
     * @return Encoder's type ID.
     */
   virtual const card16 getTypeID() const = 0;

   /**
     * Get the encoder's name.
     *
     * @return Encoder's name
     */
   virtual const char* getTypeName() const = 0;


   // ====== Initialization/Clean-up ========================================
   /**
     * Activate the encoder.
     * Usage example: Start an encoder thread.
     */
   virtual void activate() = 0;

   /**
     * Deactivate the encoder.
     * Usage example: Stop an encoder thread.
     */
   virtual void deactivate() = 0;

   /**
     * Reset the encoder.
     * Usage example: Reset an encoder thread.
     */
   virtual void reset() = 0;


   // ====== Encode frame ===================================================
   /**
     * Check, when prepareNextFrame() call reaches a new interval.
     *
     * @param time Reference to store time to next interval in microseconds.
     * @param Reference to store true, if new resource/utilization list has been reached since last call; false otherwise.
     * @return true, if new interval has been reached since last call; false otherwise.
     */
   virtual bool checkInterval(card64& time, bool& newRUList) = 0;

   /**
     * Prepare next frame.
     * Usage example: Read the next frame from file, transform it into
     * packages for transport.
     *
     * @param headerSize Size of underlying protocol's header (e.g. RTP packet)
     * @param maxPacketSize Maximum size of packet.
     * @param flags Encoder-specific flags (e.g. compression or encryption).
     * @return true, if there was a next frame; false, if not.
     */
   virtual bool prepareNextFrame(const cardinal headerSize,
                                 const cardinal maxPacketSize,
                                 const cardinal flags = 0) = 0;

   /**
     * Get next packet from current frame. The maximum payload length of the
     * packet (the size of packet->Buffer) is in packet->MaxLength.
     *
     * @param packet EncoderPacket structure.
     * @param buffer Buffer of the packet to write the data into.
     * @param maxLength Maximum length of the packet
     * @return Real length of the data written into the buffer or 0, if there is no more data of the current frame.
     */
   virtual cardinal getNextPacket(EncoderPacket* packet) = 0;


   // ====== Settings =======================================================
   /**
     * Get frame rate.
     *
     * @return Frame rate.
     */
   virtual double getFrameRate() const = 0;

   /**
     * Get QoS description. Important note: This result is a global pointer,
     * it becomes invalid when encoder is deleted!
     *
     * @param pktHeaderSize Packet header size.
     * @param pktMaxSize Maximum packet size.
     * @param offset RTP position offset.
     * @return QoS Description.
     */
   virtual AbstractQoSDescription* getQoSDescription(
                                      const cardinal pktHeaderSize,
                                      const cardinal pktMaxSize,
                                      const card64   offset) = 0;

   /**
     * Update encoder quality to changes made in QoS description returned
     * by getQoSDescription().
     *
     * @see EncoderInterface#getQoSDescription
     */
   virtual void updateQuality(const AbstractQoSDescription* aqd) = 0;
};


#endif
