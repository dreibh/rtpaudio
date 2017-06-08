// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client                                                     ####
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


#include "multiaudiowriter.h"
#include "audiodevice.h"
#include "audiodebug.h"
#include "audionull.h"
#include "tdsocket.h"
#include "audioclient.h"
#include "breakdetector.h"
#include "audioquality.h"
#include "strings.h"

#include <assert.h>
#include <fstream>


// Fast Break: Disable break detector to debug thread deadlocks
// #define FAST_BREAK


// Globals
AudioWriterInterface* audioOutput = NULL;
AudioClient*          client      = NULL;
std::ofstream*        gpScript    = NULL;
std::ofstream*        gpData      = NULL;


// ###### Clean up ##########################################################
void cleanUp(const cardinal exitCode = 0)
{
   if(client != NULL)
      delete client;
   if(audioOutput != NULL)
      delete audioOutput;
   if(exitCode == 0)
      std::cout << "Terminated!" << std::endl;
   exit(exitCode);
}


// ###### Initialize GNUplot output streams #################################
void initGNUplot(const char*    prefix,
                 const char*    info,
                 const cardinal layers,
                 const String&  address)
{
   const String scriptName = String(prefix) + ".gp";
   const String dataName   = String(prefix) + ".dat";
   time_t timeStamp = getMicroTime() / 1000000;
   const struct tm* timeptr = localtime(&timeStamp);
   char str[256];
   strftime((char*)&str,256,"%A, %B %d %G %T %Z",timeptr);
   String infoString = "Generated on " + (String)str;
   if(info) {
      infoString = infoString + " " + (String)info;
   }

   gpScript = new std::ofstream(scriptName.getData());
   if(gpScript != NULL) {
      if(gpScript->good()) {
         gpData = new std::ofstream(dataName.getData());
         if(gpData != NULL) {
            if(gpData->good()) {
               *gpScript << "set terminal postscript enhanced color dashed" << std::endl;
               *gpScript << "set encoding iso_8859_1" << std::endl;
               for(cardinal i = 1;i < 33;i++) {
                  *gpScript << "set linestyle " << i << " lt " << i << " lw 5" << std::endl;
               }
               *gpScript << "set grid" << std::endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Bytes Received\" \"Times-Roman,14\"" << std::endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << std::endl;
               *gpScript << "set ylabel \"Bytes [1]\"" << std::endl;
               *gpScript << "set xlabel \"Time [s]\"" << std::endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << std::endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Packets Received\" \"Times-Roman,14\"" << std::endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << std::endl;
               *gpScript << "set ylabel \"Packets [1]\"" << std::endl;
               *gpScript << "set xlabel \"Time [s]\"" << std::endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 2 * (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << std::endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Packets Lost\" \"Times-Roman,14\"" << std::endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << std::endl;
               *gpScript << "set ylabel \"Packets Lost [1]\"" << std::endl;
               *gpScript << "set xlabel \"Time [s]\"" << std::endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 3 * (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + 2 * (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << std::endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Fraction Lost\" \"Times-Roman,14\"" << std::endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << std::endl;
               *gpScript << "set ylabel \"Fraction Lost [%]\"" << std::endl;
               *gpScript << "set xlabel \"Time [s]\"" << std::endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 4 * (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + 3 * (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << std::endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Interarrival Jitter\" \"Times-Roman,14\"" << std::endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << std::endl;
               *gpScript << "set ylabel \"Interarrival Jitter [us]\"" << std::endl;
               *gpScript << "set xlabel \"Time [s]\"" << std::endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 5 * (layers + 1) << " title "
                         << "\"Average\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + 4 * (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << std::endl;
               return;
            }
            else {
               std::cerr << "ERROR: Unable to create data file " << dataName << "!" << std::endl;
            }
            delete gpData;
         }
      }
      else {
         std::cerr << "ERROR: Unable to create script file " << scriptName << "!" << std::endl;
      }
      delete gpScript;
   }
   exit(1);
}


// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Check arguments =================================================
   if(argc < 2) {
      std::cerr << "Usage: " << argv[0] << std::endl
           << "[URL] {[+/-]debug} {[+/-]null} {[+/-]device} {-encoding=number} {-prefix=name} {-info=infostring} {-force-ipv4}" << std::endl;
      exit(0);
   }
   bool            optAudioDevice = true;
   bool            optAudioDebug  = false;
   bool            optAudioNull   = false;
   bool            optForceIPv4   = false;
   const char*     info           = "";
   cardinal        encoding       = 0;
   char*           prefix         = NULL;
   integer         rate           = AudioQuality::HighestSamplingRate;
   integer         bits           = AudioQuality::HighestBits;
   bool            stereo         = true;
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strcasecmp(argv[i],"+debug")))               optAudioDebug  = 1;
      else if(!(strcasecmp(argv[i],"+null")))           optAudioNull   = 1;
      else if(!(strcasecmp(argv[i],"+device")))         optAudioDevice = 1;
      else if(!(strcasecmp(argv[i],"-debug")))          optAudioDebug  = 0;
      else if(!(strcasecmp(argv[i],"-null")))           optAudioNull   = 0;
      else if(!(strcasecmp(argv[i],"-device")))         optAudioDevice = 0;
      else if(!(strcasecmp(argv[i],"-force-ipv4")))     optForceIPv4  = true;
      else if(!(strcasecmp(argv[i],"-use-ipv4")))       optForceIPv4  = false;
      else if(!(strncasecmp(argv[i],"-prefix=",8)))     prefix        = &argv[i][8];
      else if(!(strncasecmp(argv[i],"-info=",6)))       info          = &argv[i][6];
      else if(!(strncasecmp(argv[i],"-rate=",6)))       rate          = atol(&argv[i][6]);
      else if(!(strncasecmp(argv[i],"-bits=",6)))       bits          = atol(&argv[i][6]);
      else if(!(strcasecmp(argv[i],"-stereo")))         stereo        = true;
      else if(!(strcasecmp(argv[i],"-mono")))           stereo        = false;
      else if(!(strncasecmp(argv[i],"-encoding=",10)))  encoding      = atol(&argv[i][10]);
      else if(argv[i][0] == '-') {
         std::cerr << "Wrong parameter: " << argv[i] << std::endl;
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         std::cerr << "NOTE: IPv6 support disabled!" << std::endl;
      }
   }


   // ====== Initialize audio output device =================================
   MultiAudioWriter* audioOutput = new MultiAudioWriter();
   assert(audioOutput != NULL);
   if(optAudioDebug) {
      AudioDebug* audioDebug = new AudioDebug();
      assert(audioDebug != NULL);
      audioOutput->addWriter(audioDebug);
   }
   if(optAudioNull) {
      AudioNull* audioNull = new AudioNull();
      assert(audioNull != NULL);
      audioOutput->addWriter(audioNull);
   }
   if(optAudioDevice) {
      AudioDevice* audioDevice = new AudioDevice();
      assert(audioDevice != NULL);
      if(!audioOutput->ready()) {
         std::cerr << "WARNING: Unable to open audio device!" << std::endl;
      }
      audioOutput->addWriter(audioDevice);
   }


   // ====== Initialize Audio client ========================================
   client = new AudioClient(audioOutput);
   if(client == NULL) {
      std::cerr << "ERROR: Client::main() - Out of memory!" << std::endl;
      cleanUp(1);
   }


   // ====== Set encoding and quality =======================================
   client->setEncoding(encoding);
   for(cardinal i = 0;i < AudioQuality::ValidRates;i++) {
      if(rate >= AudioQuality::ValidRatesTable[i]) {
         client->setSamplingRate(AudioQuality::ValidRatesTable[i]);
      }
   }
   for(cardinal i = 0;i < AudioQuality::ValidBits;i++) {
      if(bits >= AudioQuality::ValidBitsTable[i]) {
         client->setBits(AudioQuality::ValidBitsTable[i]);
      }
   }
   client->setChannels((stereo == true) ? 2 : 1);


   // ====== Start playing ==================================================
   const bool ok = client->play(argv[1]);
   if(ok == false) {
      std::cerr << "ERROR: Client::main() - AudioClient::play() failed!" << std::endl;
      cleanUp(1);
   }
#ifndef FAST_BREAK
   installBreakDetector();
#endif

   const cardinal layers = 3; // ??????
   if(prefix != NULL) {
      initGNUplot(prefix,info,layers,client->getOurAddressString(InternetAddress::PF_Full));
   }


   // ====== Print status ===================================================
   std::cout << "RTP Audio Client - Copyright (C) 1999-2017 Thomas Dreibholz" << std::endl
             << "-----------------------------------------------------------" << std::endl
             << std::endl
             << "Server Address:  " << client->getServerAddressString() << std::endl;
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",client->getOurSSRC());
   std::cout << "Client SSRC:     " << str     << std::endl
             << "Media URL:       " << argv[1] << std::endl
             << "Layers:          " << layers  << std::endl
             << std::endl;


   // ====== Main loop ======================================================
   cardinal EOFRepeatDelay = 0;
   card64   oldBytes[layers];
   card64   oldPackets[layers];
   card64   oldPacketsLost[layers];
   for(cardinal i = 0;i < layers;i++) {
      oldBytes[i] = oldPackets[i] = oldPacketsLost[i] = 0;
   }
   card64   startTS    = getMicroTime() - 1;
   card64   oldTS      = startTS;         // avoid now=oldTS (-> divide by 0)
   while(client->playing()) {
      // ====== Check for errors ============================================
      const card8 error = client->getErrorCode();
      switch(error) {
         case ME_NoError:
         case ME_NoMedia:
          break;
         case ME_EOF:
            EOFRepeatDelay++;
            if(EOFRepeatDelay > 15) {
               EOFRepeatDelay = 0;
               client->setPosition(0);
            }
          break;
         default:
            std::cerr << std::endl << "Encoder error while playing: #" << (int)error << "." << std::endl;
            goto cleanUp;
          break;
      }
#ifndef FAST_BREAK
      if(breakDetected()) {
         goto cleanUp;
      }
#endif


      // ====== Print information ===========================================
      const card64 position = client->getPosition();
      const card64 seconds  = position / PositionStepsPerSecond;
      card64 bytes[layers];
      card64 packets[layers];
      card64 packetsLost[layers];
      double fractionLost[layers];
      double jitter[layers];
      for(cardinal i = 0;i < layers;i++) {
         bytes[i]        = client->getBytesReceived(i);
         packets[i]      = client->getPacketsReceived(i);
         packetsLost[i]  = client->getPacketsLost(i);
         fractionLost[i] = client->getFractionLost(i);
         jitter[i]       = client->getJitter(i);
      }
      const cardinal currentLayers = client->getLayers();
      const card64 now = getMicroTime();
      if(!optAudioDebug) {
         printf("\x0d%2u:%02u.%02u   [Quality: %d Hz / %d Bit / %s]  [%s]      ",
                (unsigned int)(seconds / 60), (unsigned int)(seconds % 60),
                (unsigned int)((position % PositionStepsPerSecond) / (PositionStepsPerSecond / 100)),
                client->getSamplingRate(),
                client->getBits(),
                ((client->getChannels() == 2) ? "Stereo" : "Mono"),
                client->getEncoding());
         fflush(stdout);
      }


      // ====== Write GNUplot output ========================================
      if((gpData != NULL) && (currentLayers > 0)) {
         card64 oldBytesTotal = 0;
         card64 bytesTotal    = 0;
         *gpData << (double)(now - startTS) / 1000000.0 << " ";
         for(cardinal i = 0;i < layers;i++) {
            oldBytesTotal += oldBytes[i];
            bytesTotal    += bytes[i];
            const card64 bps = (bytes[i] - oldBytes[i]) * 1000000 / (now - oldTS);
            *gpData << bps << " ";
         }
         *gpData << (bytesTotal - oldBytesTotal) * 1000000 / (now - oldTS) << " ";

         card64 oldPacketsTotal = 0;
         card64 packetsTotal    = 0;
         for(cardinal i = 0;i < layers;i++) {
            oldPacketsTotal += oldPackets[i];
            packetsTotal    += packets[i];
            const card64 pps = (packets[i] - oldPackets[i]) * 1000000 / (now - oldTS);
            *gpData << pps << " ";
         }
         *gpData << (packetsTotal - oldPacketsTotal) * 1000000 / (now - oldTS) << " ";

         card64 oldPacketsLostTotal = 0;
         card64 totalPacketsLost    = 0;
         for(cardinal i = 0;i < layers;i++) {
            oldPacketsLostTotal += oldPacketsLost[i];
            totalPacketsLost += packetsLost[i];
            *gpData << (packetsLost[i] - oldPacketsLost[i]) << " ";
         }
         *gpData << (totalPacketsLost - oldPacketsLostTotal) << " ";

         double avgFractionLost = 0.0;
         for(cardinal i = 0;i < layers;i++) {
            avgFractionLost += fractionLost[i];
            *gpData << 100.0 * fractionLost[i] << " ";
         }
         avgFractionLost /= (double)currentLayers;
         *gpData << avgFractionLost * 100.0 << " ";

         double avgJitter = 0.0;
         for(cardinal i = 0;i < layers;i++) {
            avgJitter += jitter[i];
            *gpData << jitter[i] << " ";
         }
         avgJitter /= (double)currentLayers;
         *gpData << avgJitter;

         *gpData << std::endl;
      }


      // ====== Wait ========================================================
      for(cardinal i = 0;i < layers;i++) {
         oldBytes[i]       = bytes[i];
         oldPackets[i]     = packets[i];
         oldPacketsLost[i] = packetsLost[i];
      }
      oldTS      = now;
      Thread::delay(1000000,true);
   }


   // ====== Clean up =======================================================
cleanUp:
   cleanUp(0);
}
