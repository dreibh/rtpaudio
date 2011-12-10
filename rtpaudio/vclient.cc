// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Verification Client                                              ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2012 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de.de                               ####
// ####    WWW:   http://www.iem.uni-due.de.de/~dreibh/rn                ####
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


const cardinal DefaultPause      = 500;
const cardinal DefaultThreads    = 12;
char*          DefaultServer     = "localhost:7500";
char*          DefaultMedia      = "TestWav%d.list";
const cardinal DefaultMediaCount = 1;


class VerificationClientThread : public Thread
{
   // ====== Constructor ====================================================
   public:
   VerificationClientThread(const cardinal id,
                            const char*    localName,
                            const char*    server = DefaultServer,
                            const char*    media  = DefaultMedia,
                            const cardinal count  = DefaultMediaCount,
                            const cardinal pause  = DefaultPause,
                            const double   prSelectEncoding = 0.1,
                            const double   prSelectQuality  = 0.7,
                            const double   prSelectPosition = 0.3,
                            const double   prSelectMedia    = 0.05,
                            const double   prStop           = 0.03,
                            const double   prRestart        = 0.01,
                            const bool     useSCTP          = false);


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
   const char*  Server;
   const char*  Media;
   cardinal     MediaCount;
   cardinal     Pause;
   double       PrSelectEncoding;
   double       PrSelectQuality;
   double       PrSelectPosition;
   double       PrSelectMedia;
   double       PrStop;
   double       PrRestart;

   String       LocalName;
   bool         UseSCTP;
};


// ###### Constructor ######################################################
VerificationClientThread::VerificationClientThread(
                             const cardinal id,
                             const char*    localName,
                             const char*    server,
                             const char*    media,
                             const cardinal count,
                             const cardinal pause,
                             const double   prSelectEncoding,
                             const double   prSelectQuality,
                             const double   prSelectPosition,
                             const double   prSelectMedia,
                             const double   prStop,
                             const double   prRestart,
                             const bool     useSCTP)
   : Thread("VClient")
{
   ID         = id;
   LocalName  = String(localName);

   Server     = server;
   Media      = media;
   MediaCount = count;
   Pause      = pause;
   UseSCTP    = useSCTP;

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
            "#%02d =>  $%08x: %3Ld:%02Ld.%02Ld   %5d Hz / %2d Bits / %6s   %s",
            ID,SSRC,
            seconds / 60, seconds % 60,Position % 100,
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
      Client = new AudioClient(NULL,0,(AudioWriterInterface*)&OutputDevice);
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
   cout << "#" << ID << ": " << str << endl;
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
              "Select position %Ld:%02Ld.%02Ld.",
              seconds / 60, seconds % 60, (pos / (PositionStepsPerSecond / 100)) % 100);
      writeLog((char*)&str);
      Client->setPosition(pos);
   }
}


