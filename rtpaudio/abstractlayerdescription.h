// ##########################################################################
// ####                                                                  ####
// ####                    Master Thesis Implementation                  ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                  DiffServ with A Priori Knowledge                ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Abstract Layer Description Inlines                               ####
// ####                                                                  ####
// #### Version 1.00  --  February 23, 2001                              ####
// ####                                                                  ####
// #### Copyright (C) 2000/2001 Thomas Dreibholz                         ####
// #### University of Bonn, Department of Computer Science IV            ####
// #### EMail: Dreibholz@bigfoot.com                                     ####
// #### WWW:   http://www.bigfoot.com/~dreibholz/diplom/index.html       ####
// ####                                                                  ####
// ##########################################################################


#ifndef ABSTRACTLAYERDESCRIPTION_H
#define ABSTRACTLAYERDESCRIPTION_H


#include "tdsystem.h"
#include "framesizescalabilityinterface.h"
#include "internetflow.h"


namespace Coral {


/**
  * This class contains a layer's QoS requirements.
  * Important node: All frames sizes in this class are *raw* frame sizes, the
  * frames sizes in FrameSizeScalability are payload frame sizes. This class
  * does necessary translation.
  *
  * @short   Abstract Layer Description.
  * @author  Thomas Dreibholz (Dreibholz@bigfoot.com)
  * @version 1.0
*/
class AbstractLayerDescription : virtual public FrameSizeScalabilityInterface
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   AbstractLayerDescription();

   /**
     * Destructor.
     */
   virtual ~AbstractLayerDescription();

   /**
     * Initialize layer description.
     *
     * @param pktHeaderSize Packet header size, e.g. 40 + 8 + 12 (IPv6 + UDP + RTP).
     * @param pktMaxSize Maximum packet size, e.g. 1500.
     * @param maxTransferDelay Maximum transfer delay in microseconds.
     * @param maxBufferDelay Maximum buffer delay in frame rate units.
     * @param maxLossRate Maximum loss rate (out of [0,1]).
     * @param maxJitter Maximum jitter in microseconds.
     */
   inline void initLayer(const cardinal pktHeaderSize,
                         const cardinal pktMaxSize,
                         const double   maxTransferDelay,
                         const cardinal maxBufferDelay,
                         const double   maxLossRate,
                         const double   maxJitter,
                         const cardinal flags);


   // ====== Bandwidth and bandwidth quality methods ========================
   /**
     * Get bandwidth.
     *
     * @return Bandwidth.
     */
   inline card64 getBandwidth() const;

   /**
     * Set bandwidth.
     *
     * @param frameRate Frame rate.
     * @param bandwidth Bandwidth.
     * @return true, if bandwidth is sufficient for minimum requirement.
     */
   inline bool setBandwidth(const double frameRate,
                            const card64 bandwidth);

   /**
     * Get packet rate.
     *
     * @param frameRate Frame rate.
     * @return Bandwidth limit.
     */
   virtual cardinal getPacketRate(const double frameRate) const;

   /**
     * Translate frame size into bandwidth.
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Bandwidth.
     */
   static inline card64 frameSizeToBandwidth(const double   frameRate,
                                             const cardinal frameSize);

   /**
     * Translate bandwidth into frame size.
     *
     * @param frameRate Frame rate.
     * @param bandwidth Bandwidth.
     * @return Frame size.
     */
   static inline cardinal bandwidthToFrameSize(const double frameRate,
                                               const card64 bandwidth);


   /**
     * Translate bandwidth into bandwidth using different buffer delay.
     *
     * @param bandwidth Input bandwidth.
     * @param frameRate Input frame rate.
     * @param bufferDelay Input buffer delay.
     * @param newBufferDelay Output buffer delay.
     * @return Output bandwidth.
     */
   inline card64 bandwidthToBandwidth(const card64   bandwidth,
                                      const double   frameRate,
                                      const cardinal bufferDelay,
                                      const cardinal newBufferDelay) const;


   /**
     * Translate *payload* bandwidth into bandwidth using different buffer delay.
     *
     * @param bandwidth Input payload bandwidth.
     * @param frameRate Input frame rate.
     * @param bufferDelay Input buffer delay.
     * @param newBufferDelay Output buffer delay.
     * @return Output payload bandwidth.
     */
   card64 payloadBandwidthToBandwidth(const card64   bandwidth,
                                      const double   frameRate,
                                      const cardinal bufferDelay,
                                      const cardinal newBufferDelay) const;


   /**
     * Get packets per second for given frame size.
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Packets per second.
     */
   virtual cardinal frameSizeToPacketRate(const double   frameRate,
                                          const cardinal frameSize) const;

