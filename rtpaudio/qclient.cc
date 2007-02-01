// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QClient - A graphical client for the RTP Audio Server            ####
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


#include <qapp.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qplatinumstyle.h>
#include <qmenubar.h>
#include <qpopupmenu.h>

#include <fstream>


#include "tdsystem.h"
#include "audiodevice.h"
#include "audiodebug.h"
#include "audionull.h"
#include "spectrumanalyzer.h"
#include "multiaudiowriter.h"
#include "tdsocket.h"
#include "tools.h"
#include "audioclient.h"
#include "trafficclassvalues.h"

#include "qclient.h"


// ###### Constructor #######################################################
QClient::QClient(AudioWriterInterface* audioOutput,
                 const char*           receiverName,
                 const char*           defaultURL,
                 SpectrumAnalyzer*     analyzer,
                 AudioMixer*           mixer,
                 const bool            enableSCTP,
                 QWidget*              parent,
                 const char*           name)
   : QMainWindow(parent,name)
{
   // ====== Create AudioClient =============================================
   Client                 = new AudioClient(NULL,0,audioOutput);
   CHECK_PTR(Client);
   SpectrumAnalyzerWindow = NULL;
   SpectrumAnalyzerDevice = analyzer;
   MixerWindow            = NULL;
   MixerDevice            = mixer;
   InsertionRequired      = false;
   ResolveMode            = false;
   AutoSaveBookmarks      = false;
   AutoRepeat             = true;
   UseSCTP                = false;

   // ====== Main Layout ====================================================
   WhatsThis = new QWhatsThis(this);
   CHECK_PTR(WhatsThis);
   QWidget* centralWidget = new QWidget(this);
   CHECK_PTR(centralWidget);
   WhatsThis->add(centralWidget,"This is the RTP Audio Client!");
   QGridLayout* topLayout = new QGridLayout(centralWidget,3,2,5);
   CHECK_PTR(topLayout);
   topLayout->setColStretch(0,0);
   topLayout->setColStretch(1,10);
   topLayout->setRowStretch(0,10);
   topLayout->setRowStretch(1,0);

   // ====== Menu ===========================================================
   QMenuBar* menu = menuBar();
   QPopupMenu* fileMenu = new QPopupMenu(this);
   CHECK_PTR(fileMenu);
   fileMenu->insertItem("&Load Bookmarks",this,SLOT(loadBookmarks()),CTRL+Key_M);
   fileMenu->insertItem("&Save Bookmarks",this,SLOT(saveBookmarks()),CTRL+Key_O);
   fileMenu->insertSeparator();
   fileMenu->insertItem("&Quit",this,SLOT(quit()),CTRL+Key_Q);

   QPopupMenu* controlMenu = new QPopupMenu(this);
   CHECK_PTR(controlMenu);
   controlMenu->insertItem("&Play",this,SLOT(play()),CTRL+Key_P);
   controlMenu->insertItem("&Stop",this,SLOT(stop()),CTRL+Key_S);
   controlMenu->insertSeparator();
   controlMenu->insertItem("&Toggle Pause",this,SLOT(togglePause()),CTRL+Key_U);

   URLMenu = new QPopupMenu(this);
   CHECK_PTR(URLMenu);
   for(cardinal i = 0;i < LocationCount;i++) {
      URLMenu->insertItem("()",MenuIDLocation + i);
      if(i <= 9) {
         URLMenu->setAccel(CTRL+Key_0+i,MenuIDLocation + i);
      }
      URLMenu->setItemEnabled(MenuIDLocation + i,false);
      if(i == 0) {
         URLMenu->insertSeparator();
      }
   }
   URLMenu->insertSeparator();
   URLMenu->insertItem("&Clear Bookmarks",this,SLOT(clearBookmarks()),CTRL+Key_K);
   QObject::connect(URLMenu,SIGNAL(activated(int)),this,SLOT(locationSelected(int)));

   ToolsMenu = new QPopupMenu(this);
   CHECK_PTR(ToolsMenu);
   if(SpectrumAnalyzerDevice != NULL) {
      ToolsMenu->insertItem("Spectrum Analyzer",this,SLOT(spectrumAnalyzer()),CTRL+Key_A,MenuIDSpectrumAnalyzer);
      ToolsMenu->setItemChecked(MenuIDSpectrumAnalyzer,false);
      ToolsMenu->setWhatsThis(MenuIDSpectrumAnalyzer,"Check this item to show the spectrum analyzer.");
   }
   if(MixerDevice != NULL) {
      ToolsMenu->insertItem("Audio Mixer",this,SLOT(audioMixer()),CTRL+Key_M,MenuIDMixer);
      ToolsMenu->setItemChecked(MenuIDMixer,false);
      ToolsMenu->setWhatsThis(MenuIDMixer,"Check this item to show the audio mixer.");
   }

   SettingsMenu = new QPopupMenu(this);
   CHECK_PTR(SettingsMenu);
   SettingsMenu->insertItem("&Resolve Addresses",this,SLOT(toggleResolver()),CTRL+Key_R,MenuIDResolver);
   SettingsMenu->setItemChecked(MenuIDResolver,ResolveMode);
   SettingsMenu->insertItem("Auto Repeat",this,SLOT(toggleAutoRepeat()),CTRL+Key_Y,MenuIDAutoRepeat);
   SettingsMenu->setItemChecked(MenuIDAutoRepeat,AutoRepeat);
   SettingsMenu->insertItem("Auto Save Bookmarks",this,SLOT(toggleAutoSaveBookmarks()),CTRL+Key_L,MenuIDAutoSaveBookmarks);
   SettingsMenu->setItemChecked(MenuIDAutoSaveBookmarks,AutoSaveBookmarks);

   QPopupMenu* helpMenu = new QPopupMenu(this);
   CHECK_PTR(helpMenu);
   helpMenu->insertItem("&About",this,SLOT(information()),CTRL+Key_T);
   helpMenu->insertItem("&What's This?",this,SLOT(whatsThis()),Key_F12);

   menu->insertItem("&File",fileMenu);
   menu->insertItem("&Control",controlMenu);
   menu->insertItem("&Bookmarks",URLMenu);
   menu->insertItem("&Tools",ToolsMenu);
   menu->insertItem("&Settings",SettingsMenu);
   menu->insertSeparator();
   menu->insertItem("&Help",helpMenu);

   // ====== Status line ====================================================
   QLabel* copyright = new QLabel("Copyright (C) 1999-2001 Thomas Dreibholz",centralWidget);
   CHECK_PTR(copyright);
   WhatsThis->add(copyright,"RTP Audio Client\nCopyright (C) 1999-2001 Thomas Dreibholz");
   copyright->setAlignment(AlignRight);
   StatusBar = new QLabel("Welcome to the RTP Audio Client!",centralWidget);
   CHECK_PTR(StatusBar);
   WhatsThis->add(StatusBar,"The status of a connection will be shown here.");
   StatusBar->setMinimumSize(StatusBar->sizeHint());
   copyright->setMinimumSize(copyright->sizeHint());
   topLayout->addWidget(StatusBar,2,0);
   topLayout->addWidget(copyright,2,1);

   // ====== Quality ========================================================
   QButtonGroup* qualityGroup = new QButtonGroup("Quality",centralWidget);
   CHECK_PTR(qualityGroup);
   WhatsThis->add(qualityGroup,"This group contains functions to adapt the audio quality and change the encoding.");
   topLayout->addWidget(qualityGroup,1,1);

   QVBoxLayout*  qualityLayout = new QVBoxLayout(qualityGroup,20);
   CHECK_PTR(qualityLayout);
   QHBoxLayout*  checkLayout   = new QHBoxLayout();
   CHECK_PTR(checkLayout);
   QCheckBox*    stereo        = new QCheckBox("Stereo",qualityGroup);
   CHECK_PTR(stereo);
   stereo->setAccel((int)ALT + (int)'1');
   QComboBox*    bits = new QComboBox(false,qualityGroup);
   CHECK_PTR(bits);
   QLabel* ipv6 = NULL;
   if(!enableSCTP) {
      ipv6 = new QLabel(InternetAddress::hasIPv6() ? "Using IPv6" : "Using IPv4",qualityGroup);
      CHECK_PTR(ipv6);
   }
   QComboBox* protocol = NULL;
   if(enableSCTP) {
      protocol = new QComboBox(false,qualityGroup);
      CHECK_PTR(protocol);
      if(InternetAddress::hasIPv6()) {
         protocol->insertItem("UDP / IPv6",0);
         protocol->insertItem("SCTP / IPv6",1);
      }
      else {
         protocol->insertItem("UDP / IPv4",0);
         protocol->insertItem("SCTP / IPv4",1);
      }
   }
   QComboBox* rate    = new QComboBox(false,qualityGroup);
   CHECK_PTR(rate);
   QComboBox* encoder = new QComboBox(false,qualityGroup);
   CHECK_PTR(encoder);

   for(integer i = AudioQuality::ValidBits - 1;i >= 0;i--) {
      char str[64];
      snprintf((char*)&str,sizeof(str),"%d Bits",AudioQuality::ValidBitsTable[i]);
      bits->insertItem((char*)&str,AudioQuality::ValidBits - i - 1);
   }
   for(integer i = AudioQuality::ValidRates - 1;i >= 0;i--) {
      char str[64];
      snprintf((char*)&str,sizeof(str),"%d Hz",AudioQuality::ValidRatesTable[i]);
      rate->insertItem((char*)&str,AudioQuality::ValidRates - i - 1);
   }
   for(cardinal index = 0;;index++) {
      const char* encoding = Client->getEncodingName(index);
      if(encoding != NULL)
         encoder->insertItem(encoding,index);
      else
         break;
   }
   WhatsThis->add(stereo,"Check this box for stereo audio quality instead of mono.");
   WhatsThis->add(bits,"You can select the number of audio bits here.");
   WhatsThis->add(rate,"You can select the audio sampling rate here.");
   WhatsThis->add(encoder,"You can select an encoding for the audio transport here.");
   if(ipv6 != NULL) {
      WhatsThis->add(ipv6,"This label shows whether your system supports IPv6 or not.");
   }
   if(protocol != NULL) {
      WhatsThis->add(protocol,"You can select the transport protocol here: UDP or SCTP.");
   }

   stereo->setMinimumSize(stereo->sizeHint());
   stereo->setChecked(TRUE);
   rate->setMinimumSize(rate->sizeHint());
   encoder->setMinimumSize(encoder->sizeHint());

   checkLayout->addWidget(bits);
   checkLayout->addWidget(stereo);
   if(ipv6 != NULL) {
      checkLayout->addWidget(ipv6);
   }
   if(protocol != NULL) {
      checkLayout->addWidget(protocol);
   }
   qualityLayout->addLayout(checkLayout);
   qualityLayout->addWidget(rate);
   qualityLayout->addWidget(encoder);


   // ====== Transmission Status ============================================
   InfoWidget = new QInfoTabWidget(&InfoTable1,"Connection","pinguin.xpm",centralWidget);
   CHECK_PTR(InfoWidget);
   char str[128];
   for(cardinal i = 0;i < MaxLayerInfo;i++) {
      snprintf((char*)&str,sizeof(str),"L #%d",i);
      LayerInfo[i] = InfoWidget->addTable(&InfoTable2,(char*)&str,"info.xpm");
   }
   topLayout->addWidget(InfoWidget,0,1);


   // ====== Main Display ===================================================
   QGroupBox* displayGroup = new QGroupBox("Media Information",centralWidget);
   CHECK_PTR(displayGroup);
   WhatsThis->add(displayGroup,"Information on the current media playing are shown in this group.");
   topLayout->addWidget(displayGroup,0,0);

   QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup,20);
   CHECK_PTR(displayLayout);
   Counter = new QLCDNumber(10,displayGroup);
   CHECK_PTR(Counter);
   Counter->setMinimumSize(180,80);
   displayLayout->addWidget(Counter);
   updateCounter(0);

   QGroupBox* mediaInfoGroup = new QGroupBox(displayGroup);
   CHECK_PTR(mediaInfoGroup);
   QGridLayout* mediaInfoLayout = new QGridLayout(mediaInfoGroup,3,2,5);
   CHECK_PTR(mediaInfoLayout);
   mediaInfoLayout->setColStretch(1,10);

   TitleLabel = new QLabel("N/A",mediaInfoGroup);
   CHECK_PTR(TitleLabel);
   TitleLabel->setMinimumSize(TitleLabel->sizeHint());
   ArtistLabel = new QLabel("N/A",mediaInfoGroup);
   CHECK_PTR(ArtistLabel);
   ArtistLabel->setMinimumSize(ArtistLabel->sizeHint());
   CommentLabel = new QLabel("N/A",mediaInfoGroup);
   CHECK_PTR(CommentLabel);
   CommentLabel->setMinimumSize(CommentLabel->sizeHint());

   QLabel* titleLabel = new QLabel("Title:",mediaInfoGroup);
   CHECK_PTR(titleLabel);
   mediaInfoLayout->addWidget(titleLabel,0,0);
   QLabel* artistLabel = new QLabel("Artist:",mediaInfoGroup);
   CHECK_PTR(artistLabel);
   mediaInfoLayout->addWidget(artistLabel,1,0);
   QLabel* commentLabel = new QLabel("Comment:",mediaInfoGroup);
   CHECK_PTR(commentLabel);
   mediaInfoLayout->addWidget(commentLabel,2,0);

   mediaInfoLayout->addWidget(TitleLabel,0,1);
   mediaInfoLayout->addWidget(ArtistLabel,1,1);
   mediaInfoLayout->addWidget(CommentLabel,2,1);
   displayLayout->addWidget(mediaInfoGroup);
   WhatsThis->add(Counter,"This counter shows the position within the audio file in minutes/seconds/0.01 seconds format.");
   WhatsThis->add(TitleLabel,"This is the title of the file playing");
   WhatsThis->add(ArtistLabel,"This is the artist or the file playing");
   WhatsThis->add(CommentLabel,"This is a comment on the file playing");


   // ====== Control ========================================================
   QGroupBox* controlGroup = new QGroupBox("Server Control",centralWidget);
   CHECK_PTR(controlGroup);
   WhatsThis->add(controlGroup,"This group contains server control functions.");
   topLayout->addWidget(controlGroup,1,0);

   QGridLayout* controlLayout = new QGridLayout(controlGroup,4,4,20);
   CHECK_PTR(controlLayout);
   QPushButton* whatsThis     = new QPushButton("H&elp",controlGroup);
   CHECK_PTR(whatsThis);
   QPushButton* play          = new QPushButton("&Play",controlGroup);
   CHECK_PTR(play);
   QPushButton* stop          = new QPushButton("St&op",controlGroup);
   CHECK_PTR(stop);
   Pause                      = new QPushButton("P&ause",controlGroup);
   CHECK_PTR(Pause);

   bits->setMinimumHeight(stop->height());
   if(protocol != NULL) {
      protocol->setMinimumHeight(stop->height());
   }
   rate->setMinimumHeight(stop->height());
   encoder->setMinimumHeight(stop->height());

   WhatsThis->add(whatsThis,"Click here to enter What's This mode.");
   WhatsThis->add(play,"Click here to start playing.");
   WhatsThis->add(stop,"Click here to stop playing.");
   WhatsThis->add(Pause,"Click here to pause playing.");
   ScrollBar = new QScrollBar(QScrollBar::Horizontal,controlGroup);
   CHECK_PTR(ScrollBar);
   WhatsThis->add(ScrollBar,"Move this scrollbar to change the current position within a playing audio file.");
   ScrollBarUpdated     = true;
   ScrollBarUpdateDelay = 0;
   ScrollBar->setRange(0,0);
   ScrollBarUpdated = true;
   ScrollBar->setValue(0);

   QLabel* label = new QLabel("Source URL: (Example: rtpa://gaffel:7500/Test1.list)",controlGroup);
   CHECK_PTR(label);
   Location      = new QLineEdit(controlGroup);
   CHECK_PTR(Location);
   QObject::connect(Location,SIGNAL(returnPressed()),this,SLOT(play()));
   WhatsThis->add(label,"This is an example for a location.");
   WhatsThis->add(Location,"Enter the location of the audio list to play here.\nExample: rtpa://gaffel:7500/Test1.list");

   play->setMinimumSize(play->sizeHint());
   whatsThis->setMinimumSize(whatsThis->sizeHint());
   stop->setMinimumSize(stop->sizeHint());
   Pause->setMinimumSize(Pause->sizeHint());
   Pause->setToggleButton(TRUE);
   ScrollBar->setMinimumSize(ScrollBar->sizeHint());
   Location->setMinimumSize(Location->sizeHint());
   loadBookmarks();
   if(defaultURL != NULL)
      Location->setText(defaultURL);
   else {
      loadBookmarks();
      const String* url = URLList.first();
      if(url != NULL) {
          Location->setText(url->getData());
      }
      else {
         if(Client->getIPVersion() == 6)
            Location->setText("rtpa://ipv6-localhost:7500/Test.list");
         else
            Location->setText("rtpa://localhost:7500/Test.list");
      }
   }
   label->setMinimumSize(label->sizeHint());

   controlLayout->addWidget(play,0,0);
   controlLayout->addWidget(stop,0,1);
   controlLayout->addWidget(Pause,0,2);
   controlLayout->addWidget(whatsThis,0,3);
   controlLayout->addMultiCellWidget(ScrollBar,1,1,0,3);
   controlLayout->addMultiCellWidget(label,2,2,0,3);
   controlLayout->addMultiCellWidget(Location,3,3,0,3);


   // ====== Connect widgets to methods =====================================
   QObject::connect(whatsThis,SIGNAL(clicked()),this,SLOT(whatsThis()));
   QObject::connect(play,SIGNAL(clicked()),this,SLOT(play()));
   QObject::connect(stop,SIGNAL(clicked()),this,SLOT(stop()));
   QObject::connect(Pause,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
   QObject::connect(ScrollBar,SIGNAL(valueChanged(int)),this,SLOT(position(int)));
   QObject::connect(rate,SIGNAL(activated(int)),this,SLOT(setSamplingRate(int)));
   QObject::connect(stereo,SIGNAL(toggled(bool)),this,SLOT(setChannels(bool)));
   QObject::connect(bits,SIGNAL(activated(int)),this,SLOT(setBits(int)));
   if(protocol != NULL) {
      QObject::connect(protocol,SIGNAL(activated(int)),this,SLOT(setProtocol(int)));
   }
   QObject::connect(encoder,SIGNAL(activated(int)),this,SLOT(setEncoding(int)));


   // ====== Create new QTimer ==============================================
   QTimer* timer = new QTimer(this);
   CHECK_PTR(timer);
   timer->QObject::connect(timer,SIGNAL(timeout()),this,SLOT(timerEvent()));
   timer->start(250);

   setCentralWidget(centralWidget);
   setCaption("RTP Audio Client");
}


