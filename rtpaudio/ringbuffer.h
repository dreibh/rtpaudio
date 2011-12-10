/*
 *  $Id$
 *
 * SocketAPI implementation for the sctplib.
 * Copyright (C) 2005-2009 by Thomas Dreibholz
 *
 * Realized in co-operation between
 * - Siemens AG
 * - University of Essen, Institute of Computer Networking Technology
 * - University of Applied Sciences, Muenster
 *
 * Acknowledgement
 * This work was partially funded by the Bundesministerium fuer Bildung und
 * Forschung (BMBF) of the Federal Republic of Germany (Foerderkennzeichen 01AK045).
 * The authors alone are responsible for the contents.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: discussion@sctp.de
 *          dreibh@iem.uni-due.de
 *          tuexen@fh-muenster.de
 *
 * Purpose: Ring Buffer
 *
 */


#ifndef RINGBUFFER_H
#define RINGBUFFER_H


#include "tdsystem.h"
#include "condition.h"



/**
  * This class implements a ring buffer.
  *
  * @short   Ring Buffer
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class RingBuffer : public Condition
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   RingBuffer();

   /**
     * Destructor.
     */
   ~RingBuffer();


   // ====== Initialization =================================================
   /**
     * Initialize ring buffer.
     *
     * @param bytes Number of bytes to allocate for buffer.
     * @return true for success; false otherwise.
     */
   bool init(const cardinal bytes);


   // ====== Buffer maintenance =============================================
   /**
     * Flush buffer.
     */
   void flush();

   /**
     * Get number of bytes available for read.
     *
     * @return Number of bytes readable.
     */
   inline size_t bytesReadable();

   /**
     * Get number of bytes available for write = (BufferSize - bytesReadable()).
     *
     * @return Number of bytes writable.
     */
   inline size_t bytesWritable();


   // ====== Read/write =====================================================
   /**
     * Read data from ring buffer.
     *
     * @param data Data buffer to store read data to.
     * @param length Size of data buffer.
     * @return Bytes read from ring buffer.
     */
   ssize_t read(char*        data,
                const size_t length);

   /**
     * Write data into ring buffer.
     *
     * @param data Data buffer containing data to write.
     * @param length Length of data to write.
     * @return Bytes written into ring buffer.
     */
   ssize_t write(char*        data,
                 const size_t length);


   // ====== Private data ===================================================
   private:
   char*    Buffer;
   size_t   BufferSize;
   size_t   WriteStart;
   size_t   WriteEnd;
   size_t   BytesStored;
};


#include "ringbuffer.icc"


#endif
