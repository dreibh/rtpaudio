// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Abstract Layer Description Implementation                        ####
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


#include "tdsystem.h"
#include "abstractlayerdescription.h"


// Use frame count empirical envelope approximation
#define USE_FRAMECOUNT_APPROXIMATION


// Verify the raw<->payload conversion. Warning: Do not use this in
// multithread programs due to usage of global variable!!!
// #define VERIFY_RAWPAYLOADCONVERSION
// #define VERBOSE_BANDWIDTH_TO_BANDWIDTH


// ###### Constructor #######################################################
AbstractLayerDescription::AbstractLayerDescription()
{
   PktHeaderSize    = 28;
   PktMaxSize       = 1500;
   Bandwidth        = 0;
   Flags            = LF_BaseLayer;
   MaxTransferDelay = HUGE_VAL;
   MaxLossRate      = HUGE_VAL;
   MaxJitter        = HUGE_VAL;
   BufferDelay      = 1;
}


// ###### Desstructor #######################################################
AbstractLayerDescription::~AbstractLayerDescription()
{
}


// ###### Get packet rate ###################################################
cardinal AbstractLayerDescription::getPacketRate(const double frameRate) const
{
   return((cardinal)ceil(frameRate) *
             getPacketCountForSize(
                frameRate,
                (cardinal)floor((double)Bandwidth / frameRate)));
}


#ifdef VERIFY_RAWPAYLOADCONVERSION
bool RawToPayloadInUse = false;
#endif


// ###### Translate raw into payload frame size #############################
cardinal AbstractLayerDescription::rawToPayload(const double   frameRate,
                                                const cardinal raw,
                                                const cardinal bufferDelay) const
{
   // ====== Zero -> Zero ===================================================
   if(raw == 0) {
      return(0);
   }

   // ====== Calculation number of "frame start" packets ====================
#ifdef USE_FRAMECOUNT_APPROXIMATION
   const cardinal maxFrameCount = getMaxFrameCountForDelay(frameRate,bufferDelay);
   cardinal maxFramePackets     = (cardinal)ceil(frameRate * (double)maxFrameCount / (double)bufferDelay);
   if(maxFramePackets > 0) {
      maxFramePackets--;
   }
#else
   const cardinal maxFramePackets = (cardinal)ceil(frameRate);
#endif


   // ====== Calculate number of payload packets ============================
   int64 rawBandwidth = (int64)ceil((double)raw * frameRate) -
                           (int64)(maxFramePackets * PktHeaderSize);
   if(rawBandwidth <= 0) {
      return(0);
   }
   integer payloadPackets  = (integer)ceil(rawBandwidth / (double)PktMaxSize);
   if(payloadPackets < 1) {
      return(0);
   }

   // ====== Check, if correction is necessary ==============================
   if(payloadPackets > 0) {
      const card64 calcBandwidth = (card64)(payloadPackets - 1) * (card64)PktMaxSize;
      const int64 diff           = (int64)rawBandwidth - (int64)calcBandwidth;
      if((diff > 0) && (diff < (int64)PktHeaderSize)) {
         payloadPackets--;
         rawBandwidth = calcBandwidth;
      }
   }

   // ====== Calculate payload frame size ===================================
   const card64   payloadBandwidth = rawBandwidth - (card64)(payloadPackets * PktHeaderSize);
   const cardinal payload = (cardinal)floor((double)payloadBandwidth / frameRate);


#ifdef VERIFY_RAWPAYLOADCONVERSION
   if(!RawToPayloadInUse) {
      cout << "Raw: " << raw << "  ->  Payload: " << payload << endl;
      RawToPayloadInUse = true;
      const cardinal verification = payloadToRaw(frameRate,payload,bufferDelay);
      if(raw < verification) {
         // Note: raw > verification is no problem - only inaccuracy due to ceil().
         cerr << "INTERNAL ERROR: AbstractLayerDescription::rawToPayload() - Raw to Payload verification failed!" << endl
              << "Input:         " << frameRate << ", " << raw << " raw, " << bufferDelay << endl
              << "Payload:       " << payload << endl
              << "Raw Calc.:     " << verification << " vs. " << raw << endl
              << "MaxFrameCount: " << maxFrameCount << endl
              << "MaxFramePkts:  " << maxFramePackets << endl
              << "PayloadPkts:   " << payloadPackets  << endl;
         ::abort();
      }
      RawToPayloadInUse = false;
   }
/*
   cout << "Verification: Raw -> Payload" << endl
        << "   Payload:       " << frameRate << ", " << payload << ", " << bufferDelay << endl
        << "   Raw:           " << raw << endl
        << "   MaxFrameCount: " << maxFrameCount << endl
        << "   MaxFramePkts:  " << maxFramePackets << endl
        << "   PayloadPkts:   " << payloadPackets  << endl;
*/
#endif
   return(payload);
}


