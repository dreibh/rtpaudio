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
#include "rtpa-qclient.h"
#include "rtpa-qclient_moc.cc"
#include "audiodevice.h"
#include "audiodebug.h"
#include "audionull.h"
#include "spectrumanalyzer.h"
#include "multiaudiowriter.h"
#include "tdsocket.h"
#include "tools.h"
#include "audioclient.h"
#include "trafficclassvalues.h"


#include <qapplication.h>
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
#include <qmenubar.h>
#include <qmenu.h>

#include <fstream>
#include <assert.h>



// ###### Constructor #######################################################
QClient::QClient(AudioWriterInterface* audioOutput,
                 const char*           receiverName,
                 const char*           defaultURL,
                 SpectrumAnalyzer*     analyzer,
                 AudioMixer*           mixer,
                 QWidget*              parent)
   : QMainWindow(parent)
{
   // ====== Create AudioClient =============================================
   Client                 = new AudioClient(audioOutput);
   Q_CHECK_PTR(Client);
   SpectrumAnalyzerWindow = NULL;
   SpectrumAnalyzerDevice = analyzer;
   MixerWindow            = NULL;
   MixerDevice            = mixer;
   InsertionRequired      = FALSE;

   // ====== Menu ===========================================================
   QMenuBar* menu = menuBar();

   QMenu* fileMenu = new QMenu("&File", this);
   Q_CHECK_PTR(fileMenu);
   fileMenu->addAction("&Load Bookmarks",this,SLOT(loadBookmarks()),Qt::CTRL+Qt::Key_O);
   fileMenu->addAction("&Save Bookmarks",this,SLOT(saveBookmarks()),Qt::CTRL+Qt::Key_W);
   fileMenu->addSeparator();
   fileMenu->addAction("&Quit",this,SLOT(quit()),Qt::CTRL+Qt::Key_Q);
   menu->addMenu(fileMenu);

   QMenu* controlMenu = new QMenu("&Control", this);
   Q_CHECK_PTR(controlMenu);
   controlMenu->addAction("&Play",this,SLOT(play()),Qt::CTRL+Qt::Key_T);
   controlMenu->addAction("&Stop",this,SLOT(stop()),Qt::CTRL+Qt::Key_Z);
   controlMenu->addSeparator();
   controlMenu->addAction("&Toggle Pause",this,SLOT(togglePause()),Qt::CTRL+Qt::Key_U);
   menu->addMenu(controlMenu);

   URLMenu = new QMenu("&Bookmarks", this);
   Q_CHECK_PTR(URLMenu);
   for(cardinal i = 0;i < LocationCount;i++) {
      LocationAction[i] = URLMenu->addAction("()");
      Q_CHECK_PTR(LocationAction[i]);
      LocationAction[i]->setEnabled(FALSE);
      if(i <= 9) {
         LocationAction[i]->setShortcut(Qt::Key_F1 + i);
      }
      if(i == 0) {
         URLMenu->addSeparator();
      }
   }
   URLMenu->addSeparator();
   URLMenu->addAction("&Clear Bookmarks",this,SLOT(clearBookmarks()),Qt::ALT+Qt::Key_K);
   menu->addMenu(URLMenu);
   QObject::connect(URLMenu,SIGNAL(triggered(QAction*)),this,SLOT(locationSelected(QAction*)));

   ToolsMenu = new QMenu("&Tools", this);
   Q_CHECK_PTR(ToolsMenu);
   SpectrumAnalyzerAction = ToolsMenu->addAction("Spectrum Analyzer",this,SLOT(spectrumAnalyzer()),Qt::CTRL+Qt::Key_F);
   Q_CHECK_PTR(SpectrumAnalyzerAction);
   SpectrumAnalyzerAction->setCheckable(TRUE);
   SpectrumAnalyzerAction->setChecked(FALSE);
   if(mixer != NULL) {
      MixerAction = ToolsMenu->addAction("Audio Mixer",this,SLOT(audioMixer()),Qt::CTRL+Qt::Key_M);
      Q_CHECK_PTR(MixerAction);
      MixerAction->setCheckable(TRUE);
      MixerAction->setChecked(FALSE);
   }
   else {
      MixerAction = NULL;
   }
   menu->addMenu(ToolsMenu);

   SettingsMenu = new QMenu("&Settings", this);
   Q_CHECK_PTR(SettingsMenu);
   ResolverAction = SettingsMenu->addAction("&Resolve Addresses",this,SLOT(toggleAddressResolution(bool)),Qt::CTRL+Qt::Key_R);
   Q_CHECK_PTR(ResolverAction);
   ResolverAction->setCheckable(TRUE);
   AutoRepeatAction = SettingsMenu->addAction("Auto Repeat");
   AutoRepeatAction->setShortcut(Qt::CTRL+Qt::Key_E);
   Q_CHECK_PTR(AutoRepeatAction);
   AutoRepeatAction->setCheckable(TRUE);
   AutoSaveBookmarksAction = SettingsMenu->addAction("Auto Save Bookmarks");
   Q_CHECK_PTR(AutoSaveBookmarksAction);
   AutoSaveBookmarksAction->setShortcut(Qt::CTRL+Qt::Key_B);
   menu->addMenu(SettingsMenu);
   AutoSaveBookmarksAction->setCheckable(TRUE);

   QMenu* helpMenu = new QMenu("&Help", this);
   Q_CHECK_PTR(helpMenu);
   helpMenu->addAction("&About",this,SLOT(information()),Qt::Key_F11);
   helpMenu->addAction("&What's This?",this,SLOT(whatsThis()),Qt::Key_F12);
   menu->addSeparator();
   menu->addMenu(helpMenu);


   // ====== Main Layout ====================================================
   QWidget* centralWidget = new QWidget(this);
   Q_CHECK_PTR(centralWidget);
   centralWidget->setWhatsThis("This is the RTP Audio Client!");

   QGridLayout* topLayout = new QGridLayout(centralWidget);
   Q_CHECK_PTR(topLayout);
   topLayout->setColumnStretch(0,0);
   topLayout->setColumnStretch(1,10);
   topLayout->setRowStretch(0,10);
   topLayout->setRowStretch(1,0);

   // ====== Status line ====================================================
   QLabel* copyright = new QLabel("Copyright (C) 1999-2012 Thomas Dreibholz",centralWidget);
   Q_CHECK_PTR(copyright);
   copyright->setWhatsThis("RTP Audio Client\nCopyright (C) 1999-2012 Thomas Dreibholz");
   copyright->setAlignment(Qt::AlignRight);
   StatusBar = new QLabel("Welcome to the RTP Audio Client!",centralWidget);
   Q_CHECK_PTR(StatusBar);
   StatusBar->setWhatsThis("The status of a connection will be shown here.");
   StatusBar->setMinimumSize(StatusBar->sizeHint());
   copyright->setMinimumSize(copyright->sizeHint());
   topLayout->addWidget(StatusBar,2,0);
   topLayout->addWidget(copyright,2,1);

   // ====== Quality ========================================================
   QGroupBox* qualityGroup = new QGroupBox("Quality");
   Q_CHECK_PTR(qualityGroup);
   qualityGroup->setWhatsThis("This group contains functions to adapt the audio quality and change the encoding.");
   topLayout->addWidget(qualityGroup,1,1);

   QVBoxLayout* qualityLayout = new QVBoxLayout(qualityGroup);
   Q_CHECK_PTR(qualityLayout);
   QHBoxLayout* checkLayout   = new QHBoxLayout();
   Q_CHECK_PTR(checkLayout);
   QCheckBox*   stereo        = new QCheckBox("Ste&reo",qualityGroup);
   Q_CHECK_PTR(stereo);
   QComboBox*   bits          = new QComboBox(qualityGroup);
   Q_CHECK_PTR(bits);
   QLabel* ipv6               = new QLabel(InternetAddress::hasIPv6() ? "Using IPv6" : "Using IPv4",qualityGroup);
   Q_CHECK_PTR(ipv6);
   QComboBox* rate            = new QComboBox(qualityGroup);
   Q_CHECK_PTR(rate);
   QComboBox* encoder         = new QComboBox(qualityGroup);
   Q_CHECK_PTR(encoder);

   for(integer i = AudioQuality::ValidBits - 1;i >= 0;i--) {
      char str[64];
      snprintf((char*)&str,sizeof(str),"%d Bits",AudioQuality::ValidBitsTable[i]);
      bits->insertItem(AudioQuality::ValidBits - i - 1, (char*)&str);
   }
   for(integer i = AudioQuality::ValidRates - 1;i >= 0;i--) {
      char str[64];
      snprintf((char*)&str,sizeof(str),"%d Hz",AudioQuality::ValidRatesTable[i]);
      rate->insertItem(AudioQuality::ValidRates - i - 1, (char*)&str);
   }
   for(cardinal index = 0;;index++) {
      const char* encoding = Client->getEncodingName(index);
      if(encoding != NULL)
         encoder->insertItem(index, encoding);
      else
         break;
   }
   stereo->setWhatsThis("Check this box for stereo audio quality instead of mono.");
   bits->setWhatsThis("You can select the number of audio bits here.");
   rate->setWhatsThis("You can select the audio sampling rate here.");
   encoder->setWhatsThis("You can select an encoding for the audio transport here.");
   ipv6->setWhatsThis("This label shows whether your system supports IPv6 or not.");

   stereo->setMinimumSize(stereo->sizeHint());
   stereo->setChecked(TRUE);
   rate->setMinimumSize(rate->sizeHint());
   encoder->setMinimumSize(encoder->sizeHint());

   checkLayout->addWidget(bits);
   checkLayout->addWidget(stereo);
   checkLayout->addWidget(ipv6);
   qualityLayout->addLayout(checkLayout);
   qualityLayout->addWidget(rate);
   qualityLayout->addWidget(encoder);


   // ====== Transmission Status ============================================
   InfoWidget = new QInfoTabWidget(&InfoTable1,"Connection","pinguin.xpm",centralWidget);
   Q_CHECK_PTR(InfoWidget);
   char str[128];
   for(cardinal i = 0;i < MaxLayerInfo;i++) {
      snprintf((char*)&str,sizeof(str),"L #%d",i);
      LayerInfo[i] = InfoWidget->addTable(&InfoTable2,(char*)&str,"info.xpm");
   }
   InfoWidget->setMinimumWidth(350);
   topLayout->addWidget(InfoWidget,0,1);


   // ====== Main Display ===================================================
   QGroupBox* displayGroup = new QGroupBox("Media Information",centralWidget);
   Q_CHECK_PTR(displayGroup);
   displayGroup->setWhatsThis("Information on the current media playing are shown in this group.");
   topLayout->addWidget(displayGroup,0,0);

   QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
   Q_CHECK_PTR(displayLayout);
   Counter = new QLCDNumber(10,displayGroup);
   Q_CHECK_PTR(Counter);
   Counter->setMinimumSize(180,80);
   displayLayout->addWidget(Counter);
   updateCounter(0);

   QGroupBox* mediaInfoGroup = new QGroupBox(displayGroup);
   Q_CHECK_PTR(mediaInfoGroup);
   QGridLayout* mediaInfoLayout = new QGridLayout(mediaInfoGroup);
   Q_CHECK_PTR(mediaInfoLayout);
   mediaInfoLayout->setColumnStretch(1,10);

   TitleLabel = new QLabel("N/A",mediaInfoGroup);
   Q_CHECK_PTR(TitleLabel);
   TitleLabel->setMinimumSize(TitleLabel->sizeHint());
   ArtistLabel = new QLabel("N/A",mediaInfoGroup);
   Q_CHECK_PTR(ArtistLabel);
   ArtistLabel->setMinimumSize(ArtistLabel->sizeHint());
   CommentLabel = new QLabel("N/A",mediaInfoGroup);
   Q_CHECK_PTR(CommentLabel);
   CommentLabel->setMinimumSize(CommentLabel->sizeHint());

   QLabel* titleLabel = new QLabel("Title:",mediaInfoGroup);
   Q_CHECK_PTR(titleLabel);
   mediaInfoLayout->addWidget(titleLabel,0,0);
   QLabel* artistLabel = new QLabel("Artist:",mediaInfoGroup);
   Q_CHECK_PTR(artistLabel);
   mediaInfoLayout->addWidget(artistLabel,1,0);
   QLabel* commentLabel = new QLabel("Comment:",mediaInfoGroup);
   Q_CHECK_PTR(commentLabel);
   mediaInfoLayout->addWidget(commentLabel,2,0);

   mediaInfoLayout->addWidget(TitleLabel,0,1);
   mediaInfoLayout->addWidget(ArtistLabel,1,1);
   mediaInfoLayout->addWidget(CommentLabel,2,1);
   displayLayout->addWidget(mediaInfoGroup);
   Counter->setWhatsThis("This counter shows the position within the audio file in minutes/seconds/0.01 seconds format.");
   TitleLabel->setWhatsThis("This is the title of the file playing");
   ArtistLabel->setWhatsThis("This is the artist or the file playing");
   CommentLabel->setWhatsThis("This is a comment on the file playing");


   // ====== Control ========================================================
   QGroupBox* controlGroup = new QGroupBox("Server Control",centralWidget);
   Q_CHECK_PTR(controlGroup);
   controlGroup->setWhatsThis("This group contains server control functions.");
   topLayout->addWidget(controlGroup,1,0);

   QGridLayout* controlLayout = new QGridLayout(controlGroup);
   Q_CHECK_PTR(controlLayout);
   QPushButton* whatsThis     = new QPushButton("H&elp",controlGroup);
   Q_CHECK_PTR(whatsThis);
   QPushButton* play          = new QPushButton("&Play",controlGroup);
   Q_CHECK_PTR(play);
   QPushButton* stop          = new QPushButton("St&op",controlGroup);
   Q_CHECK_PTR(stop);
   Pause                      = new QPushButton("P&ause",controlGroup);
   Q_CHECK_PTR(Pause);

   bits->setMinimumHeight(stop->height());
   rate->setMinimumHeight(stop->height());
   encoder->setMinimumHeight(stop->height());

   whatsThis->setWhatsThis("Click here to enter What's This mode.");
   play->setWhatsThis("Click here to start playing.");
   stop->setWhatsThis("Click here to stop playing.");
   Pause->setWhatsThis("Click here to pause playing.");
   ScrollBar = new QScrollBar(Qt::Horizontal,controlGroup);
   Q_CHECK_PTR(ScrollBar);
   ScrollBar->setWhatsThis("Move this scrollbar to change the current position within a playing audio file.");
   ScrollBarUpdated     = TRUE;
   ScrollBarUpdateDelay = 0;
   ScrollBar->setRange(0,0);
   ScrollBarUpdated = TRUE;
   ScrollBar->setValue(0);

   QLabel* label = new QLabel("Source URL: (Example: rtpa://gaffel:7500/Test1.list)",controlGroup);
   Q_CHECK_PTR(label);
   Location      = new QLineEdit(controlGroup);
   Q_CHECK_PTR(Location);
   QObject::connect(Location,SIGNAL(returnPressed()),this,SLOT(play()));
   label->setWhatsThis("This is an example for a location.");
   Location->setWhatsThis("Enter the location of the audio list to play here.\nExample: rtpa://gaffel:7500/Test1.list");

   play->setMinimumSize(play->sizeHint());
   whatsThis->setMinimumSize(whatsThis->sizeHint());
   stop->setMinimumSize(stop->sizeHint());
   Pause->setMinimumSize(Pause->sizeHint());
   Pause->setCheckable(TRUE);
   ScrollBar->setMinimumSize(ScrollBar->sizeHint());
   Location->setMinimumSize(Location->sizeHint());
   loadBookmarks();
   if(defaultURL != NULL)
      Location->setText(defaultURL);
   else {
      loadBookmarks();
      QList<String*>::iterator found = URLList.begin();
      if(found != URLList.end()) {
         const String* url = *found;
          Location->setText(url->getData());
      }
      else {
         if(Client->getIPVersion() == 6)
            Location->setText("rtpa://[::1]:7500/Test.list");
         else
            Location->setText("rtpa://localhost:7500/Test.list");
      }
   }
   label->setMinimumSize(label->sizeHint());

   controlLayout->addWidget(play,0,0);
   controlLayout->addWidget(stop,0,1);
   controlLayout->addWidget(Pause,0,2);
   controlLayout->addWidget(whatsThis,0,3);
   controlLayout->addWidget(ScrollBar,1,0,1,4);
   controlLayout->addWidget(label,2,0,1,4);
   controlLayout->addWidget(Location,3,0,1,4);


   // ====== Connect widgets to methods =====================================
   QObject::connect(whatsThis,SIGNAL(clicked()),this,SLOT(whatsThis()));
   QObject::connect(play,SIGNAL(clicked()),this,SLOT(play()));
   QObject::connect(stop,SIGNAL(clicked()),this,SLOT(stop()));
   QObject::connect(Pause,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
   QObject::connect(ScrollBar,SIGNAL(valueChanged(int)),this,SLOT(position(int)));
   QObject::connect(rate,SIGNAL(activated(int)),this,SLOT(setSamplingRate(int)));
   QObject::connect(stereo,SIGNAL(toggled(bool)),this,SLOT(setChannels(bool)));
   QObject::connect(bits,SIGNAL(activated(int)),this,SLOT(setBits(int)));
   QObject::connect(encoder,SIGNAL(activated(int)),this,SLOT(setEncoding(int)));


   // ====== Create new QTimer ==============================================
   QTimer* timer = new QTimer(this);
   Q_CHECK_PTR(timer);
   timer->QObject::connect(timer,SIGNAL(timeout()),this,SLOT(timerEvent()));
   timer->start(250);

   setCentralWidget(centralWidget);
   setWindowTitle("RTP Audio Client");
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
      "RTP Audio Client - Version 2.00\n\n"
      "Copyright (C) 1999-2012\n"
      "Thomas Dreibholz\n"
      "dreibh@iem.uni-due.de.de",
      "Okay");
}