// ###### Destructor ########################################################
QClient::~QClient()
{
   if(SpectrumAnalyzerWindow) {
      delete SpectrumAnalyzerWindow;
      SpectrumAnalyzerWindow = NULL;
   }
   if(MixerWindow) {
      delete MixerWindow;
      MixerWindow = NULL;
   }
   clearBookmarks();
   delete Client;
   Client = NULL;
}


// ###### Show information window ###########################################
void QClient::information()
{
   QMessageBox::information(this,
      "RTP Audio Information",
      "RTP Audio Client - Version 1.50\n\n"
      "Copyright (C) 1999-2007\n"
      "Thomas Dreibholz\n"
      "dreibh@exp-math.uni-essen.de",
      "Okay");
}


// ###### Show encoder error window #########################################
void QClient::showError(const cardinal error)
{
   char  str[256];
   char  strDefault[64];
   char* errorString;
   switch(error) {
      case ME_BadMedia:
         errorString = "Media not found or invalid file format!";
       break;
      case ME_ReadError:
         errorString = "Error during read!";
       break;
      case ME_OutOfMemory:
         errorString = "Out of memory!";
        break;
      default:
         snprintf((char*)&strDefault,sizeof(strDefault),"Error #%d!",(int)error);
         errorString = (char*)&strDefault;
         break;
    }
   strcpy((char*)&str,"Connection terminated due to encoder error:\n");
   strcat((char*)&str,errorString);
   QMessageBox::warning(this,"RTP Audio Encoder Error",(char*)&str,"Okay!");
}


