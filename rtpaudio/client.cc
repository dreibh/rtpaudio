// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Audio Client                                                     ####
// ####                                                                  ####
// #### Version 1.50  --  August 01, 2001                                ####
// ####                                                                  ####
// ####            Copyright (C) 1999-2001 by Thomas Dreibholz           ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@exp-math.uni-essen.de                           ####
// ####    WWW:   http://www.exp-math.uni-essen.de/~dreibh/rtpaudio      ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software; you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License      ####
// #### as published by the Free Software Foundation; either version 2   ####
// #### of the License, or (at your option) any later version.           ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// ##########################################################################


#include "audiodevice.h"
#include "audiodebug.h"
#include "audionull.h"
#include "tdsocket.h"
#include "audioclient.h"
#include "breakdetector.h"
#include "audioquality.h"
#include "strings.h"

#include <fstream>


#define SCTP_MAXADDRESSES 20


// Fast Break: Disable break detector to debug thread deadlocks
// #define FAST_BREAK


// Globals
AudioWriterInterface* audioOutput = NULL;
AudioClient*          client      = NULL;
ofstream*             gpScript    = NULL;
ofstream*             gpData      = NULL;


// ###### Clean up ##########################################################
void cleanUp(const cardinal exitCode = 0)
{
   if(client != NULL)
      delete client;
   if(audioOutput != NULL)
      delete audioOutput;
   if(exitCode == 0)
      cout << "Terminated!" << endl;
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

   gpScript = new ofstream(scriptName.getData());
   if(gpScript != NULL) {
      if(gpScript->good()) {
         gpData = new ofstream(dataName.getData());
         if(gpData != NULL) {
            if(gpData->good()) {
               *gpScript << "set terminal postscript enhanced color dashed" << endl;
               *gpScript << "set encoding iso_8859_1" << endl;
               for(cardinal i = 1;i < 33;i++) {
                  *gpScript << "set linestyle " << i << " lt " << i << " lw 5" << endl;
               }
               *gpScript << "set grid" << endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Bytes Received\" \"Times-Roman,14\"" << endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << endl;
               *gpScript << "set ylabel \"Bytes [1]\"" << endl;
               *gpScript << "set xlabel \"Time [s]\"" << endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Packets Received\" \"Times-Roman,14\"" << endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << endl;
               *gpScript << "set ylabel \"Packets [1]\"" << endl;
               *gpScript << "set xlabel \"Time [s]\"" << endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 2 * (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Packets Lost\" \"Times-Roman,14\"" << endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << endl;
               *gpScript << "set ylabel \"Packets Lost [1]\"" << endl;
               *gpScript << "set xlabel \"Time [s]\"" << endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 3 * (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + 2 * (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Fraction Lost\" \"Times-Roman,14\"" << endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << endl;
               *gpScript << "set ylabel \"Fraction Lost [%]\"" << endl;
               *gpScript << "set xlabel \"Time [s]\"" << endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 4 * (layers + 1) << " title "
                         << "\"Total\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + 3 * (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << endl;

               *gpScript << "set title \"RTP Audio Client " << address << " - Interarrival Jitter\" \"Times-Roman,14\"" << endl;
               *gpScript << "set timestamp \"" << infoString << "\" 0,0 \"Times-Roman,12\"" << endl;
               *gpScript << "set ylabel \"Interarrival Jitter [us]\"" << endl;
               *gpScript << "set xlabel \"Time [s]\"" << endl;
               *gpScript << "plot \"" << dataName << "\" using 1:"
                         << 1 + 5 * (layers + 1) << " title "
                         << "\"Average\" with lines ls 1";
               for(cardinal i = 0;i < layers;i++) {
                  *gpScript << ", \"" << dataName << "\" using 1:"
                            << 1 + 4 * (layers + 1) + (i + 1) << " title "
                            << "\"Layer #" << i << "\" with lines ls " << (i + 2);
               }
               *gpScript << endl;
               return;
            }
            else {
               cerr << "ERROR: Unable to create data file " << dataName << "!" << endl;
            }
            delete gpData;
         }
      }
      else {
         cerr << "ERROR: Unable to create script file " << scriptName << "!" << endl;
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
      cerr << "Usage: " << argv[0] << endl
           << "[URL] {-local=host{:Port}} {-debug} {-null} {-encoding=number} {-prefix=name} {-info=infostring} {-force-ipv4}" << endl;
      exit(0);
   }
   bool            optAudioDebug = false;
   bool            optAudioNull  = false;
   bool            optForceIPv4  = false;
   bool            optUseSCTP    = false;
   cardinal        encoding      = 0;
   char*           info          = "";
   char*           prefix        = NULL;
   integer         rate          = AudioQuality::HighestSamplingRate;
   integer         bits          = AudioQuality::HighestBits;
   bool            stereo        = true;
   SocketAddress** localAddressArray = NULL;
   cardinal        localAddresses    = 0;
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strcasecmp(argv[i],"-debug")))           optAudioDebug = true;
      else if(!(strcasecmp(argv[i],"-null")))       optAudioNull  = true;
      else if(!(strcasecmp(argv[i],"-force-ipv4"))) optForceIPv4  = true;
      else if(!(strcasecmp(argv[i],"-use-ipv4")))   optForceIPv4  = false;
      else if(!(strcasecmp(argv[i],"-sctp")))       optUseSCTP    = true;
      else if(!(strcasecmp(argv[i],"-nosctp")))     optUseSCTP    = false;
      else if(!(strncasecmp(argv[i],"-local=",7))) {
         if(localAddresses < SCTP_MAXADDRESSES) {
            if(localAddressArray == NULL) {
               localAddressArray = SocketAddress::newAddressList(SCTP_MAXADDRESSES);
               if(localAddressArray == NULL) {
                  cerr << "ERROR: Out of memory!" << endl;
                  exit(1);
               }
            }
            localAddressArray[localAddresses] = SocketAddress::createSocketAddress(
                                                   SocketAddress::PF_HidePort,
                                                   &argv[i][7]);
            if(localAddressArray[localAddresses] == NULL) {
               cerr << "ERROR: Argument #" << i << " is an invalid address!" << endl;
               exit(1);
            }
            localAddresses++;
         }
         else {
            cerr << "ERROR: Too many local addresses!" << endl;
            exit(1);
         }
      }
      else if(!(strncasecmp(argv[i],"-prefix=",8))) prefix        = &argv[i][8];
      else if(!(strncasecmp(argv[i],"-info=",6)))   info          = &argv[i][6];
      else if(!(strncasecmp(argv[i],"-rate=",6)))   rate          = atol(&argv[i][6]);
      else if(!(strncasecmp(argv[i],"-bits=",6)))   bits          = atol(&argv[i][6]);
      else if(!(strcasecmp(argv[i],"-stereo")))         stereo    = true;
      else if(!(strcasecmp(argv[i],"-mono")))           stereo    = false;
      else if(!(strncasecmp(argv[i],"-encoding=",10)))  encoding  = atol(&argv[i][10]);
      else if(argv[i][0] == '-') {
         cerr << "Wrong parameter: " << argv[i] << endl;
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         cerr << "NOTE: IPv6 support disabled!" << endl;
      }
   }
   if(localAddressArray == NULL) {
      if(optUseSCTP == true) {
         if(!Socket::getLocalAddressList(
               localAddressArray,
               localAddresses,
               Socket::GLAF_HideBroadcast|Socket::GLAF_HideMulticast|Socket::GLAF_HideAnycast)) {
            cerr << "ERROR: Cannot obtain local addresses!" << endl;
            exit(1);
         }
         if(localAddresses < 1) {
            cerr << "ERROR: No valid local addresses have been found?!" << endl
                 << "       Check your network interface configuration!" << endl;
            exit(1);
         }
      }
      else {
         localAddressArray = SocketAddress::newAddressList(SCTP_MAXADDRESSES);
         if(localAddressArray == NULL) {
            cerr << "ERROR: Out of memory!" << endl;
            exit(1);
         }
         localAddressArray[0] = new InternetAddress(0);
         if(localAddressArray[0] == NULL) {
            cerr << "ERROR: Out of memory!" << endl;
            exit(1);
         }
         localAddresses = 1;
      }
   }


   // ====== Get server address and media name ==============================
   String protocol;
   String host;
   String path;
   bool ok = scanURL(argv[1],protocol,host,path);
   if((ok == false) || (protocol != "rtpa")) {
      cerr << "ERROR: Invalid URL! Check URL and try again." << endl;
      cerr << "       Example: rtpa://gaffel:7500/Test.list" << endl;
      cleanUp(1);
   }
   InternetAddress serverAddress(host);
   if(!serverAddress.isValid()) {
      cerr << "ERROR: Invalid server address! Check URL and try again." << endl;
      cleanUp(1);
   }

   // ====== Initialize audio output device =================================
   if(optAudioDebug) {
      audioOutput = new AudioDebug();
   }
   else if(optAudioNull) {
      audioOutput = new AudioNull();
   }
   else {
      audioOutput = new AudioDevice();
      if((audioOutput != NULL) && (!audioOutput->ready())) {
         cerr << "WARNING: Unable to open audio device - Using AudioNull!" << endl;
         audioOutput = new AudioNull();
      }
   }
   if(audioOutput == NULL) {
      cerr << "ERROR: Client::main() - Out of memory!" << endl;
      cleanUp(1);
   }


   // ====== Initialize Audio client ========================================
   client = new AudioClient(localAddressArray,localAddresses,audioOutput);
   if(client == NULL) {
      cerr << "ERROR: Client::main() - Out of memory!" << endl;
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
   ok = client->play(host.getData(),path.getData(),optUseSCTP);
   if(ok == false) {
      cerr << "ERROR: Client::main() - AudioClient::play() failed!" << endl;
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
   cout << "RTP Audio Client - Copyright (C) 1999-2002 Thomas Dreibholz" << endl;
   cout << "-----------------------------------------------------------" << endl;
   cout << endl;
   cout << "Version:         " << __DATE__ << ", " << __TIME__ << endl;
   if(optUseSCTP) {
      cout << "SCTP:            on" << endl;
   }
   else {
      cout << "SCTP:            off" << endl;
   }
   localAddressArray[0]->setPrintFormat(SocketAddress::PF_Address|SocketAddress::PF_HidePort);
   cout << "Local Addresses: " << *(localAddressArray[0]) << endl;
   for(cardinal i = 1;i < localAddresses;i++) {
      localAddressArray[i]->setPrintFormat(SocketAddress::PF_Address|SocketAddress::PF_HidePort);
      cout << "                 " << *(localAddressArray[i]) << endl;
   }
   cout << "Server Address:  " << client->getServerAddressString() << endl;
   cout << endl;
   char str[32];
   snprintf((char*)&str,sizeof(str),"$%08x",client->getOurSSRC());
   cout << "Client SSRC:     " << str << endl;
   cout << "Media Name:      " << path << endl;
   cout << "Layers:          " << layers << endl;
   cout << endl;


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
            cerr << endl << "Encoder error while playing: #" << (int)error << "." << endl;
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
         printf("\x0d%2Ld:%02Ld.%02Ld   [Quality: %d Hz / %d Bit / %s]  [%s]      ",
                (seconds / 60),(seconds % 60),
                (position % PositionStepsPerSecond) / (PositionStepsPerSecond / 100),
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

         *gpData << endl;
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
   SocketAddress::deleteAddressList(localAddressArray);
   cleanUp(0);
}