// ###### Show encoder error window #########################################
void QClient::showError(const cardinal error)
{
   char        str[256];
   char        strDefault[64];
   const char* errorString;
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
   const bool newOK = scanURL((const char*)Location->text().toUtf8().constData(),protocol,host,path);
   protocol = protocol.toLower();
   if((newOK == FALSE) || ((protocol != "rtpa") && (protocol != "rtpa+udp") && (protocol != "rtpa+sctp"))) {
      StatusBar->setText("Invalid URL! Check URL and try again.");
      QMessageBox::warning(this,"Warning",
                           "This URL is invalid!\n"
                           "Check URL and try again.\n"
                           "Example: rtpa://odin:7500/Test.list",
                           "Okay!");
      return;
   }

   // ====== Check, if URL is new => Restart or change media ================
   if(Client->playing() == TRUE) {
      const String newURL = String(Location->text().toUtf8().constData());
      if(PlayingURL == newURL) {
         Pause->setChecked(FALSE);
         return;
      }

      String oldProtocol;
      String oldHost;
      String oldPath;
      const bool oldOK = scanURL(PlayingURL,oldProtocol,oldHost,oldPath);
      oldProtocol = oldProtocol.toLower();
      if(oldOK == TRUE) {
         if((oldHost == host) && (oldProtocol == protocol)) {
            Client->change((const char*)Location->text().toUtf8().constData());
            Pause->setChecked(FALSE);
            PlayingURL = newURL;
            InsertionRequired = TRUE;
            return;
         }
      }
      stop();
   }

   // ====== Start client ===================================================
   Pause->setChecked(FALSE);
   PlayingURL = (const char*)Location->text().toUtf8().constData();
   const bool ok = Client->play(PlayingURL.getData());

   // ====== Update status display ==========================================
   if(ok) {
      EOFRepeatDelay = 0;
      StatusBar->setText("Waiting for data from server...");
      const InternetAddress::PrintFormat format =
         (ResolverAction->isChecked()) ? InternetAddress::PF_Address :
                                         InternetAddress::PF_Hostname;
      const String serverAddress = Client->getServerAddressString(format);
      const String ourAddress    = Client->getOurAddressString(format);
      InfoWidget->update("SA",serverAddress.getData());
      InfoWidget->update("CA",ourAddress.getData());
      InfoWidget->update("CSSRC",card64ToQString(Client->getOurSSRC(),"$%08x"));
      InsertionRequired = TRUE;
   }
   else {
      StatusBar->setText("Unable to find server! Check parameters and try again.");
      QMessageBox::warning(this,"Warning",
                           "Unable to find server!\n"
                           "Check parameters and try again.\n"
                           "Example: rtpa+udp://odin:7500/CD1.list",
                           "Okay!");
   }
}