// ###### Enter What's This mode ############################################
void QClient::whatsThis()
{
   WhatsThis->enterWhatsThisMode();
}


// ###### Start playing #####################################################
void QClient::play()
{
   // ====== Get server address and media name ==============================
   String protocol;
   String host;
   String path;
   bool newOK = scanURL((const char*)Location->text(),protocol,host,path);
   if((newOK == false) || (protocol != "rtpa")) {
      StatusBar->setText("Invalid URL! Check URL and try again.");
      QMessageBox::warning(this,"Warning",
                           "This URL is invalid!\n"
                           "Check URL and try again.\n"
                           "Example: rtpa://odin:7500/Test.list",
                           "Okay!");
      return;
   }

   // ====== Check, if URL is new => Restart or change media ================
   if(Client->playing() == true) {
      const char* oldURL = PlayingURL.getData();
      const char* newURL = Location->text();
      if(!(strcmp(oldURL,newURL))) {
         Pause->setOn(false);
         return;
      }

      String oldProtocol;
      String oldHost;
      String oldPath;
      bool oldOK = scanURL(PlayingURL,oldProtocol,oldHost,oldPath);
      if(oldOK == true) {
         if((oldHost == host) && (oldProtocol == protocol)) {
            Client->change(path.getData());
            Pause->setOn(FALSE);
            PlayingURL = (const char*)Location->text();
            InsertionRequired = true;
            return;
         }
      }
      stop();
   }

   // ====== Start client ===================================================
   Pause->setOn(FALSE);
   PlayingURL = (const char*)Location->text();
   bool ok = Client->play(host.getData(),path.getData(),UseSCTP);

   // ====== Update status display ==========================================
   if(ok) {
      EOFRepeatDelay = 0;
      StatusBar->setText("Waiting for data from server...");
      InternetAddress::PrintFormat format = InternetAddress::PF_Address;
      if(ResolveMode) {
         format = InternetAddress::PF_Hostname;
      }
      const String serverAddress = Client->getServerAddressString(format);
      const String ourAddress    = Client->getOurAddressString(format);
      InfoWidget->update("SA",serverAddress.getData());
      InfoWidget->update("CA",ourAddress.getData());
      InfoWidget->update("CSSRC",card64ToQString(Client->getOurSSRC(),"$%08x"));
      InsertionRequired = true;
   }
   else {
      StatusBar->setText("Unable to find server! Check parameters and try again.");
      QMessageBox::warning(this,"Warning",
                           "Unable to find server!\n"
                           "Check parameters and try again.\n"
                           "Example: rtpa://ipv6-odin:7500/CD1.list",
                           "Okay!");
   }
}


