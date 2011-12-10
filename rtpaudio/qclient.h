// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QClient - A graphical client for the RTP Audio Server            ####
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


#ifndef QCLIENT_H
#define QCLIENT_H


#include "audiowriterinterface.h"
#include "spectrumanalyzer.h"
#include "audiomixer.h"
#include "tools.h"
#include "strings.h"
#include "audioclient.h"
#include "strings.h"


#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qwhatsthis.h>
#include <qmainwindow.h>
#include <qlist.h>


#include "qspectrumanalyzer.h"
#include "qaudiomixer.h"
#include "qinfotabwidget.h"


/**
  * Transmission status info table #1 entries.
  */
const InfoEntry InfoEntries1[] = {
   {"SA",    "Server Address",   "This are the IPv4 or IPv6 address and port number of the audio server."},
   {"TF",    "TOS/Flow Label",   "This are the TOS/traffic class values for each layer and the flow label "
                                 "(IPv6 only) of the received packets."},
   {"SSSRC", "Server SSRC",      "This is the audio server's RTP SSRC. It is a 32-bit random number."},
   {"CA",    "Client Address",   "This is the IPv4 or IPv6 address and port number of the audio client."},
   {"CSSRC", "Client SSRC",      "This is the audio clients's RTP SSRC. It is a 32-bit random number."},
   {"BR",    "Bytes Received",   "This is a counter for the number of bytes received from server "
                                 "(IP/UDP/RTP/RTP Audio headers + payload)."},
   {"PR",    "Packets Received", "This is a counter for the number of packets received from server. "
                                 "The bytes per second value is the value for the quality received "
                                 "from server (IP/UDP/RTP/RTP Audio headers + payload)."},
   {"PL",    "Packets Lost",     "This is a counter for the number of packets lost during transmission."
                                 "The loss fraction shows the fraction of packets lost during the last "
                                 "RTCP report interval in each layer."},
   {"IJ", "Interarrival Jitter", "This is the interarrival jitter: An estimate of the statistical variance of "
                                 "the RTP data packet interarrival time, measured in milliseconds.\n\n"
                                 "Definition:\n"
                                 "Let Si, Sj be the RTP timestamps of packets i, j.\n"
                                 "Let Ri, Rj be the arrival timestamps.\n"
                                 "Dij := (Rj - Sj) - (Ri - Si).\n"
                                 "Jitter := Jitter + (1.0/16.0) * abs(Dij).\n\n"
                                 "See RFC 1889, Page 25-26 for more details."},
   {"Q", "Quality",  "This is the audio quality received from server: Sampling rate, bits and channels."},
   {"E", "Encoding", "This is the name of the audio encoding format received from server."},
};


/**
  * Transmission status info table #1.
  */
const InfoTable InfoTable1 =
{
   sizeof(InfoEntries1) / sizeof(InfoEntry),
   (const InfoEntry*)&InfoEntries1
};



/**
  * Transmission status info table #1 entries.
  */
const InfoEntry InfoEntries2[] = {
   {"LSA", "Source",           "This is the current layer's source address and port number."},
   {"LTF", "TOS/Flow Label",   "This are the current layer's traffic class and flow label (IPv6 only)."},
   {"CA",  "Destination",      "This is the current layer's destination address and port number."},
   {"LPR", "Packets Received", "This is the number of packets received in this layer."},
   {"LPL", "Packets Lost",     "This is the number of packets lost in this layer."},
   {"LFL", "Fraction Lost",    "This is the fraction of packets lost in this layer."},
   {"LBR", "Bytes Received",   "This is the sum of bytes received in this layer."},
   {"LIJ", "Interarrival Jitter", "This is the interarrival jitter of this layer."},
   {"Q", "Quality",  "This is the audio quality received from server: Sampling rate, bits and channels."},
   {"E", "Encoding", "This is the name of the audio encoding format received from server."},
};


/**
  * Transmission status info table #2.
  */
const InfoTable InfoTable2 =
{
   sizeof(InfoEntries2) / sizeof(InfoEntry),
   (const InfoEntry*)&InfoEntries2
};