// ###### Translate payload into raw frame size #############################
cardinal AbstractLayerDescription::payloadToRaw(
                                      const double   frameRate,
                                      const cardinal payload,
                                      const cardinal bufferDelay) const
{
   // ====== Zero -> Zero ===================================================
   if(payload == 0) {
      return(0);
   }

   // ====== Calculation number of "frame start" packets ====================
#ifdef USE_FRAMECOUNT_APPROXIMATION
   const cardinal maxFrameCount = getMaxFrameCountForDelay(frameRate,bufferDelay);
   cardinal maxFramePackets     = (cardinal)ceil(frameRate * (double)maxFrameCount / (double)bufferDelay);
   if(maxFramePackets > 0) {
      maxFramePackets--;
   }
#else
   const cardinal maxFramePackets = (cardinal)ceil(frameRate);
#endif

   // ====== Calculate number of payload packets ============================
   const cardinal maxPktPayload    = PktMaxSize - PktHeaderSize;
   const card64   payloadBandwidth = (card64)ceil(frameRate * (double)payload);
   const cardinal payloadPackets   = (cardinal)ceil((double)payloadBandwidth / (double)maxPktPayload);

   // ====== Calculate raw frame size =======================================
   const card64   rawBandwidth = payloadBandwidth + (card64)(PktHeaderSize * (payloadPackets + maxFramePackets));
   const cardinal raw          = (cardinal)ceil((double)rawBandwidth / frameRate);


#ifdef VERIFY_RAWPAYLOADCONVERSION
   if(!RawToPayloadInUse) {
      cout << "Payload: " << payload << "  ->  Raw: " << raw << endl;
      RawToPayloadInUse = true;
      const cardinal verification = rawToPayload(frameRate,raw,bufferDelay);
      if(payload != verification) {
         cerr << "INTERNAL ERROR: AbstractLayerDescription::payloadToRaw() - Payload to Raw conversion failed!" << endl
              << "Input:         " << frameRate << ", " << payload << " payload, " << bufferDelay << endl
              << "Raw:           " << raw << endl
              << "Payload Calc.: " << verification << " vs. " << payload << endl
              << "MaxFrameCount: " << maxFrameCount << endl
              << "MaxFramePkts:  " << maxFramePackets << endl
              << "PayloadPkts:   " << payloadPackets  << endl
              << "MaxPktPayload: " << maxPktPayload << endl;
         ::abort();
      }
      RawToPayloadInUse = false;
   }
/*
   cout << "Verification: Payload -> Raw" << endl
        << "   Payload:       " << frameRate << ", " << payload << ", " << bufferDelay << endl
        << "   Raw:           " << raw << endl
        << "   MaxFrameCount: " << maxFrameCount << endl
        << "   MaxFramePkts:  " << maxFramePackets << endl
        << "   PayloadPkts:   " << payloadPackets  << endl
        << "   MaxPktPayload: " << maxPktPayload << endl;
*/
#endif
   return(raw);
}


// ###### Translate bandwidth into bandwidth using different buffer delay ###
card64 AbstractLayerDescription::payloadBandwidthToBandwidth(
                                    const card64   bandwidth,
                                    const double   frameRate,
                                    const cardinal bufferDelay,
                                    const cardinal newBufferDelay) const
{
   // ====== Check for necessity to translate bandwidths ====================
   if(frameRate == 0.0) {
      return(0);
   }
   if(bufferDelay == newBufferDelay) {
#ifndef VERBOSE_BANDWIDTH_TO_BANDWIDTH
      return(bandwidth);
#endif
   }


   // ====== Calculate scale factor =========================================
   const cardinal frameSize     = bandwidthToFrameSize(
                                     frameRate, bandwidth);
   const cardinal minFrameSizeO = getMinPayloadFrameSizeForDelay(
                                     frameRate, bufferDelay);
   const cardinal maxFrameSizeO = getMaxPayloadFrameSizeForDelay(
                                     frameRate, bufferDelay);
   double scaleFactor;
   if(frameSize < minFrameSizeO) {
#ifdef VERBOSE_BANDWIDTH_TO_BANDWIDTH
      cerr << "WARNING: AbstractLayerDescription::payloadBandwidthToBandwidth() - "
              "Scale factor < 0.0!" << endl;
      cerr << "bandwidth " << bandwidth << ", frame rate " << frameRate << ":  "
           << frameSize << " vs. " << minFrameSizeO << endl;
      ::abort();
#endif
      return(0);
   }
   else if(frameSize > maxFrameSizeO) {
#ifdef VERBOSE_BANDWIDTH_TO_BANDWIDTH
      cerr << "WARNING: AbstractLayerDescription::payloadBandwidthToBandwidth() - "
              "Scale factor > 1.0!" << endl;
      cerr << "bandwidth " << bandwidth << ", frame rate " << frameRate << ":  "
           << frameSize << " vs. " << maxFrameSizeO << endl;
#endif
      scaleFactor = 1.0;
   }
   else {
      scaleFactor = (double)(frameSize - minFrameSizeO) /
                       (double)(maxFrameSizeO - minFrameSizeO);
   }
   // printf("min=%d max=%d  fs=%d   -> SF=%1.9f\n",minFrameSizeO,maxFrameSizeO,frameSize,scaleFactor);


   // ====== Get new frame size; nearest lower valid value ==================
   const cardinal minFrameSizeN = getMinPayloadFrameSizeForDelay(
                                     frameRate, newBufferDelay);
   const cardinal maxFrameSizeN = getMaxPayloadFrameSizeForDelay(
                                     frameRate, newBufferDelay);
   cardinal newFrameSize  = minFrameSizeN +
                               (cardinal)ceil(scaleFactor * (maxFrameSizeN - minFrameSizeN));
   newFrameSize = getNearestValidPayloadFrameSize(frameRate,newBufferDelay,newFrameSize);

   // ====== Convert frame size to bandwidth ================================
   const card64 newBandwidth = frameSizeToBandwidth(frameRate, newFrameSize);
   return(newBandwidth);
}


// ###### Calculate packet rate for frame size ##############################
cardinal AbstractLayerDescription::frameSizeToPacketRate(
                                      const double   frameRate,
                                      const cardinal frameSize) const
{
   return((cardinal)ceil((double)getPacketCountForSize(
                                    frameRate, frameSize) * frameRate));
}


// ###### Set buffer delay ##################################################
cardinal AbstractLayerDescription::setBufferDelay(const cardinal bufferDelay)
{
   if(BufferDelay < 1) {
      BufferDelay = 1;
   }
   else {
      BufferDelay = bufferDelay;
   }
   return(BufferDelay);
}
