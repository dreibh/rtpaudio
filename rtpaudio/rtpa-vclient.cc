// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Verification Client                                              ####
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
// $Id$


#include "tdsystem.h"
#include "audionull.h"
#include "audioclient.h"
#include "randomizer.h"
#include "thread.h"
#include "mediainfo.h"
#include "breakdetector.h"

#include <sys/types.h>
#include <signal.h>


const cardinal DefaultPause      = 500;
const cardinal DefaultThreads    = 12;
const char*    DefaultURL        = "rtpa+udp://localhost:7500/Test%u.list";
const cardinal DefaultMediaCount = 4;


class VerificationClientThread : public Thread
{
   // ====== Constructor ====================================================
   public:
   VerificationClientThread(const cardinal id,
                            const char*    localName,
                            const char*    url              = DefaultURL,
                            const cardinal count            = DefaultMediaCount,
                            const cardinal pause            = DefaultPause,
                            const double   prSelectEncoding = 0.1,
                            const double   prSelectQuality  = 0.7,
                            const double   prSelectPosition = 0.3,
                            const double   prSelectMedia    = 0.05,
                            const double   prStop           = 0.03,
                            const double   prRestart        = 0.01);


   // ====== Status functions ===============================================
   char* getStatusString(char* str, const cardinal maxLength);
   void* stop();


   // ====== Private data ===================================================
   private:
   void run();
   void test();
   void writeLog(const char* str);
   void selectEncoding();
   void selectQuality();
   void selectPosition();
   void selectMedia();
   void restart();


   Randomizer   Random;
   AudioNull    OutputDevice;
   AudioClient* Client;

   card32       SSRC;
   card64       Position;
   AudioQuality Quality;
   const char*  Encoding;

   cardinal     ID;
   const char*  URL;
   cardinal     MediaCount;
   cardinal     Pause;
   double       PrSelectEncoding;
   double       PrSelectQuality;
   double       PrSelectPosition;
   double       PrSelectMedia;
   double       PrStop;
   double       PrRestart;

   String       LocalName;
};


// ###### Constructor ######################################################
VerificationClientThread::VerificationClientThread(
                             const cardinal id,
                             const char*    localName,
                             const char*    url,
                             const cardinal count,
                             const cardinal pause,
                             const double   prSelectEncoding,
                             const double   prSelectQuality,
                             const double   prSelectPosition,
                             const double   prSelectMedia,
                             const double   prStop,
                             const double   prRestart)
   : Thread("VClient")
{
   ID         = id;
   LocalName  = String(localName);
   URL        = url;
   MediaCount = count;
   Pause      = pause;

   PrSelectEncoding = prSelectEncoding;
   PrSelectQuality  = prSelectQuality;
   PrSelectPosition = prSelectPosition;
   PrSelectMedia    = prSelectMedia;
   PrStop           = prStop;
   PrRestart        = prRestart;
}


// ###### Get last reported position as string #############################
char* VerificationClientThread::getStatusString(char*          str,
                                                const cardinal maxLength)
{
   synchronized();
   const card64 seconds = Position / 1000;
   snprintf(str,maxLength,
            "#%02d =>  $%08x: %3u:%02u.%02u   %5d Hz / %2d Bits / %6s   %s",
            ID,SSRC,
            (unsigned int)(seconds / 60),
            (unsigned int)(seconds % 60),
            (unsigned int)(Position % 100),
            Quality.getSamplingRate(),
            Quality.getBits(),
            (Quality.getChannels() == 2) ? "Stereo" : "Mono",
            Encoding);
   unsynchronized();
   return(str);
}


// ###### Run test ##########################################################
void VerificationClientThread::run()
{
   for(;;) {
      synchronized();
      Client = new AudioClient((AudioWriterInterface*)&OutputDevice);
      unsynchronized();
      if(Client != NULL) {
         test();
         synchronized();
         delete Client;
         Client = NULL;
         unsynchronized();
      }
      else {
         break;
      }
   }
}


// ###### Stop thread #######################################################
void* VerificationClientThread::stop()
{
   void* result = this->Thread::stop();

   synchronized();
   if(Client != NULL) {
      Client->stop();
      delete Client;
      Client = NULL;
   }
   unsynchronized();

   return(result);
}


// ###### Write log entry ###################################################
void VerificationClientThread::writeLog(const char* str)
{
   std::cout << "#" << ID << ": " << str << std::endl;
}


// ###### Select encoding ###################################################
void VerificationClientThread::selectEncoding()
{
   cardinal count;
   for(count = 0;count < 20;count++) {
      if(Client->getEncodingName(count) == NULL)
         break;
   }
   const cardinal index = Random.random(0,count - 1);

   char str[128];
   snprintf((char*)&str,sizeof(str),
            "Select encoding: %s.",Client->getEncodingName(index));
   writeLog((char*)&str);

   Client->setEncoding(index);
}