// ###### Stop playing ######################################################
void QClient::stop()
{
   InsertionRequired = FALSE;
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
void QClient::toggleAddressResolution(bool selected)
{
   if(Client->playing()) {
      const InternetAddress::PrintFormat format =
         (ResolverAction->isChecked()) ? InternetAddress::PF_Address :
                                         InternetAddress::PF_Hostname;
      const String serverAddress = Client->getServerAddressString(format);
      const String ourAddress    = Client->getOurAddressString(format);
      InfoWidget->update("SA",serverAddress.getData());
      InfoWidget->update("CA",ourAddress.getData());
   }
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
            SpectrumAnalyzerAction->setChecked(TRUE);
         }
      }
      else {
         delete SpectrumAnalyzerWindow;
         SpectrumAnalyzerWindow = NULL;
         SpectrumAnalyzerAction->setChecked(FALSE);
      }
   }
}


// ###### Close spectrum analyzer ############################################
void QClient::closeSpectrumAnalyzer()
{
   if(SpectrumAnalyzerWindow != NULL) {
      delete SpectrumAnalyzerWindow;
      SpectrumAnalyzerWindow = NULL;
      SpectrumAnalyzerAction->setChecked(FALSE);
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
            MixerAction->setChecked(TRUE);
         }
      }
      else {
         delete MixerWindow;
         MixerWindow = NULL;
         MixerAction->setChecked(FALSE);
      }
   }
}


