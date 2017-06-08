// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Decoder Interface                                                ####
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


#ifndef DECODERINTERFACE_H
#define DECODERINTERFACE_H


#include "tdsystem.h"
#include "sourcestateinfo.h"
#include "mediainfo.h"


/**
  * This structure contains packet information for handleNextPacket() call.
  *
  * @short   DecoderPacket
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
struct DecoderPacket
{
   /**
     * Buffer to write packet payload into.
     */
   void* Buffer;

   /**
     * Maximum length of payload to be written into Buffer.
     */
   cardinal Length;

   /**
     * The packet's sequence number.
     */
   card16 SequenceNumber;

   /**
     * The packet's time stamp.
     */
   card32 TimeStamp;

   /**
     * The packet's payload type.
     */
   card8 PayloadType;

   /**
     * The packet's marker.
     */
   bool Marker;


   /**
     * Source state info array for packet validation within handleNextPacket().
     */
   SourceStateInfo** SSIArray;


   /**
     * The packet's layer number, to be set within handleNextPacket().
     * This is used in RTPReceiver to decide to which layer the packet's
     * FlowInfo belongs. Set Layer (cardinal)-1, if the packet does not belong
     * to a layer, is invalid etc.
     */
   cardinal Layer;

   /**
     * The number of layers of the packet's encoding quality, to be set within handleNextPacket().
     * Set to (cardinal)-1, if the packet does not belong to a layer, is invalid etc.
     */
   cardinal Layers;
};



/**
  * This class is the interface for a decoder.
  *
  * @short   Decoder Interface
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class DecoderInterface
{
   // ====== Decoder type ===================================================
   public:
   /**
     * Get the decoder's type ID.
     *
     * @return Decoder's type ID.
     */
   virtual const card16 getTypeID() const = 0;

   /**
     * Get the decoder's name.
     *
     * @return Decoder's name
     */
   virtual const char* getTypeName() const = 0;


   // ====== Initialization/Clean-up ========================================
   /**
     * Activate the decoder.
     * Usage example: Start an decoder thread.
     */
   virtual void activate() = 0;

   /**
     * Deactivate the decoder.
     * Usage example: Stop an decoder thread.
     */
   virtual void deactivate() = 0;

   /**
     * Reset the decoder.
     * Usage example: Reset an decoder thread.
     */
   virtual void reset() = 0;


   // ====== Check next packet ==============================================
   /**
     * Check next packet. This function has to set valid packet->Layers
     * and packet->Layer value.
     *
     * @param decoderPacket DecoderPacket structure.
     * @return true, if packet is valid; false otherwise.
     */
   virtual bool checkNextPacket(DecoderPacket* packet) = 0;


   // ====== Decode next packet =============================================
   /**
     * Handle next received packet.
     *
     * @param decoderPacket DecoderPacket structure.
     */
   virtual void handleNextPacket(const DecoderPacket* decoderPacket) = 0;


   // ====== Status functions ===============================================
   /**
     * Get media info.
     *
     * @param mediaInfo Reference to store MediaInfo to.
     */
   virtual void getMediaInfo(MediaInfo& mediaInfo) const = 0;

   /**
     * Get error code
     * Usage example: Return error, if reading from file failed.
     *
     * @return Error code
     */
   virtual card8 getErrorCode() const = 0;

   /**
     * Get current position in nanoseconds.
     *
     * @return Position in nanoseconds.
     */
   virtual card64 getPosition() const = 0;

   /**
     * Get maximum position in nanoseconds.
     *
     * @return Maximum position in nanoseconds.
     */
   virtual card64 getMaxPosition() const = 0;
};


#endif