// ###### Stop playing ######################################################
void QClient::stop()
{
   InsertionRequired = false;
   Client->stop();
   if(SpectrumAnalyzerWindow != NULL) {
      SpectrumAnalyzerWindow->reset();
   }
   StatusBar->setText("Ready!");
   TitleLabel->setText("N/A");
   ArtistLabel->setText("N/A");
   CommentLabel->setText("N/A");
   ScrollBar->setRange(0,0);
   ScrollBar->setValue(0);
   updateCounter(0);
}


// ###### Begin/end pause mode ##############################################
void QClient::pause(bool on)
{
   Client->setPause(on);
}


// ###### Toggle pause mode #################################################
void QClient::togglePause()
{
   Pause->toggle();
}


// ###### Toggle address resolve mode #######################################
void QClient::toggleResolver()
{
   ResolveMode = !ResolveMode;
   SettingsMenu->setItemChecked(MenuIDResolver,ResolveMode);

   if(Client->playing()) {
      InternetAddress::PrintFormat format = InternetAddress::PF_Address;
      if(ResolveMode) {
         format = InternetAddress::PF_Hostname;
      }
      const String serverAddress = Client->getServerAddressString(format);
      const String ourAddress    = Client->getOurAddressString(format);
      InfoWidget->update("SA",serverAddress.getData());
      InfoWidget->update("CA",ourAddress.getData());
   }
}


