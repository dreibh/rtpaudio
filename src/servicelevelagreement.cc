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
// ####           Copyright (C) 1999-2023 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "servicelevelagreement.h"


// Print information
// #define PRINT_WANTED


// ###### Output operator ###################################################
std::ostream& operator<<(std::ostream& os, const ServiceLevelAgreement sla)
{
   char str[256];
   os << "Idx  TC         Bandwidth   Cost Factor MaxDelay [ms]     MaxLossRate   MaxJitter [ms]"
      << std::endl;
   os << "--------------------------------------------------------------------------------------"
      << std::endl;
   for(cardinal i = 0;i < sla.Classes;i++) {
      cardinal lossRate = (cardinal)rint(sla.Class[i].MaxLossRate * 10000.0);
      snprintf((char*)&str,sizeof(str),
              "#%02d  %-4s %15llu    %1.2f       %4u (+/- %3u %%)   %3u.%02u %%     %-1.3f",
              i,
              TrafficClassValues::getNameForTrafficClass(sla.Class[i].TrafficClass),
              (unsigned long long)sla.Class[i].BytesPerSecond,
              sla.Class[i].CostFactor,
              (unsigned int)floor(sla.Class[i].MaxTransferDelay / 1000.0),
              (unsigned int)floor(sla.Class[i].DelayVariability * 100.0),
              lossRate / 100, lossRate % 100,
              sla.Class[i].MaxJitter / 1000.0);
      os << str << std::endl;
   }
   os << "--------------------------------------------------------------------------------------"
      << std::endl
      << "Total " << sla.TotalBandwidth << std::endl;
   return(os);
};


// ###### Constructor #######################################################
ServiceLevelAgreement::ServiceLevelAgreement()
{
   Classes        = 0;
   BestEffort     = (cardinal)-1;
   TotalBandwidth = 0;
}


// ###### Destructor ########################################################
ServiceLevelAgreement::~ServiceLevelAgreement()
{
}


