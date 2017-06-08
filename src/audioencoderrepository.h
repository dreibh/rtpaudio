// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Encoder Repository                                         ####
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


#ifndef AUDIOENCODERREPOSITORY_H
#define AUDIOENCODERREPOSITORY_H


#include "tdsystem.h"
#include "synchronizable.h"
#include "encoderrepositoryinterface.h"
#include "audioencoderinterface.h"

#include <map>


/**
  * This class is a repository for audio encoders.
  *
  * @short   Audio Encoder Repository
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioEncoderRepository : virtual public EncoderRepositoryInterface,
                               virtual public AudioEncoderInterface
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   AudioEncoderRepository();

   /**
     * Destructor.
     */
   ~AudioEncoderRepository();


   // ====== Repository functionality =======================================
   /**
     * Add audio encoder to repository.
     *
     * @param encoder New audio encoder to be added.
     * @return true, if encoder has been added; false, if not.
     */
   bool addEncoder(AudioEncoderInterface* encoder);

   /**
     * Remove audio encoder from repository.
     *
     * @param encoder Audio encoder to be removed.
     */
   void removeEncoder(AudioEncoderInterface* encoder);

   /**
     * selectEncoderForTypeID() implementation of EncoderRepositoryInterface.
     *
     * @see EncoderRepositoryInterface#selectEncoderForTypeID
     */
   bool selectEncoderForTypeID(const card16 typeID);

   /**
     * Set AutoDelete mode. If true, all encoders will be deleted with delete
     * operator by the destructor.
     */
   inline void setAutoDelete(const bool on);

   /**
     * getCurrentEncoder() implementation of EncoderRepositoryInterface.
     *
     * @see EncoderRepositoryInterface#getCurrentEncoder
     */
   EncoderInterface* getCurrentEncoder() const;

   /**
     * Get AudioEncoderInterface of the current encoder.
     *
     * @return Current encoder's AudioEncoderInterface.
     */
   AudioEncoderInterface* getCurrentAudioEncoder() const;


   // ====== EncoderInterface implementation ================================
   /**
     * getTypeID() implementation of EncoderInterface.
     *
     * @see EncoderInterface#getTypeID
     */
   const card16 getTypeID() const;

   /**
     * getTypeName implementation of EncoderInterface.
     *
     * @see EncoderInterface#getTypeName
     */
   const char* getTypeName() const;

   /**
     * activate() implementation of EncoderInterface.
     *
     * @see EncoderInterface#activate
     */
   void activate();

   /**
     * deactivate() implementation of EncoderInterface.
     *
     * @see EncoderInterface#deactivate
     */
   void deactivate();

   /**
     * reset() implementation of EncoderInterface.
     *
     * @see EncoderInterface#reset
     */
   void reset();

   /**
     * checkInterval() implementation of EncoderInterface.
     *
     * @see EncoderInterface#checkInterval
     */
   bool checkInterval(card64& time, bool& newRUList);

   /**
     * prepareNextFrame() implementation of EncoderInterface.
     *
     * @see EncoderInterface#prepareNextFrame
     */
   bool prepareNextFrame(const cardinal headerSize,
                         const cardinal maxPacketSize,
                         const cardinal flags);

   /**
     * getNextPacket() implementation of EncoderInterface.
     *
     * @see EncoderInterface#getNextPacket
     */
   cardinal getNextPacket(EncoderPacket* encoderPacket);



   // ====== AudioEncoderInterface implementation ===========================
   /**
     * getSamplingRate() implementation of AudioEncoderInterface
     *
     * @see AudioEncoderInterface#getSamplingRate
     */
   card16 getSamplingRate() const;

   /**
     * getBits() implementation of AudioEncoderInterface
     *
     * @see AudioEncoderInterface#getBits
     */
   card8 getBits() const;

   /**
     * getChannels() implementation of AudioEncoderInterface
     *
     * @see AudioEncoderInterface#getChannels
     */
   card8 getChannels() const;


   /**
     * getByteOrder() Implementation of AudioEncoderInterface.
     *
     * @see AudioEncoderInterface#getByteOrder
     */
   card16 getByteOrder() const;


   /**
     * getBytesPerSecond() implementation of AudioEncoderInterface.
     *
     * @see AudioEncoderInterface#getBytesPerSecond
     */
   cardinal getBytesPerSecond() const;

   /**
     * getBitsPerSample() implementation of AudioEncoderInterface.
     *
     * @see AudioEncoderInterface#getBitsPerSample
     */
   cardinal getBitsPerSample() const;


   /**
     * setSamplingRate() implementation of AudioEncoderInterface
     *
     * @see AudioEncoderInterface#setSamplingRate
     */
   card16 setSamplingRate(const card16 rate);

   /**
     * setBits() implementation of AudioEncoderInterface
     *
     * @see AudioEncoderInterface#setBits
     */
   card8 setBits(const card8 bits);

   /**
     * setChannels() implementation of AudioEncoderInterface
     *
     * @see AudioEncoderInterface#setChannels
     */
   card8 setChannels(const card8 channels);


   /**
     * setByteOrder() Implementation of AudioEncoderInterface.
     *
     * @see AudioEncoderInterface#setByteOrder
     */
   card16 setByteOrder(const card16 byteOrder);


   // ====== Settings ========================================================
   /**
     * getFrameRate() implementation of EncoderInterface.
     *
     * @return EncoderInterface#getFrameRate
     */
   double getFrameRate() const;

   /**
     * getQoSDescription() implementation of EncoderInterface.
     *
     * @see EncoderInterface#getQoSDescription
     */
   AbstractQoSDescription* getQoSDescription(
                              const cardinal pktHeaderSize,
                              const cardinal pktMaxSize,
                              const card64   offset);

   /**
     * updateQuality() implementation of EncoderInterface.
     *
     * @see EncoderInterface#updateQuality
     */
   void updateQuality(const AbstractQoSDescription* aqd);


   // ====== Private data ===================================================
   private:
   std::multimap<const card16,AudioEncoderInterface*> Repository;
   AudioEncoderInterface*                             Encoder;
   bool                                               AutoDelete;
};


#include "audioencoderrepository.icc"


#endif