// ###### Toggle auto repeat mode ###########################################
void QClient::toggleAutoRepeat()
{
   AutoRepeat = !AutoRepeat;
   SettingsMenu->setItemChecked(MenuIDAutoRepeat,AutoRepeat);
}


// ###### Toggle auto save bookmarks mode ###################################
void QClient::toggleAutoSaveBookmarks()
{
   AutoSaveBookmarks = !AutoSaveBookmarks;
   SettingsMenu->setItemChecked(MenuIDAutoSaveBookmarks,AutoSaveBookmarks);
}


// ###### Show spectrum analyzer ############################################
void QClient::spectrumAnalyzer()
{
   if(SpectrumAnalyzerDevice != NULL) {
      if(SpectrumAnalyzerWindow == NULL) {
         SpectrumAnalyzerWindow = new QSpectrumAnalyzer(SpectrumAnalyzerDevice);
         if(SpectrumAnalyzerWindow != NULL) {
            QObject::connect(SpectrumAnalyzerWindow,SIGNAL(closeSpectrumAnalyzer()),this,SLOT(closeSpectrumAnalyzer()));
            SpectrumAnalyzerWindow->show();
            ToolsMenu->setItemChecked(MenuIDSpectrumAnalyzer,true);
         }
      }
      else {
         delete SpectrumAnalyzerWindow;
         SpectrumAnalyzerWindow = NULL;
         ToolsMenu->setItemChecked(MenuIDSpectrumAnalyzer,false);
      }
   }
}


// ###### Close spectrum analyzer ############################################
void QClient::closeSpectrumAnalyzer()
{
   if(SpectrumAnalyzerWindow != NULL) {
      delete SpectrumAnalyzerWindow;
      SpectrumAnalyzerWindow = NULL;
      ToolsMenu->setItemChecked(MenuIDSpectrumAnalyzer,false);
   }
}


// ###### Show audio mixer ##################################################
void QClient::audioMixer()
{
   if(MixerDevice != NULL) {
      if(MixerWindow == NULL) {
         MixerWindow = new QAudioMixer(MixerDevice);
         if(MixerWindow != NULL) {
            QObject::connect(MixerWindow,SIGNAL(closeAudioMixer()),this,SLOT(closeAudioMixer()));
            MixerWindow->show();
            ToolsMenu->setItemChecked(MenuIDMixer,true);
         }
      }
      else {
         delete MixerWindow;
         MixerWindow = NULL;
         ToolsMenu->setItemChecked(MenuIDMixer,false);
      }
   }
}


// ###### Close audio mixer #################################################
void QClient::closeAudioMixer()
{
   if(MixerWindow != NULL) {
      delete MixerWindow;
      MixerWindow = NULL;
      ToolsMenu->setItemChecked(MenuIDMixer,false);
   }
}


// ###### Quit QClient ######################################################
void QClient::quit()
{
   stop();
   if(AutoSaveBookmarks) {
      saveBookmarks();
   }
   exit(0);
}


// ###### Update counter ####################################################
void QClient::updateCounter(card64 position)
{
   // ====== Update frame counter ===========================================
   char str[32];
   snprintf((char*)&str,sizeof(str),"%08Ld",position / (PositionStepsPerSecond / 1000));

   // ====== Update time counter ============================================
   const card64 seconds = position / PositionStepsPerSecond;
   snprintf((char*)&str,sizeof(str),
            "%02Ld:%02Ld.%02Ld",
            (seconds / 60),
            (seconds % 60),
            ((position % PositionStepsPerSecond) / (PositionStepsPerSecond / 100)));
   Counter->display((char*)&str);
}


// ###### Set new position ##################################################
void QClient::position(int value)
{
   if(!ScrollBarUpdated) {
      const card64 position = (card64)value * (PositionStepsPerSecond / 10);
      Client->setPosition(position);
      Pause->setOn(FALSE);
      ScrollBarUpdateDelay = MaxScrollBarUpdateDelay;
   }
   else {
      ScrollBarUpdated = false;
   }
}


// ###### Set sampling rate #################################################
void QClient::setSamplingRate(int index)
{
   if((index >= 0) && (index < (int)AudioQuality::ValidRates)) {
      Client->setSamplingRate(AudioQuality::ValidRatesTable[
         AudioQuality::ValidRates - index - 1]);
   }
}