   /**
     * Get maximum transfer delay.
     *
     * @return Maximum transfer delay in microseconds.
     */
   inline double getMaxTransferDelay() const;

   /**
     * Set maximum transfer delay.
     *
     * @param maxDelay Maximum transfer delay in microseconds.
     */
   inline void setMaxTransferDelay(const double maxDelay);

   /**
     * Get maximum loss rate.
     *
     * @return Maximum loss rate (out of [0,1]).
     */
   inline double getMaxLossRate() const;

   /**
     * Set maximum loss rate.
     *
     * @param maxLossRate Maximum loss rate (out of [0,1]).
     */
   inline void setMaxLossRate(const double maxLossRate);

   /**
     * Get maximum jitter.
     *
     * @return Maximum jitter in microseconds.
     */
   inline double getMaxJitter() const;

   /**
     * Get maximum jitter.
     *
     * @param maxJitter Maximum jitter in microseconds.
     */
   inline void setMaxJitter(const double maxJitter);


   // ====== Frame size methods =============================================
   /**
     * Check, if given frame size is valid for given frame rate and buffer delay.
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay.
     * @param size FrameSize.
     * @return true, if frame size is valid; false otherwise.
     */
   inline bool isValidFrameSize(const double   frameRate,
                                const cardinal bufferDelay,
                                const cardinal size) const;

   /**
     * Get nearest lower frame size for given frame rate and buffer delay.
     *
     * @param frameRate Frame rate.
     * @param bufferDelay Buffer delay.
     * @param size FrameSize.
     * @return Nearest lower frame size.
     */
   inline cardinal getNearestValidFrameSize(const double   frameRate,
                                            const cardinal bufferDelay,
                                            const cardinal size) const;

   /**
     * Translate payload frame size into raw frame size.
     *
     * @param frameRate Frame rate.
     * @param payload Payload frame size.
     * @param bufferDelay Buffer delay.
     * @return Raw frame size.
     */
   virtual cardinal payloadToRaw(const double   frameRate,
                                 const cardinal payload,
                                 const cardinal bufferDelay) const;

   /**
     * Translate raw frame size into payload frame size.
     *
     * @param frameRate Frame rate.
     * @param raw Raw frame size.
     * @param bufferDelay Buffer delay.
     * @return Payload frame size.
     */
   virtual cardinal rawToPayload(const double   frameRate,
                                 const cardinal raw,
                                 const cardinal bufferDelay) const;


   // ====== Scaling and utilization methods, for current buffer delay ======
   /**
     * Get minimum frame size for given frame rate.
     *
     * @param frameRate Frame rate.
     * @return Minimum frame size.
     */
   inline cardinal getMinFrameSize(const double frameRate) const;

   /**
     * Get maximum frame size for given frame rate.
     *
     * @param frameRate Frame rate.
     * @return Maximum frame size.
     */
   inline cardinal getMaxFrameSize(const double frameRate) const;

   /**
     * Get peak frame size for given frame rate and size.
     *
     * @param frameRate Frame rate.
     * @return Peak frame size.
     */
   inline cardinal getPeakFrameSizeForSize(const double   frameRate,
                                           const cardinal frameSize) const;

   /**
     * Get number of packets (upper limit) for given frame rate and size.
     *
     * @param frameRate Frame rate.
     * @return Number of packets.
     */
   inline cardinal getPacketCountForSize(const double   frameRate,
                                         const cardinal frameSize) const;

   /**
     * Get next lower frame size for given frame rate and size and size.
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Next lower frame size.
     */
   inline double getPrevFrameSizeForSize(const double   frameRate,
                                         const cardinal frameSize) const;

   /**
     * Get next higher frame size for given frame rate and size.
     *
     * @param frameRate Frame rate and size.
     * @param frameSize Frame size.
     * @return Next higher frame size.
     */
   inline double getNextFrameSizeForSize(const double   frameRate,
                                         const cardinal frameSize) const;

   /**
     * Get frame size scale factor for given frame rate and size:
     * (size - MinFrameSize) / (MaxFrameSize - MinFrameSize).
     *
     * @param frameRate Frame rate and size.
     * @param frameSize Frame size.
     * @return Scale factor (out of [0,1]).
     */
   inline double getFrameSizeScaleFactorForSize(const double   frameRate,
                                                const cardinal frameSize) const;

   /**
     * Get frame size utilization for given frame rate and size.
     *
     * @param frameRate Frame rate and size.
     * @param frameSize Frame size.
     * @return Utilization (out of [0,1]).
     */
   inline double getFrameSizeUtilizationForSize(const double   frameRate,
                                                const cardinal frameSize) const;


