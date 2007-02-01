#include "tdsystem.h"
#include "cbrframesizescalability.h"
#include "audioquality.h"


class ConstantBitrateAudioFrameSizeScalability : public ConstantBitrateFrameSizeScalability
{
   public:
   ConstantBitrateAudioFrameSizeScalability();

   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getNextFrameSizeForDelayAndSize
     */
   cardinal getNextPayloadFrameSizeForDelayAndSize(const double   frameRate,
                                                   const cardinal bufferDelay,
                                                   const cardinal frameSize) const;
   /**
     * Implementation of FrameSizeScalabilityInterface.
     *
     * @see FrameSizeScalabilityInterface#getPrevFrameSizeForDelayAndSize
     */
   cardinal getPrevPayloadFrameSizeForDelayAndSize(const double   frameRate,
                                                   const cardinal bufferDelay,
                                                   const cardinal frameSize) const;

};


ConstantBitrateAudioFrameSizeScalability::ConstantBitrateAudioFrameSizeScalability()
{
   MinFrameSize = AudioQuality::ValidRatesTable[0];
   MaxFrameSize = AudioQuality::ValidRatesTable[AudioQuality::ValidRates - 1];
}


cardinal ConstantBitrateAudioFrameSizeScalability::getPrevPayloadFrameSizeForDelayAndSize(
                                                      const double   frameRate,
                                                      const cardinal bufferDelay,
                                                      const cardinal frameSize) const
{
   integer i = (integer)AudioQuality::ValidRates - 1;
   while(i >= 0) {
      cardinal size = (cardinal)ceil((double)AudioQuality::ValidRatesTable[i] / frameRate);
      if(size < frameSize) {
         return(size);
      }
      i--;
   }
   return(0);
}


cardinal ConstantBitrateAudioFrameSizeScalability::getNextPayloadFrameSizeForDelayAndSize(
                                                      const double   frameRate,
                                                      const cardinal bufferDelay,
                                                      const cardinal frameSize) const
{
   cardinal i = 0;
   while(i < AudioQuality::ValidRates) {
      cardinal size = (cardinal)ceil((double)AudioQuality::ValidRatesTable[i] / frameRate);
      if(size > frameSize) {
         return(size);
      }
      i++;
   }
   return((cardinal)ceil((double)AudioQuality::ValidRatesTable[AudioQuality::ValidRates - 1] / frameRate));
}


int main(int argc, char** argv)
{
   ConstantBitrateAudioFrameSizeScalability x;
   cardinal r = 0,r2=0;
   for(int i = 0;i < 20;i++) {
      r2 = x.getNextPayloadFrameSizeForDelayAndSize(1.0,1,r);
      printf("up   %02d: %6d -> %6d\n",i,r,r2);
      if(r2 == AudioQuality::HighestSamplingRate) break;
      r = r2;
   }

   puts("------------------------------");

   for(int i = 0;i < 20;i++) {
      r2 = x.getPrevPayloadFrameSizeForDelayAndSize(1.0,1,r);
      printf("down %02d: %6d -> %6d\n",i,r,r2);
      if(r2 == AudioQuality::LowestSamplingRate) break;
      r = r2;
   }
}