// ###### Select quality ####################################################
void VerificationClientThread::selectQuality()
{
   AudioQuality quality = AudioQuality::getRandomQuality(&Random);

   char str[128];
   snprintf((char*)&str,sizeof(str),
           "Select quality: %d Hz/%d/%s.",
           quality.getSamplingRate(),quality.getBits(),
           (quality.getChannels() == 1) ? "Mono" : "Stereo");
   writeLog((char*)&str);

   Client->setSamplingRate(quality.getSamplingRate());
   Client->setBits(quality.getBits());
   Client->setChannels(quality.getChannels());
}


// ###### Select position ###################################################
void VerificationClientThread::selectPosition()
{
   card64 max = Client->getMaxPosition();
   if(max > 0) {
      card64 pos = Random.random64() % max;

      char str[128];
      const card64 seconds = pos / PositionStepsPerSecond;
      snprintf((char*)&str,sizeof(str),
              "Select position %u:%02u.%02u.",
              (unsigned int)(seconds / 60),
              (unsigned int)(seconds % 60),
              (unsigned int)((pos / (PositionStepsPerSecond / 100)) % 100));
      writeLog((char*)&str);
      Client->setPosition(pos);
   }
}


// ###### Select media ######################################################
void VerificationClientThread::selectMedia()
{
   char str[128];
   cardinal number = Random.random(1,MediaCount);
   snprintf((char*)&str,sizeof(str),URL,(int)number);

   if(Client->playing()) {
      Client->change((char*)&str);
   }
   else {
      if(Client->play((const char*)&str) == false) {
         std::cerr << "ERROR: Connection to server failed!" << std::endl;
         kill(getpid(),SIGINT);
      }
   }
}


// ###### Restart ###########################################################
void VerificationClientThread::restart()
{
   writeLog("Stop.");
   Client->stop();
   selectMedia();
}


// ###### Run test ##########################################################
void VerificationClientThread::test()
{
   selectMedia();
   for(;;) {
      // ====== Get parameters ==============================================
      synchronized();
      SSRC     = Client->getOurSSRC();
      Position = (cardinal)(Client->getPosition() / (PositionStepsPerSecond / 1000));
      Quality  = AudioQuality(*Client);
      Encoding = Client->getEncoding();
      unsynchronized();


      // ====== Wait ========================================================
      const card64 delay = Pause * (Random.random32() % 1000);
      Thread::delay(delay,true);


      // ====== Selection of a new encoding =================================
      synchronized();
      double r = Random.random();
      if(r < PrSelectEncoding) {
         selectEncoding();
      }
      unsynchronized();

      // ====== Selection of a new quality ==================================
      synchronized();
      r = Random.random();
      if(r < PrSelectQuality) {
         selectQuality();
      }
      unsynchronized();

      // ====== Selection of new position ===================================
      synchronized();
      r = Random.random();
      if(r < PrSelectPosition) {
         selectPosition();
      }
      unsynchronized();

      // ====== Selection of new media ======================================
      synchronized();
      r = Random.random();
      if(r < PrSelectMedia) {
         selectMedia();
      }
      unsynchronized();

      // ====== Stop and start ==============================================
      synchronized();
      r = Random.random();
      if(r < PrStop) {
         restart();
      }
      unsynchronized();

      // ====== Do complete restart =========================================
      synchronized();
      r = Random.random();
      if(r < PrRestart) {
         writeLog("Restart.");
         Client->stop();
         unsynchronized();
         return;
      }
      unsynchronized();

      // ====== Check for error =============================================
      synchronized();
      const card8 error = Client->getErrorCode();
      if(error > ME_UnrecoverableError) {
         writeLog("Media error!");
         Client->stop();
         unsynchronized();
         return;
      }
      unsynchronized();
   }
}


// ###### Validate probability ##############################################
inline void validatePr(double& p)
{
   if(p < 0.0) {
      p = 0.0;
   }
   else if(p > 1.0) {
      p = 1.0;
   }
}