   // ====== Scaling and utilization methods, for given buffer delay ========
   /**
     * Get minimum frame size for given frame rate and buffer delay.
     *
     * @param frameRate Frame rate.
     * @return Minimum frame size.
     */
   inline cardinal getMinFrameSizeForDelay(const double   frameRate,
                                           const cardinal bufferDelay) const;

   /**
     * Get maximum frame size for given frame rate and buffer delay.
     *
     * @param frameRate Frame rate.
     * @return Maximum frame size.
     */
   inline cardinal getMaxFrameSizeForDelay(const double   frameRate,
                                           const cardinal bufferDelay) const;

   /**
     * Get peak frame size for given frame rate, size and buffer delay.
     *
     * @param frameRate Frame rate.
     * @return Peak frame size.
     */
   inline cardinal getPeakFrameSizeForDelayAndSize(const double   frameRate,
                                                   const cardinal bufferDelay,
                                                   const cardinal frameSize) const;

   /**
     * Get number of packets (upper limit) for given frame rate, size and buffer delay.
     *
     * @param frameRate Frame rate.
     * @return Number of packets.
     */
   inline cardinal getPacketCountForDelayAndSize(const double   frameRate,
                                                 const cardinal bufferDelay,
                                                 const cardinal frameSize) const;

   /**
     * Get next lower frame size for given frame rate, size and buffer delay.
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Next lower frame size.
     */
   inline double getPrevFrameSizeForDelayAndSize(const double   frameRate,
                                                 const cardinal bufferDelay,
                                                 const cardinal frameSize) const;
   /**
     * Get next higher frame size for given frame rate, size and buffer delay.
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Next higher frame size.
     */
   inline double getNextFrameSizeForDelayAndSize(const double   frameRate,
                                                 const cardinal bufferDelay,
                                                 const cardinal frameSize) const;

   /**
     * Get frame size scale factor for given frame rate, size and buffer delay.
     * (size - MinFrameSize) / (MaxFrameSize - MinFrameSize).
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Scale factor (out of [0,1]).
     */
   inline double getFrameSizeScaleFactorForDelayAndSize(const double   frameRate,
                                                        const cardinal bufferDelay,
                                                        const cardinal frameSize) const;

   /**
     * Get frame size utilization for given frame rate, size and buffer delay.
     *
     * @param frameRate Frame rate.
     * @param frameSize Frame size.
     * @return Utilization (out of [0,1]).
     */
   inline double getFrameSizeUtilizationForDelayAndSize(const double   frameRate,
                                                        const cardinal bufferDelay,
                                                        const cardinal frameSize) const;


   // ====== Buffer delay methods ===========================================
   /**
     * Get buffer delay.
     *
     * @return Buffer delay in frame rate units.
     */
   inline cardinal getBufferDelay() const;

   /**
     * Set buffer delay.
     *
     * @param bufferDelay Buffer delay in frame rate units.
     * @return Buffer delay set in frame rate units.
     */
   cardinal setBufferDelay(const cardinal bufferDelay);

   /**
     * Get next lower buffer delay.
     *
     * @param frameRate Frame rate.
     * @return Buffer delay in frame rate units.
     */
   inline cardinal getPrevBufferDelay(const double frameRate) const;

   /**
     * Get next higher buffer delay.
     *
     * @param frameRate Frame rate.
     * @return Buffer delay in frame rate units.
     */
   inline cardinal getNextBufferDelay(const double frameRate) const;


   // ====== Source and destination address methods =========================
   /**
     * Get source address.
     *
     * @return Source address.
     */
   inline InternetAddress getSource() const;

   /**
     * Get destination address.
     *
     * @return Destination address.
     */
   inline InternetFlow getDestination() const;

   /**
     * Set source address.
     *
     * @param source Source address.
     */
   inline void setSource(const InternetAddress& source);

   /**
     * Set destination address.
     *
     * @param destination Destination address
     */
   inline void setDestination(const InternetFlow& destination);


   // ====== Flags ==========================================================
   /**
     * Get flags.
     */
   inline cardinal getFlags() const;

   /**
     * Set flags.
     */
   inline void setFlags(const cardinal flags);

   /**
     * Layer flags.
     */
   enum LayerFlags {
      LF_BaseLayer      = 0,
      LF_ExtensionLayer = (1 << 0)
   };


   // ====== Protected data =================================================
   protected:
   cardinal        PktHeaderSize;
   cardinal        PktMaxSize;

   card64          Bandwidth;

   double          MaxTransferDelay;
   double          MaxLossRate;
   double          MaxJitter;

   cardinal        BufferDelay;
   cardinal        MaxBufferDelay;
   cardinal        Flags;

   InternetAddress Source;
   InternetFlow    Destination;
};


}


#include "abstractlayerdescription.icc"


#endif