// ###### Set number of channels ############################################
void QClient::setChannels(bool stereo)
{
   if(stereo)
      Client->setChannels(2);
   else
      Client->setChannels(1);
}


// ###### Set number of bits ################################################
void QClient::setBits(int index)
{
   Client->setBits(AudioQuality::ValidBitsTable[AudioQuality::ValidBits - index - 1]);
}


// ###### Set protocol ######################################################
void QClient::setProtocol(int index)
{
   UseSCTP = (index == 1);
}


// ###### Set encoder #######################################################
void QClient::setEncoding(int index)
{
   Client->setEncoding(index);
}


// ###### TimedThread's timerEvent() implementation #########################
void QClient::timerEvent()
{
   char str[128];

   if(Client->playing()) {
      // ====== Check for errors ============================================
      const card8 error = Client->getErrorCode();
      if(error >= ME_UnrecoverableError) {
         stop();
         showError(error);
         return;
      }

      // ====== Insert URL into list ========================================
      if(InsertionRequired == true) {
         InsertionRequired = false;
         insertURL(PlayingURL);
      }


      // ====== Display source state info ===================================
      InfoWidget->update("SSSRC",card64ToQString(Client->getServerSSRC(),"$%08x"));

      const cardinal layers = Client->getLayers();
      QString flString;
      QString jString;
      card64  totalLost = 0;

      for(cardinal i = 0;i < min(layers,(cardinal)3);i++) {
         if(i > 0) {
            flString += " / ";
            jString  += " / ";
         }

         // ====== Update fraction lost =====================================
         const double fractionLost        = Client->getFractionLost(i) * 100.0;
         const QString fractionLostString = doubleToQString(fractionLost,"%1.1f%%");
         flString += fractionLostString;
         if(i < MaxLayerInfo) {
            LayerInfo[i]->update("LFL",fractionLostString);
         }

         // ====== Update packets lost ======================================
         const card64 packetsLost = Client->getPacketsLost(i);
         totalLost += packetsLost;
         if(i < MaxLayerInfo) {
            LayerInfo[i]->update("LPL",card64ToQString(packetsLost));
         }

         // ====== Update jitter ============================================
         const double jitter        = Client->getJitter(i) / 1000.0;
         const QString jitterString = doubleToQString(jitter,"%1.2f");
         jString += jitterString;
         if(i < MaxLayerInfo) {
            LayerInfo[i]->update("LIJ",jitterString + " [ms]");
         }
      }
      jString += " [ms]";
      InfoWidget->update("IJ",jString);
      flString = card64ToQString(totalLost,"%Ld ") + flString;
      InfoWidget->update("PL",flString);


      // ====== Display receiver statistics =================================
      InfoWidget->update("PR",card64ToQString(Client->getPacketsReceived((cardinal)-1)) +
                              doubleToQString(Client->getRawBytesPerSecond() / 1024.0," (%1.2f KB/s)"));
      InfoWidget->update("BR",bytesToQString(Client->getBytesReceived((cardinal)-1)));


      // ====== Display layer statistics ====================================
      for(cardinal i = 0;i < MaxLayerInfo;i++) {
         InternetFlow flow = Client->getInternetFlow(i);
         if(!ResolveMode) {
            flow.setPrintFormat(InternetAddress::PF_Address);
         }
         else {
            flow.setPrintFormat(InternetAddress::PF_Hostname);
         }
         LayerInfo[i]->update("LSA",((InternetAddress)flow).getAddressString().getData());
         LayerInfo[i]->update("LTF",flowInfoToQString(flow.getTrafficClass(),flow.getFlowLabel()));
         LayerInfo[i]->update("LBR",bytesToQString(Client->getBytesReceived(i)));
         LayerInfo[i]->update("LPR",card64ToQString(Client->getPacketsReceived(i)));
         LayerInfo[i]->update("LFL",doubleToQString(100.0 * Client->getFractionLost(i),"%1.2f %%"));
      }


      if(error == ME_EOF) {
         // ====== End of media -> Auto-repeat ==============================
         if(!Pause->isOn()) {
            if(AutoRepeat) {
               EOFRepeatDelay++;
               snprintf((char*)&str,sizeof(str),
                        "End of media reached! Auto-restart in %d seconds...",
                        (int)((EOFRepeatInterval - EOFRepeatDelay) * DisplayUpdateInterval) / 1000);
               StatusBar->setText((char*)&str);
               if(EOFRepeatDelay >= EOFRepeatInterval) {
                  EOFRepeatDelay = 0;
                  ScrollBarUpdated = false;
                  position(0);
               }
            }
            else {
               StatusBar->setText("End of media reached!");
            }
         }
         else {
            StatusBar->setText("End of media reached!");
         }
      }
      else if(Client->getBytesReceived() > 0) {
         EOFRepeatDelay = 0;

         // ====== Print traffic classes and layer #0 flow label ============
         QString tflString;
         for(cardinal i = 0;i < layers;i++) {
            tflString += flowInfoToQString(Client->getTrafficClass(i),0);
            if(i < layers - 1) {
               tflString += ":";
            }
         }
         card32 flowLabel = Client->getFlowLabel();
         if(flowLabel != 0) {
            tflString += card64ToQString(flowLabel, " / $%05Lx");
         }
         InfoWidget->update("TF",tflString);
         StatusBar->setText("Receiving data...");
      }


      // ====== Display quality and decoder name ============================
      snprintf((char*)&str,sizeof(str),"%s",Client->getEncoding());
      InfoWidget->update("E",(char*)&str);
      snprintf((char*)&str,sizeof(str),
               "%d Hz / %d Bit / %s",
               Client->getSamplingRate(),
               Client->getBits(),
               Client->getChannels() == 2 ? "Stereo" : "Mono");
      InfoWidget->update("Q",(char*)&str);


      // ====== Display MediaInfo values ====================================
      const MediaInfo mediaInfo = Client->getMediaInfo();
      TitleLabel->setText(QString(mediaInfo.Title).stripWhiteSpace());
      ArtistLabel->setText(QString(mediaInfo.Artist).stripWhiteSpace());
      CommentLabel->setText(QString(mediaInfo.Comment).stripWhiteSpace());


      // ====== Update counter ==============================================
      updateCounter(Client->getPosition());


      // ====== Update scrollbar's range ====================================
      if(!ScrollBar->draggingSlider()) {
         if(ScrollBarUpdateDelay > 0)
            ScrollBarUpdateDelay--;
         if(ScrollBarUpdateDelay == 0) {
            ScrollBarUpdated = true;
            ScrollBar->setRange(0,Client->getMaxPosition() / (PositionStepsPerSecond / 10));
            ScrollBarUpdated = true;
            ScrollBar->setValue(Client->getPosition() / (PositionStepsPerSecond / 10));
         }
      }
   }
   else {
      InfoWidget->clear();
   }
}