// ###### Close audio mixer #################################################
void QClient::closeAudioMixer()
{
   if(MixerWindow != NULL) {
      delete MixerWindow;
      MixerWindow = NULL;
      MixerAction->setChecked(FALSE);
   }
}


// ###### Quit QClient ######################################################
void QClient::quit()
{
   stop();
   if(AutoSaveBookmarksAction->isChecked()) {
      saveBookmarks();
   }
   exit(0);
}


// ###### Update counter ####################################################
void QClient::updateCounter(card64 position)
{
   // ====== Update frame counter ===========================================
   char str[32];
   snprintf((char*)&str,sizeof(str),"%08lld",
            (long long)(position / (PositionStepsPerSecond / 1000)));

   // ====== Update time counter ============================================
   const card64 seconds = position / PositionStepsPerSecond;
   snprintf((char*)&str,sizeof(str),
            "%02u:%02u.%02u",
            (unsigned int)(seconds / 60),
            (unsigned int)(seconds % 60),
            (unsigned int)((position % PositionStepsPerSecond) / (PositionStepsPerSecond / 100)));
   Counter->display((char*)&str);
}


// ###### Set new position ##################################################
void QClient::position(int value)
{
   if(!ScrollBarUpdated) {
      const card64 position = (card64)value * (PositionStepsPerSecond / 10);
      Client->setPosition(position);
      Pause->setChecked(FALSE);
      ScrollBarUpdateDelay = MaxScrollBarUpdateDelay;
   }
   else {
      ScrollBarUpdated = FALSE;
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
      if(InsertionRequired == TRUE) {
         InsertionRequired = FALSE;
         insertURL(PlayingURL);
      }


      // ====== Display source state info ===================================
      InfoWidget->update("SSSRC",card64ToQString(Client->getServerSSRC(),"$%08x"));

      const cardinal layers = Client->getLayers();
      QString flString;
      QString jString;
      card64  totalLost = 0;

      for(cardinal i = 0;i < std::min(layers,(cardinal)3);i++) {
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
      flString = card64ToQString(totalLost,"%lld ") + flString;
      InfoWidget->update("PL",flString);


      // ====== Display receiver statistics =================================
      InfoWidget->update("PR",card64ToQString(Client->getPacketsReceived((cardinal)-1)) +
                              doubleToQString(Client->getRawBytesPerSecond() / 1024.0," (%1.2f KiB/s)"));
      InfoWidget->update("BR",bytesToQString(Client->getBytesReceived((cardinal)-1)));


      // ====== Display layer statistics ====================================
      for(cardinal i = 0;i < MaxLayerInfo;i++) {
         InternetFlow flow = Client->getInternetFlow(i);
         flow.setPrintFormat((ResolverAction->isChecked()) ? InternetAddress::PF_Address :
                                                             InternetAddress::PF_Hostname);
         LayerInfo[i]->update("LSA",((InternetAddress)flow).getAddressString().getData());
         LayerInfo[i]->update("LTF",flowInfoToQString(flow.getTrafficClass(),flow.getFlowLabel()));
         LayerInfo[i]->update("LBR",bytesToQString(Client->getBytesReceived(i)));
         LayerInfo[i]->update("LPR",card64ToQString(Client->getPacketsReceived(i)));
         LayerInfo[i]->update("LFL",doubleToQString(100.0 * Client->getFractionLost(i),"%1.2f %%"));
      }


      if(error == ME_EOF) {
         // ====== End of media -> Auto-repeat ==============================
         if(!Pause->isChecked()) {
            if(AutoRepeatAction->isChecked()) {
               EOFRepeatDelay++;
               snprintf((char*)&str,sizeof(str),
                        "End of media reached! Auto-restart in %d seconds...",
                        (int)((EOFRepeatInterval - EOFRepeatDelay) * DisplayUpdateInterval) / 1000);
               StatusBar->setText((char*)&str);
               if(EOFRepeatDelay >= EOFRepeatInterval) {
                  EOFRepeatDelay = 0;
                  ScrollBarUpdated = FALSE;
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
      TitleLabel->setText(QString::fromUtf8(mediaInfo.Title).trimmed());
      ArtistLabel->setText(QString::fromUtf8(mediaInfo.Artist).trimmed());
      CommentLabel->setText(QString::fromUtf8(mediaInfo.Comment).trimmed());


      // ====== Update counter ==============================================
      updateCounter(Client->getPosition());


      // ====== Update scrollbar's range ====================================
      if(!ScrollBar->isSliderDown()) {
         if(ScrollBarUpdateDelay > 0)
            ScrollBarUpdateDelay--;
         if(ScrollBarUpdateDelay == 0) {
            ScrollBarUpdated = TRUE;
            ScrollBar->setRange(0,Client->getMaxPosition() / (PositionStepsPerSecond / 10));
            ScrollBarUpdated = TRUE;
            ScrollBar->setValue(Client->getPosition() / (PositionStepsPerSecond / 10));
         }
      }
   }
   else {
      InfoWidget->clear();
   }
}


// ###### Location selected slot ############################################
void QClient::locationSelected(QAction* action)
{
   if((Location->text() != action->text()) || (!Client->playing())) {
      if(action->text().left(4) == "rtpa") {
         Location->setText(action->text());
         play();
      }
   }
}

// ###### Insert current URL into URL list ##################################
void QClient::insertURL(const String& urlToInsert, const int where)
{
   // ====== Remove old copy of new URL and insert URL at head of list ======
   String* newURL = new String(urlToInsert);
   Q_CHECK_PTR(newURL);
   for(QList<String*>::iterator iterator = URLList.begin();
       iterator != URLList.end(); ++iterator) {
      String* url = *iterator;
      if(*url == urlToInsert) {
         URLList.erase(iterator);
         delete url;
         break;
      }
   }
   URLList.insert(where,newURL);
   if(URLList.count() > (int)LocationCount) {
      String* url = URLList.last();
      URLList.removeLast();
      delete url;
   }

   // ====== Update menu ====================================================
   unsigned int i = 0;
   for(QList<String*>::iterator iterator = URLList.begin();
      iterator != URLList.end(); ++iterator) {
      assert(i < LocationCount);
      String* url = *iterator;
      if(url != NULL) {
         LocationAction[i]->setText(url->getData());
         LocationAction[i]->setEnabled(TRUE);
      }
      else {
         LocationAction[i]->setText("()");
         LocationAction[i]->setEnabled(FALSE);
      }
      i++;
   }
}


// ###### Remove all bookmarks ##############################################
void QClient::clearBookmarks()
{
   // ====== Delete URL list ================================================
   QList<String*>::iterator iterator = URLList.begin();
   while(iterator != URLList.end()) {
      String* url = *iterator;
      delete url;
      URLList.removeFirst();
      iterator = URLList.begin();
   }

   // ====== Update menu ====================================================
   for(cardinal i = 0; i < LocationCount; i++) {
      LocationAction[i]->setText("()");
      LocationAction[i]->setEnabled(FALSE);
   }
}


// ###### Load bookmarks ####################################################
void QClient::loadBookmarks()
{
   std::ifstream is("qclient-bookmarks");
   if(is.good()) {
      clearBookmarks();
      while(!is.eof()) {
         char str[512];
         is.getline(str,512);
         if((strlen((char*)&str) > 1) && (str[0] != '#')) {
            insertURL(String((char*)&str), URLList.size());
         }
      }
   }
}


// ###### Save bookmarks ####################################################
void QClient::saveBookmarks()
{
   std::ofstream os("qclient-bookmarks");
   if(os.good()) {
      os << "# -- QClient 1.00 bookmarks file --" << std::endl;
      os << "# This is an automatically generated file." << std::endl;
      os << "# It will be read and overwritten. Do *not* edit!" << std::endl;
      os << "#" << std::endl;
      for(QList<String*>::iterator iterator = URLList.begin();
          iterator != URLList.end(); ++iterator) {
         String* url = *iterator;
         os << *url << std::endl;
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
         snprintf((char*)&str,sizeof(str),"%1.5e (%1.2f %ciB)",
                  (double)bytes,bytesDouble,unit);
      else
         snprintf((char*)&str,sizeof(str),"%llu (%1.2f %ciB)",
                  (unsigned long long)bytes,bytesDouble,unit);
   }
   else {
      snprintf((char*)&str,sizeof(str),"%Lu",
               (unsigned long long)bytes);
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
   cardinal optAudioDevice = 1;
   cardinal optAudioDebug  = 0;
   cardinal optAudioNull   = 0;
   cardinal optAnalyzer    = 1;
   cardinal optMixer       = 1;
   bool     optForceIPv4   = FALSE;
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
      else if(!(strcasecmp(argv[i],"-force-ipv4")))   optForceIPv4   = TRUE;
      else if(!(strncasecmp(argv[i],"-url=",5)))      defaultURL     = &argv[i][5];
      else if(!(strncasecmp(argv[i],"-local=hostname",7))) local     = &argv[i][7];
      else {
         std::cerr << "Usage: " << argv[0] << std::endl
              << " {-url=URL} {[+/-]debug} {[+/-]null} {[+/-]device} {[+/-]analyzer} {[+/-]mixer} {-force-ipv4} {-local=hostname{:port}}" << std::endl;
         exit(0);
      }
   }
   if(optForceIPv4) {
      if(InternetAddress::UseIPv6 == TRUE) {
         InternetAddress::UseIPv6 = FALSE;
         std::cerr << "NOTE: IPv6 support disabled!" << std::endl;
      }
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
   Q_CHECK_PTR(audioOutput);
   if(optAudioDebug > 0) {
      AudioDebug* audioDebug = new AudioDebug();
      Q_CHECK_PTR(audioDebug);
      audioOutput->addWriter(audioDebug);
   }
   AudioDevice* audioDevice = NULL;
   if(optAudioDevice > 0) {
      audioDevice = new AudioDevice();
      Q_CHECK_PTR(audioDevice);
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
      Q_CHECK_PTR(audioNull);
      audioOutput->addWriter(audioNull);
   }
   SpectrumAnalyzer* spectrumAnalyzer = NULL;
   if(optAnalyzer > 0) {
      spectrumAnalyzer = new SpectrumAnalyzer();
      Q_CHECK_PTR(spectrumAnalyzer);
      audioOutput->addWriter(spectrumAnalyzer);
   }

   // ====== Initialize audio mixer =========================================
   AudioMixer* mixer = NULL;
   if(optMixer > 0) {
      mixer = new AudioMixer(audioDevice);
      Q_CHECK_PTR(mixer);
      if(mixer->ready() == FALSE) {
         std::cerr << "NOTE: Audio mixer not ready => Disabling mixer!" << std::endl;
         delete mixer;
         mixer = NULL;
      }
   }

   // ======Initialize GUI ==================================================
   QApplication* application = new QApplication(argc,argv);
   Q_CHECK_PTR(application);
   QClient* player = new QClient(audioOutput,local,defaultURL,spectrumAnalyzer,mixer);
   Q_CHECK_PTR(player);
   application->setActiveWindow(player);
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