/**
  * This class is the Qt-Toolkit GUI for the RTP audio client.
  *
  * @short   QClient
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class QClient : public QMainWindow
{
   Q_OBJECT

   // ====== Constructor/Destructor =========================================
   public:
   /**
     * Constructor for new QClient.
     *
     * @param audioOutput AudioWriter.
     * @param receiverName Receiver name (e.g. ipv6-gaffel:1234); default NULL.
     * @param defaultUTL Default URL (e.g. rtpa://gaffel:7500/Test.list); default NULL.
     * @param analyzer SpectrumAnalyzer object; default NULL.
     * @param mixer AudioMixer object; default NULL.
     * @param parent Parent QWidget; default NULL.
     */
   QClient(AudioWriterInterface* audioOutput,
           const char*           receiverName = NULL,
           const char*           defaultURL   = NULL,
           SpectrumAnalyzer*     analyzer     = NULL,
           AudioMixer*           mixer        = NULL,
           const bool            enableSCTP   = false,
           QWidget*              parent       = NULL);

   /**
     * Destructor.
     */
   ~QClient();


   // ====== Slots for Qt ===================================================
   public slots:
   /**
     * Slot for "Play" button.
     */
   void play();

   /**
     * Slot for "Stop" button.
     */
   void stop();

   /**
     * Slot for "Information" button.
     */
   void information();

   /**
     * Slot for What's This mode.
     */
   void whatsThis();

   /**
     * Slot for "Pause" button.
     */
   void pause(bool on);

   /**
     * Slot for "Toggle Pause" menu item.
     */
   void togglePause();

   /**
     * Slot for "Resolve Addresses" menu item.
     */
   void toggleResolver();

   /**
     * Slot for "Auto Repeat" menu item.
     */
   void toggleAutoRepeat();

   /**
     * Slot for "Auto Save Bookmarks" menu item.
     */
   void toggleAutoSaveBookmarks();

   /**
     * Slot for "Spectrum Analyzer" menu item.
     */
   void spectrumAnalyzer();

   /**
     * Slot for "Audio Mixer" menu item.
     */
   void audioMixer();

   /**
     * Slot for close button of QSpectrumAnalyzer.
     */
   void closeSpectrumAnalyzer();

   /**
     * Slot for close button of QAudioMixer.
     */
   void closeAudioMixer();

   /**
     * Slot for "Quit" menu item.
     */
   void quit();

   /**
     * Slot for position scrollbar.
     */
   void position(int value);

   /**
     * Slot for sampling rate combobox.
     */
   void setSamplingRate(int index);

   /**
     * Slot for "Stereo" checkbox.
     */
   void setChannels(bool stereo);

   /**
     * Slot for "protocol" combobox.
     */
   void setProtocol(int index);

   /**
     * Slot for "Bits" combobox.
     */
   void setBits(int index);

   /**
     * Slot for "Encoding" combobox.
     */
   void setEncoding(int index);

   /**
     * Slot for location menu item.
     */
   void locationSelected(int selection);

   /**
     * Slot for loading bookmarks.
     */
   void loadBookmarks();

   /**
     * Slot for removing all bookmarks.
     */
   void clearBookmarks();

   /**
     * Slot for saving bookmarks.
     */
   void saveBookmarks();

   /**
     * Slot for QTimer.
     */
   void timerEvent();


   // ====== Status update ==================================================
   private:
   void updateCounter(card64 position);
   void insertURL(const String& urlToInsert);


   // ====== Private data ===================================================
   private:
   void showError(const cardinal error);
   QString bytesToQString(const card64 bytes) const;
   QString card64ToQString(const card64 value,
                           const char*  formatString = "%Ld") const;
   QString doubleToQString(const double value,
                           const char*  formatString = "%f") const;
   QString flowInfoToQString(const card8  trafficClass,
                             const card32 flowLabel) const;

   static const cardinal LocationCount           = 15;
   static const cardinal DisplayUpdateInterval   = 250;
   static const cardinal EOFRepeatInterval       = 20000 / DisplayUpdateInterval;
   static const cardinal MaxScrollBarUpdateDelay = 4;
   static const cardinal MaxLayerInfo            = 3;

   QAction*              ResolverAction;
   QAction*              MixerAction;
   QAction*              SpectrumAnalyzerAction;
   QAction*              AutoRepeatAction;
   QAction*              AutoSaveBookmarksAction;

   QAction*              LocationAction[LocationCount];

   QSpectrumAnalyzer*    SpectrumAnalyzerWindow;
   QAudioMixer*          MixerWindow;
   SpectrumAnalyzer*     SpectrumAnalyzerDevice;
   AudioMixer*           MixerDevice;

   QMenu*                ToolsMenu;
   QMenu*                URLMenu;
   QMenu*                SettingsMenu;
   QLineEdit*            Location;
   QLCDNumber*           Counter;
   QLabel*               TitleLabel;
   QLabel*               ArtistLabel;
   QLabel*               CommentLabel;
   QLabel*               StatusBar;
   QInfoTabWidget*       InfoWidget;
   QInfoWidget*          LayerInfo[MaxLayerInfo];
   QScrollBar*           ScrollBar;
   bool                  ScrollBarUpdated;
   cardinal              ScrollBarUpdateDelay;
   cardinal              EOFRepeatDelay;
   QPushButton*          Pause;
   QWhatsThis*           WhatsThis;

   QList<String*>        URLList;
   String                PlayingURL;
   bool                  InsertionRequired;
   bool                  ResolveMode;
   bool                  AutoRepeat;
   bool                  AutoSaveBookmarks;
   bool                  UseSCTP;

   AudioClient*          Client;
};


#endif