// ###### Main program ######################################################
int main(int argc, char** argv)
{
   // ====== Get arguments ==================================================
   bool        optForceIPv4     = false;
   double      prSelectEncoding = 0.1;
   double      prSelectQuality  = 0.7;
   double      prSelectPosition = 0.3;
   double      prSelectMedia    = 0.05;
   double      prStop           = 0.03;
   double      prRestart        = 0.01;
   const char* receiverName     = NULL;
   const char* url              = DefaultURL;
   cardinal    count            = DefaultMediaCount;
   cardinal    pause            = DefaultPause;
   cardinal    threads          = DefaultThreads;
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strncasecmp(argv[i],"-se=",4)))           prSelectEncoding = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-sq=",4)))      prSelectQuality  = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-sp=",4)))      prSelectPosition = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-sm=",4)))      prSelectMedia    = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-st=",4)))      prStop           = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-re=",4)))      prRestart        = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-threads=",9))) threads          = atol(&argv[i][9]);
      else if(!(strncasecmp(argv[i],"-url=",5)))     url              = &argv[i][5];
      else if(!(strncasecmp(argv[i],"-count=",7)))   count            = atol(&argv[i][7]);
      else if(!(strncasecmp(argv[i],"-pause=",7)))   pause            = atol(&argv[i][7]);
      else if(!(strcasecmp(argv[i],"-force-ipv4")))  optForceIPv4     = true;
      else if(!(strcasecmp(argv[i],"-use-ipv4")))    optForceIPv4     = false;
      else if(!(strncasecmp(argv[i],"-local=",7)))   receiverName     = &argv[i][7];
      else {
         std::cerr << "Usage: " << argv[0] << std::endl
              << "   {-local=hostname}" << std::endl
              << "   {-threads=count} {-pause=milliseconds}" << std::endl
              << "   {-url=URL with %%u} {-count=media count}" << std::endl
              << "   {-se=probability} {-sq=probability} {-sp=probability}" << std::endl
              << "   {-sm=probability} {-st=probability} {-re=probability}" << std::endl;
         exit(1);
      }
   }
   validatePr(prSelectEncoding);
   validatePr(prSelectQuality);
   validatePr(prSelectPosition);
   validatePr(prSelectMedia);
   validatePr(prStop);
   validatePr(prRestart);
   if(threads > 100) {
      threads = 100;
   }
   if(count > 100) {
      count = 100;
   }
   if(pause > 100000) {
      pause = 100000;
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         std::cerr << "NOTE: IPv6 support disabled!" << std::endl;
      }
   }


   // ====== Print information ==============================================
   std::cerr << "RTP Audio Verification Client - Copyright (C) 1999-2017 Thomas Dreibholz" << std::endl;
   std::cerr << "------------------------------------------------------------------------" << std::endl;
   std::cerr << std::endl;
   std::cout << "Version:       " << __DATE__ << ", " << __TIME__ << std::endl;
   if(receiverName != NULL) {
      std::cerr << "Local Address: " << receiverName << std::endl;
   }
   std::cerr << std::endl;
   std::cerr << "Threads                     = " << threads << std::endl;
   std::cerr << "URL                         = " << url << std::endl;
   std::cerr << "Media count                 = " << count << std::endl;
   std::cerr << "Maximum pause               = " << pause << " [ms]" << std::endl;
   std::cerr << "Select encoding probability = " << prSelectEncoding << std::endl;
   std::cerr << "Select quality probability  = " << prSelectQuality << std::endl;
   std::cerr << "Select position probability = " << prSelectPosition << std::endl;
   std::cerr << "Select media probability    = " << prSelectMedia << std::endl;
   std::cerr << "Restart probability         = " << prStop << std::endl;
   std::cerr << "Client restart probability  = " << prRestart << std::endl;
   std::cerr << std::endl;
   Thread::delay(1000000);


   // ====== Initialize threads =============================================
   VerificationClientThread* thread[threads];
   for(cardinal i = 0;i < threads;i++) {
      thread[i] = new VerificationClientThread(
                         i + 1,
                         receiverName,
                         url, count, pause,
                         prSelectEncoding, prSelectQuality, prSelectPosition,
                         prSelectMedia, prStop, prRestart);
      char str[256];
      snprintf((char*)&str,sizeof(str),"VClient Thread #%02d",i);
      thread[i]->start((char*)&str);
   }


   // ====== Main loop ======================================================
   installBreakDetector();
   do {
      Thread::delay(1000000,true);
      printTimeStamp(std::cerr);
      std::cerr << std::endl;
      for(cardinal i = 0;i < threads;i++) {
         char str[512];
         if(breakDetected()) {
            // Status printing may take a lot of time due to synchronization!
            // -> Check for break every iteration.
            break;
         }
         std::cerr << thread[i]->getStatusString((char*)&str,sizeof(str)) << std::endl;
      }
      std::cerr << std::endl;
   } while(!breakDetected());


   // ====== Clean-up =======================================================
   for(cardinal i = 0;i < threads;i++) {
      thread[i]->stop();
   }
   std::cerr << "Terminated!" << std::endl;
   return(0);
}
