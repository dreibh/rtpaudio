// ##########################################################################
// ####                                                                  ####
// ####                   Master Thesis Implementation                   ####
// ####  Management of Layered Variable Bitrate Multimedia Streams over  ####
// ####                 DiffServ with A Priori Knowledge                 ####
// ####                                                                  ####
// #### ================================================================ ####
// ####                                                                  ####
// ####                                                                  ####
// #### Service Level Agreement                                          ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
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


#ifndef SERVICELEVELAGREEMENT_H
#define SERVICELEVELAGREEMENT_H


#include "tdsystem.h"
#include "bandwidthinfo.h"
#include "trafficclassvalues.h"
#include "abstractlayerdescription.h"


/**
  * This is a DiffServ class.
  *
  * @short   DiffServ Class
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
struct DiffServClass
{
   /**
     * Bytes per second.
     */
   card64 BytesPerSecond;

   /**
     * Maximum transfer delay.
     */
   double MaxTransferDelay;

   /**
     * Maximum loss rate.
     */
   double MaxLossRate;

   /**
     * Maximum jitter.
     */
   double MaxJitter;

   /**
     * Cost factor.
     */
   double CostFactor;

   /**
     * Delay variability: Fraction of the transfer delay (out of [0,1]).
     */
   double DelayVariability;

   /**
     * Traffic class byte.
     */
   card8 TrafficClass;
};



/**
  * This class is a service level agreement (SLA).
  *
  * @short   Trace Layer Configuration
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class ServiceLevelAgreement
{
   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor.
     */
   ServiceLevelAgreement();

   /**
     * Destructor.
     */
   ~ServiceLevelAgreement();


   // ====== Load SLA =======================================================
   /**
     * Load configuration from file.
     */
   bool load(const char* fileName);


   // ====== SLA access =====================================================
   /**
     * Get possible DiffServ classes for given bandwidth info.
     *
     * @param ald AbstractLayerDescription.
     * @param classList Array to store class index numbers.
     * @return Number of class index numbers stored (0 = no possible classes found).
     */
   cardinal getPossibleClassesForBandwidthInfo(
               const AbstractLayerDescription* ald,
               cardinal*                       classList) const;


   // ====== Public data ====================================================
   public:
   card64        TotalBandwidth;
   cardinal      BestEffort;
   cardinal      Classes;
   DiffServClass Class[TrafficClassValues::MaxValues];
};


/**
  * Output operator.
  */
std::ostream& operator<<(std::ostream& os, const ServiceLevelAgreement config);


#include "servicelevelagreement.icc"


#endif