// ###### Location selected slot ############################################
void QClient::locationSelected(int selection)
{
   if((selection >= (int)MenuIDLocation) && (selection < (int)(MenuIDLocation + LocationCount))) {
      cardinal number = selection - MenuIDLocation;
      String* url = URLList.first();
      while(url != NULL) {
         if(number == 0) {
            const char* oldURL = Location->text();
            const char* newURL = url->getData();
            if((strcmp(oldURL,newURL)) || (!Client->playing())) {
               Location->setText(newURL);
               play();
            }
            break;
         }
         number--;
         url = URLList.next();
      }
   }
}

// ###### Insert current URL into URL list ##################################
void QClient::insertURL(const String& urlToInsert)
{
   // ====== Remove old copy of new URL and insert URL at head of list ======
   String* newURL = new String(urlToInsert);
   CHECK_PTR(newURL);
   String* url = URLList.first();
   while(url != NULL) {
      if(*url == urlToInsert) {
         URLList.remove(url);
         delete url;
      }
      url = URLList.next();
   }
   URLList.insert(0,newURL);
   if(URLList.count() > LocationCount) {
      url = URLList.last();
      URLList.removeLast();
      delete url;
   }

   // ====== Update menu ====================================================
   url = URLList.first();
   for(cardinal i = MenuIDLocation;i < MenuIDLocation + LocationCount;i++) {
      if(url != NULL) {
         URLMenu->changeItem(i,url->getData());
         URLMenu->setItemEnabled(i,true);
         url = URLList.next();
      }
      else {
         URLMenu->changeItem(i,"()");
         URLMenu->setItemEnabled(i,false);
      }
   }
}


// ###### Remove all bookmarks ##############################################
void QClient::clearBookmarks()
{
   // ====== Delete URL list ================================================
   String* url = URLList.first();
   while(url != NULL) {
      URLList.removeFirst();
      delete url;
      url = URLList.first();
   }

   // ====== Update menu ====================================================
   for(cardinal i = MenuIDLocation;i < MenuIDLocation + LocationCount;i++) {
      URLMenu->changeItem(i,"()");
      URLMenu->setItemEnabled(i,false);
   }
}


// ###### Load bookmarks ####################################################
void QClient::loadBookmarks()
{
   ifstream is("qclient-bookmarks");
   if(is.good()) {
      clearBookmarks();
      while(!is.eof()) {
         char str[512];
         is.getline(str,512);
         if((strlen((char*)&str) > 1) && (str[0] != '#')) {
            insertURL(String((char*)&str));
         }
      }
   }
}


// ###### Save bookmarks ####################################################
void QClient::saveBookmarks()
{
   ofstream os("qclient-bookmarks");
   if(os.good()) {
      os << "# -- QClient 1.00 bookmarks file --" << endl;
      os << "# This is an automatically generated file." << endl;
      os << "# It will be read and overwritten. Do *not* edit!" << endl;
      os << "#" << endl;
      String* url = URLList.last();
      while(url != NULL) {
         os << *url << endl;
         url = URLList.prev();
      }
   }
}


// ###### Convert bytes value into string ###################################
QString QClient::bytesToQString(const card64 bytes) const
{
   double bytesDouble = (double)bytes;
   char str[128];

   if(bytesDouble >= 1024.0) {
      char unit;
      bytesDouble /= 1024.0;
      if(bytesDouble >= 1024.0) {
         bytesDouble /= 1024.0;
         if(bytesDouble >= 1024.0) {
            bytesDouble /= 1024.0;
            unit = 'G';
         }
         else {
            unit = 'M';
         }
      }
      else {
         unit = 'K';
      }
      if(bytes >= 1000000000)
         snprintf((char*)&str,sizeof(str),"%1.5e (%1.2f %cB)",(double)bytes,bytesDouble,unit);
      else
         snprintf((char*)&str,sizeof(str),"%Ld (%1.2f %cB)",bytes,bytesDouble,unit);
   }
   else {
      snprintf((char*)&str,sizeof(str),"%Ld",bytes);
   }
   return(QString((char*)&str));
}


// ###### Convert card64 value into string ##################################
QString QClient::card64ToQString(const card64 value,
                                 const char*  formatString) const
{
   char str[128];
   snprintf((char*)&str,sizeof(str),formatString,value);
   return(QString((char*)&str));
}


// ###### Convert percent value into string #################################
QString QClient::doubleToQString(const double value,
                                 const char*  formatString) const
{
   char str[128];
   snprintf((char*)&str,sizeof(str),formatString,value);
   return(QString((char*)&str));
}