// ###### Select media ######################################################
void VerificationClientThread::selectMedia()
{
   char str[128];
   cardinal number = Random.random(1,MediaCount);
   snprintf((char*)&str,sizeof(str),Media,(int)number);

   if(Client->playing()) {
      Client->change((char*)&str);
   }
   else {
      if(Client->play(Server,(char*)&str,UseSCTP) == false) {
         cerr << "ERROR: Connection to server failed!" << endl;
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
   bool     optForceIPv4     = false;
   bool     optUseSCTP       = false;
   double   prSelectEncoding = 0.1;
   double   prSelectQuality  = 0.7;
   double   prSelectPosition = 0.3;
   double   prSelectMedia    = 0.05;
   double   prStop           = 0.03;
   double   prRestart        = 0.01;
   char*    server           = DefaultServer;
   char*    media            = DefaultMedia;
   char*    receiverName     = NULL;
   cardinal count            = DefaultMediaCount;
   cardinal pause            = DefaultPause;
   cardinal threads          = DefaultThreads;
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strncasecmp(argv[i],"-se=",4)))       prSelectEncoding = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-sq=",4)))  prSelectQuality  = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-sp=",4)))  prSelectPosition = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-sm=",4)))  prSelectMedia    = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-st=",4)))  prStop           = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-re=",4)))  prRestart        = atof(&argv[i][4]);
      else if(!(strncasecmp(argv[i],"-threads=",9))) threads = atol(&argv[i][9]);
      else if(!(strncasecmp(argv[i],"-server=",8)))  server  = &argv[i][8];
      else if(!(strncasecmp(argv[i],"-media=",7)))   media   = &argv[i][7];
      else if(!(strncasecmp(argv[i],"-count=",7)))   count   = atol(&argv[i][7]);
      else if(!(strncasecmp(argv[i],"-pause=",7)))   pause   = atol(&argv[i][7]);
      else if(!(strcasecmp(argv[i],"-force-ipv4")))  optForceIPv4 = true;
      else if(!(strcasecmp(argv[i],"-use-ipv4")))    optForceIPv4 = false;
      else if(!(strcasecmp(argv[i],"-sctp")))        optUseSCTP   = true;
      else if(!(strcasecmp(argv[i],"-nosctp")))      optUseSCTP   = false;
      else if(!(strncasecmp(argv[i],"-local=",7)))   receiverName = &argv[i][7];
      else {
         cerr << "Usage: " << argv[0] << endl
              << "   {-local=hostname}" << endl
              << "   {-threads=count} {-pause=milliseconds}" << endl
              << "   {-server=host:port} {-media=name with %%d} {-count=media count}" << endl
              << "   {-se=probability} {-sq=probability} {-sp=probability}" << endl
              << "   {-sm=probability} {-st=probability} {-re=probability}" << endl;
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
         cerr << "NOTE: IPv6 support disabled!" << endl;
      }
   }


   // ====== Print information ==============================================
   cerr << "RTP Audio Verification Client - Copyright (C) 1999-2001 Thomas Dreibholz" << endl;
   cerr << "------------------------------------------------------------------------" << endl;
   cerr << endl;
   cout << "Version:       " << __DATE__ << ", " << __TIME__ << endl;
   if(optUseSCTP) {
      cerr << "SCTP:          on" << endl;
   }
   else {
      cerr << "SCTP:          off" << endl;
   }
   if(receiverName != NULL) {
      cerr << "Local Address: " << receiverName << endl;
   }
   cerr << endl;
   cerr << "Threads                     = " << threads << endl;
   cerr << "Server                      = " << server << endl;
   cerr << "Media name                  = " << media << endl;
   cerr << "Media count                 = " << count << endl;
   cerr << "Maximum pause               = " << pause << " [ms]" << endl;
   cerr << "Select encoding probability = " << prSelectEncoding << endl;
   cerr << "Select quality probability  = " << prSelectQuality << endl;
   cerr << "Select position probability = " << prSelectPosition << endl;
   cerr << "Select media probability    = " << prSelectMedia << endl;
   cerr << "Restart probability         = " << prStop << endl;
   cerr << "Client restart probability  = " << prRestart << endl;
   cerr << endl;
   Thread::delay(1000000);


   // ====== Initialize threads =============================================
   VerificationClientThread* thread[threads];
   for(cardinal i = 0;i < threads;i++) {
      thread[i] = new VerificationClientThread(
                         i + 1,
                         receiverName,
                         server, media, count, pause,
                         prSelectEncoding, prSelectQuality, prSelectPosition,
                         prSelectMedia, prStop, prRestart,
                         optUseSCTP);
      char str[256];
      snprintf((char*)&str,sizeof(str),"VClient Thread #%02d",i);
      thread[i]->start((char*)&str);
   }


   // ====== Main loop ======================================================
   installBreakDetector();
   do {
      Thread::delay(1000000,true);
      printTimeStamp(cerr);
      cerr << endl;
      for(cardinal i = 0;i < threads;i++) {
         char str[512];
         if(breakDetected()) {
            // Status printing may take a lot of time due to synchronization!
            // -> Check for break every iteration.
            break;
         }
         cerr << thread[i]->getStatusString((char*)&str,sizeof(str)) << endl;
      }
      cerr << endl;
   } while(!breakDetected());


   // ====== Clean-up =======================================================
   for(cardinal i = 0;i < threads;i++) {
      thread[i]->stop();
   }
   cerr << "Terminated!" << endl;
   return(0);
}
