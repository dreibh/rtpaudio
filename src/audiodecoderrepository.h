// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Decoder Repository                                         ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2018 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   https://www.uni-due.de/~be0001/rtpaudio                ####
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


#ifndef AUDIODECODERREPOSITORY_H
#define AUDIODECODERREPOSITORY_H


#include "tdsystem.h"
#include "audiodecoderinterface.h"
#include "decoderrepositoryinterface.h"
#include "audioquality.h"


#include <map>


/**
  * This class is a repository for audio decoders.
  *
  * @short   Audio Decoder Repository
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class AudioDecoderRepository : virtual public DecoderRepositoryInterface,
                               virtual public AudioDecoderInterface
{
   public:
   // ====== Constructor/Destructor =========================================
   /**
     * Constructor.
     */
   AudioDecoderRepository();

   /**
     * Destructor.
     */
   virtual ~AudioDecoderRepository();


   // ====== Repository functionality =======================================
   /**
     * Add audio decoder to repository.
     *
     * @param decoder New audio decoder to be added.
     * @return true, if decoder has been added; false, if not.
     */
   bool addDecoder(AudioDecoderInterface* decoder);

   /**
     * Remove audio decoder from repository.
     *
     * @param decoder Audio decoder to be removed.
     */
   void removeDecoder(AudioDecoderInterface* decoder);

   /**
     * selectDecoderForTypeID() implementation of DecoderRepositoryInterface.
     *
     * @see DecoderRepositoryInterface#selectDecoderForTypeID
     */
   bool selectDecoderForTypeID(const card16 typeID);

   /**
     * Set AutoDelete mode. If true, all decoders will be deleted with delete
     * operator by the destructor.
     */
   inline void setAutoDelete(const bool on);

   /**
     * getCurrentDecoder() implementation of DecoderRepositoryInterface.
     *
     * @see DecoderRepositoryInterface#getCurrentDecoder
     */
   DecoderInterface* getCurrentDecoder() const;

   /**
     * Get AudioDecoderInterface of the current decoder.
     *
     * @return Current decoder's AudioDecoderInterface.
     */
   AudioDecoderInterface* getCurrentAudioDecoder() const;


   // ====== DecoderInterface implementation ================================
   /**
     * getTypeID() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getTypeID
     */
   const card16 getTypeID() const;

   /**
     * getTypeName implementation of DecoderInterface.
     *
     * @see DecoderInterface#getTypeName
     */
   const char* getTypeName() const;

   /**
     * activate() implementation of DecoderInterface.
     *
     * @see DecoderInterface#activate
     */
   void activate();

   /**
     * deactivate() implementation of DecoderInterface.
     *
     * @see DecoderInterface#deactivate
     */
   void deactivate();

   /**
     * reset() implementation of DecoderInterface.
     *
     * @see DecoderInterface#reset
     */
   void reset();

   /**
     * getMediaInfo() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getMediaInfo
     */
   void getMediaInfo(MediaInfo& mediaInfo) const;

   /**
     * getErrorCode() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getErrorCode
     */
   card8 getErrorCode() const;

   /**
     * getPosition() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getPosition
     */
   card64 getPosition() const;

   /**
     * getMaxPosition() implementation of DecoderInterface.
     *
     * @see DecoderInterface#getMaxPosition
     */
   card64 getMaxPosition() const;

   /**
     * checkNextPacket() implementation of DecoderInterface.
     *
     * @see DecoderInterface#checkNextPacket
     */
   bool checkNextPacket(DecoderPacket* decoderPacket);

   /**
     * handleNextPacket() implementation of DecoderInterface.
     *
     * @see DecoderInterface#handleNextPacket
     */
   void handleNextPacket(const DecoderPacket* decoderPacket);


   // ====== AudioDecoderInterface implementation ===========================
   /**
     * getChannels() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getChannels
     */
   card8 getChannels() const;

   /**
     * getBits() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getBits
     */
   card8 getBits() const;

   /**
     * getSamplingRate() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getSamplingRate
     */
   card16 getSamplingRate() const;


   /**
     * getByteOrder() Implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getByteOrder
     */
   card16 getByteOrder() const;


   /**
     * getBytesPerSecond() implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getBytesPerSecond
     */
   cardinal getBytesPerSecond() const;

   /**
     * getBitsPerSample() implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#getBitsPerSample
     */
   cardinal getBitsPerSample() const;

   /**
     * getWantedQuality() implementation of AudioDecoderInterface.
     *
     * see AudioDecoderInterface#getWantedQuality
     */
   AudioQuality getWantedQuality() const;

   /**
     * setWantedQuality() implementation of AudioDecoderInterface.
     *
     * @see AudioDecoderInterface#setWantedQuality
     */
   void setWantedQuality(const AudioQualityInterface& wantedQuality);


   // ====== Private data ===================================================
   private:
   std::multimap<const card16,AudioDecoderInterface*> Repository;
   AudioDecoderInterface*                             Decoder;
   bool                                               AutoDelete;
};


#include "audiodecoderrepository.icc"


#endif