// ###### Convert flow into to string #######################################
QString QClient::flowInfoToQString(const card8  trafficClass,
                                   const card32 flowLabel) const
{
   // ====== Convert traffic class ==========================================
   QString result;
   const char* tvalue = TrafficClassValues::getNameForTrafficClass(trafficClass);
   if(tvalue != NULL) {
      result += tvalue;
   }
   else {
      result += card64ToQString((card64)trafficClass,"$%02Ld");
   }

   // ====== Convert flow label =============================================
   if(flowLabel != 0) {
      result += card64ToQString((card64)flowLabel," / $%05Lx");
   }
   return(result);
}



// ###### Main program ######################################################
int main(int argc, char* argv[])
{
   // ===== Check arguments =================================================
   cardinal optAudioDevice = 0;
   cardinal optAudioDebug  = 0;
   cardinal optAudioNull   = 0;
   cardinal optAnalyzer    = 1;
   cardinal optMixer       = 1;
   bool     optForceIPv4   = false;
   bool     optUseSCTP     = false;
   char*    local          = NULL;
   char*    defaultURL     = NULL;
   for(cardinal i = 1;i < (cardinal)argc;i++) {
      if(!(strcasecmp(argv[i],"+debug")))             optAudioDebug  = 1;
      else if(!(strcasecmp(argv[i],"+null")))         optAudioNull   = 1;
      else if(!(strcasecmp(argv[i],"+device")))       optAudioDevice = 1;
      else if(!(strcasecmp(argv[i],"+analyzer")))     optAnalyzer    = 2;
      else if(!(strcasecmp(argv[i],"+mixer")))        optMixer       = 2;
      else if(!(strcasecmp(argv[i],"-debug")))        optAudioDebug  = 0;
      else if(!(strcasecmp(argv[i],"-null")))         optAudioNull   = 0;
      else if(!(strcasecmp(argv[i],"-device")))       optAudioDevice = 0;
      else if(!(strcasecmp(argv[i],"-analyzer")))     optAnalyzer    = 0;
      else if(!(strcasecmp(argv[i],"-mixer")))        optMixer       = 0;
      else if(!(strcasecmp(argv[i],"-force-ipv4")))   optForceIPv4   = true;
      else if(!(strcasecmp(argv[i],"-sctp")))         optUseSCTP     = true;
      else if(!(strcasecmp(argv[i],"-nosctp")))       optUseSCTP     = false;
      else if(!(strncasecmp(argv[i],"-url=",5)))      defaultURL     = &argv[i][5];
      else if(!(strncasecmp(argv[i],"-local=hostname",7))) local     = &argv[i][7];
      else {
         cerr << "Usage: " << argv[0] << endl
              << " {-url=URL} {[+/-]debug} {[+/-]null} {[+/-]device} {[+/-]analyzer} {[+/-]mixer} {-force-ipv4} {-local=hostname{:port}}" << endl;
         exit(0);
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == true) {
         InternetAddress::UseIPv6 = false;
         cerr << "NOTE: IPv6 support disabled!" << endl;
      }
   }
   if((optUseSCTP) && (local == NULL)) {
      cerr << "ERROR: No local hostname given but required for SCTP!" << endl;
      exit(1);
   }
   if((optAudioDebug == 0) && (optAudioNull == 0) && (optAudioDevice == 0)) {
      optAudioDevice = 1;
      optAnalyzer    = 2;
      optMixer       = 2;
   }
   if(optAudioDevice == 0) {
      optMixer = 0;
   }
   if((optAudioDebug >= 1) || (optAudioNull >= 1) || (optAudioDevice >= 1) || (optAnalyzer >= 1)) {
      optAudioNull = 0;
   }
   if((optAudioDebug == 0) && (optAudioDevice == 0) && (optAnalyzer == 0)) {
      optAudioNull = 0;
   }

   // ====== Initialize audio output device =================================
   MultiAudioWriter* audioOutput = new MultiAudioWriter();
   CHECK_PTR(audioOutput);
   if(optAudioDebug > 0) {
      AudioDebug* audioDebug = new AudioDebug();
      CHECK_PTR(audioDebug);
      audioOutput->addWriter(audioDebug);
   }
   if(optAudioDevice > 0) {
      AudioDevice* audioDevice = new AudioDevice();
      CHECK_PTR(audioDevice);
      if(audioDevice->ready()) {
         audioOutput->addWriter(audioDevice);
      }
      else {
         delete audioDevice;
         audioDevice = NULL;
         optMixer    = 0;
      }
   }
   if(optAudioNull > 0) {
      AudioNull* audioNull = new AudioNull();
      CHECK_PTR(audioNull);
      audioOutput->addWriter(audioNull);
   }
   SpectrumAnalyzer* spectrumAnalyzer = NULL;
   if(optAnalyzer > 0) {
      spectrumAnalyzer = new SpectrumAnalyzer();
      CHECK_PTR(spectrumAnalyzer);
      audioOutput->addWriter(spectrumAnalyzer);
   }

   // ====== Initialize audio mixer =========================================
   AudioMixer* mixer = NULL;
   if(optMixer > 0) {
      mixer = new AudioMixer();
      CHECK_PTR(mixer);
      if(mixer->ready() == false) {
         cerr << "WARNING: Audio mixer not ready => Disabling mixer!" << endl;
         delete mixer;
         mixer = NULL;
      }
   }

   // ======Initialize GUI ==================================================
   QApplication* application = new QApplication(argc,argv);
   CHECK_PTR(application);
   application->setStyle(new QPlatinumStyle());
   QClient* player = new QClient(audioOutput,local,defaultURL,spectrumAnalyzer,mixer,optUseSCTP);
   CHECK_PTR(player);
   application->setMainWidget(player);
   player->show();
   if(optAnalyzer > 1) {
      player->spectrumAnalyzer();
   }
   if(optMixer > 1) {
      player->audioMixer();
   }
   if(defaultURL != NULL) {
      player->play();
   }
   return application->exec();
}
