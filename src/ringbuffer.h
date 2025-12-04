// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Ring Buffer Implementation                                       ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2026 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: thomas.dreibholz@gmail.com                             ####
// ####    WWW:   https://www.nntb.no/~dreibh/rtpaudio                   ####
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


#ifndef RINGBUFFER_H
#define RINGBUFFER_H


#include "tdsystem.h"
#include "condition.h"



/**
  * This class implements a ring buffer.
  *
  * @short   Ring Buffer
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
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
   ssize_t write(const char*  data,
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