// ###### Load configuration from file ######################################
bool ServiceLevelAgreement::load(const char* fileName)
{
   // ====== Open file ======================================================
   FILE* inputFD = fopen(fileName,"r");
   if(inputFD == NULL) {
      std::cerr << "ERROR: Unable to open input file <" << fileName << ">!" << std::endl;
      return(false);
   }

   // ====== Read configuration from file ===================================
   char str[256];
   char* result = fgets((char*)&str,sizeof(str),inputFD);
   cardinal line  = 0;
   Classes        = 0;
   BestEffort     = (cardinal)-1;
   TotalBandwidth = 0;
   while( (result != NULL) && (!feof(inputFD)) ) {
      line++;
      const cardinal inputLength = strlen((char*)&str);
      if(inputLength > 1) {
         str[inputLength - 1] = 0x00;
         switch(str[0]) {
            // ====== Line is a comment =====================================
            case '#':
             break;

            // ====== Line is a setting =====================================
            default:
               const String input(str);
               String name;
               String value;
               if(input.scanSetting(name,value)) {
                  if(name == "NEW CLASS") {
                     if(Classes >= TrafficClassValues::MaxValues) {
                        std::cerr << "ERROR: Too many classes!" << std::endl;
                        fclose(inputFD);
                        return(false);
                     }
                     card64 bandwidth;
                     double maxJitter;
                     double maxLossRate;
                     double maxDelay;
                     double delayVariability;
                     double costFactor;
                     char   name[20];
                     cardinal count =
                        sscanf(value.getData(),
                               "%5s %llu %lf %lf %lf %lf %lf",
                               (char*)&name,
                               (unsigned long long*)&bandwidth,
                               &costFactor,
                               &maxDelay,
                               &delayVariability,
                               &maxLossRate,
                               &maxJitter);
                     if(count < 7) {
                        std::cerr << "ERROR: Bad class setting, "
                                "line " << line << "!" << std::endl;
                        std::cerr << "       Syntax: Class = <name> <bandwidth> <cost factor> <max delay> <delay variability> <max loss> <max jitter>" << std::endl;
                        fclose(inputFD);
                        return(false);
                     }
                     const card16 tc = TrafficClassValues::getTrafficClassForName(name);
                     if(tc == 0xffff) {
                        std::cerr << "ERROR: Unknown class name, "
                                "line " << line << "!" << std::endl;
                        fclose(inputFD);
                        return(false);
                     }
                     if(maxLossRate < 0.0) {
                        maxLossRate = 1.0;
                     }
                     else if(maxLossRate > 1.0) {
                        maxLossRate = 1.0;
                     }
                     maxJitter *= 1000.0;
                     if(maxJitter < 0.0) {
                        maxJitter = HUGE_VAL;
                     }
                     if(maxDelay < 0.0) {
                        maxDelay = HUGE_VAL;
                     }
                     if(costFactor < 0.0) {
                        costFactor = 0.0;
                     }
                     if(delayVariability < 0.0) {
                        delayVariability = 0.0;
                     }
                     else if(delayVariability > 1.0) {
                        delayVariability = 1.0;
                     }
                     Class[Classes].BytesPerSecond   = bandwidth;
                     Class[Classes].MaxTransferDelay = maxDelay;
                     Class[Classes].MaxLossRate      = maxLossRate;
                     Class[Classes].MaxJitter        = maxJitter;
                     Class[Classes].CostFactor       = costFactor;
                     Class[Classes].TrafficClass     = (card8)tc;
                     Class[Classes].DelayVariability = delayVariability;
                     if(Class[Classes].TrafficClass == 0x00) {
                        if(BestEffort != (cardinal)-1) {
                           std::cerr << "ERROR: Duplicate definition of best effort class, "
                                   "line " << line << "!" << std::endl;
                           fclose(inputFD);
                           return(false);
                        }
                        BestEffort = Classes;
                     }
                     Classes++;
                     TotalBandwidth += bandwidth;
                  }
                  else {
                     std::cerr << "ERROR: Unknown option <"
                          << name << " = " << value << ">, "
                             "line " << line << "!" << std::endl;
                     fclose(inputFD);
                     return(false);
                  }
               }
             break;
         }
      }
      result = fgets((char*)&str,256,inputFD);
   }
   fclose(inputFD);


   // ====== Check for missing best effort class ============================
   if(BestEffort == (cardinal)-1) {
      std::cerr << "WARNING: No best effort class - dummy entry added!" << std::endl;
      Class[Classes].BytesPerSecond   = 0;
      Class[Classes].MaxTransferDelay = HUGE_VAL;
      Class[Classes].MaxLossRate      = 1.0;
      Class[Classes].MaxJitter        = HUGE_VAL;
      Class[Classes].CostFactor       = 1.0;
      Class[Classes].TrafficClass     = 0x00;
      Class[Classes].DelayVariability = 1.0;
      BestEffort = Classes;
      Classes++;
   }


   // ====== Sort classes by cost ascending =================================
   for(cardinal i = 0;i < Classes;i++) {
      for(cardinal j = i + 1;j < Classes;j++) {
         if(Class[i].CostFactor > Class[j].CostFactor) {
            const DiffServClass tmp = Class[i];
            Class[i] = Class[j];
            Class[j] = tmp;
         }
      }
   }
   return(true);
}


// ###### Get possible classes for BandwidthInfo ############################
cardinal ServiceLevelAgreement::getPossibleClassesForBandwidthInfo(
                                   const AbstractLayerDescription* ald,
                                   cardinal*                       classList) const
{
   cardinal count = 0;

#ifdef PRINT_WANTED
   cout << "Wanted delay < " << ald->getMaxTransferDelay() << ", "
        << " loss rate < " << ald->getMaxLossRate() << ", "
        << " jitter < " << ald->getMaxJitter() << std::endl;
#endif

   for(cardinal i = 0;i < Classes;i++) {
      if((Class[i].MaxTransferDelay <= ald->getMaxTransferDelay()) &&
         (Class[i].MaxLossRate      <= ald->getMaxLossRate())      &&
         (Class[i].MaxJitter        <= ald->getMaxJitter())) {
         classList[count] = i;
         count++;
      }
   }
   return(count);
}
