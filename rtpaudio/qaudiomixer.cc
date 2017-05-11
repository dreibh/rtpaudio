// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QAudioMixer implementation                                       ####
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
#include "qaudiomixer.h"
#include "audiodevice.h"
#include "audiodebug.h"
#include "audioconverter.h"
#include "timedthread.h"


#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qmainwindow.h>
#include <qtimer.h>


// ###### Constructor #######################################################
QAudioMixer::QAudioMixer(AudioMixer* mixer,
                         QWidget*    parent)
   : QMainWindow(parent)
{
   Mixer = mixer;

   // ====== Central widget =================================================
   QWidget* centralWidget = new QWidget(this);
   Q_CHECK_PTR(centralWidget);
   QVBoxLayout* layout    = new QVBoxLayout(centralWidget);
   Q_CHECK_PTR(layout);

   // ====== New group ======================================================
   QGroupBox* controlGroup = new QGroupBox("Stereo Audio Mixer",centralWidget);
   Q_CHECK_PTR(controlGroup);
   layout->addWidget(controlGroup);
   QGridLayout* controlLayout = new QGridLayout(controlGroup);
   Q_CHECK_PTR(controlLayout);

   // ====== Balance ========================================================
   QLabel* label1 = new QLabel("Balance:",controlGroup);
   Q_CHECK_PTR(label1);
   controlLayout->addWidget(label1,0,0);
   Balance = new QSlider(Qt::Horizontal,controlGroup);
   Q_CHECK_PTR(Balance);
   Balance->setMinimum(0);
   Balance->setMaximum(100);
   Balance->setTickPosition(QSlider::TicksBelow);
   Balance->setTickInterval(10);
   Balance->setMinimumWidth(200);
   controlLayout->addWidget(Balance,0,1);
   QPushButton* center = new QPushButton("Center",controlGroup);
   Q_CHECK_PTR(center);
   controlLayout->addWidget(center,0,2);
   QObject::connect(center,SIGNAL(clicked()),this,SLOT(centerBalance()));
   QObject::connect(Balance,SIGNAL(valueChanged(int)),this,SLOT(balance(int)));

   // ====== Volume =========================================================
   QLabel* label2 = new QLabel("Volume:",controlGroup);
   Q_CHECK_PTR(label2);
   controlLayout->addWidget(label2,1,0);
   Volume = new QSlider(Qt::Horizontal,controlGroup);
   Q_CHECK_PTR(Volume);
   Volume->setMinimum(0);
   Volume->setMaximum(100);
   Volume->setTickPosition(QSlider::TicksBelow);
   Volume->setTickInterval(10);
   controlLayout->addWidget(Volume,1,1);
   Mute = new QPushButton("Mute",controlGroup);
   Q_CHECK_PTR(Mute);
   Mute->setCheckable(true);
   controlLayout->addWidget(Mute,1,2);
   QObject::connect(Mute,SIGNAL(clicked()),this,SLOT(mute()));
   QObject::connect(Volume,SIGNAL(valueChanged(int)),this,SLOT(volume(int)));
   QTimer* timer = new QTimer(this);
   Q_CHECK_PTR(timer);
   QObject::connect(timer,SIGNAL(timeout()),this,SLOT(updateVolumeFromDevice()));
   timer->start(1000);

   // ====== Values =========================================================
   QLabel* label3 = new QLabel("Values:",controlGroup);
   Q_CHECK_PTR(label3);
   controlLayout->addWidget(label3,2,0);
   Values = new QLabel(controlGroup);
   Q_CHECK_PTR(Values);
   controlLayout->addWidget(Values,2,1,1,2);

   setCentralWidget(centralWidget);
   setWindowTitle("Audio Mixer");
   updateVolumeFromDevice();
}


// ###### Destructor ########################################################
QAudioMixer::~QAudioMixer()
{
}


// ###### Update volume from device #########################################
void QAudioMixer::updateVolumeFromDevice()
{
   if(Mute->isChecked() == false) {
      // ====== Set start values from mixer device ==========================
      card8 left,right;
      if(Mixer->getVolume(left,right)) {
         if(left > right) {
            VolumeSetting       = (integer)left;
            const integer value = (integer)VolumeSetting - (integer)right;
            BalanceSetting      = 50 - ((value * 50) / VolumeSetting);
         }
         else if(right > left) {
            VolumeSetting       = (integer)right;
            const integer value = (integer)VolumeSetting - (integer)left;
            BalanceSetting      = 50 + ((value * 50) / VolumeSetting);
         }
         else {
            VolumeSetting  = (integer)left;
            BalanceSetting = 50;
         }

         Volume->setValue(VolumeSetting);
         Balance->setValue(BalanceSetting);

         updateText(left, right);
      }
   }
}


// ###### Update volumes on mixer device ####################################
void QAudioMixer::setVolumeOnDevice()
{
   if(Mute->isChecked() == false) {
      integer balance = (integer)BalanceSetting - 50;
      card8 left;
      card8 right;
      if(balance < 0) {
         const card8 value = (card8)(floor(((double)(-balance) * (double)VolumeSetting) / 50.0));
         left  = (card8)VolumeSetting;
         right = (card8)left - value;
      }
      else {
         const card8 value = (card8)(floor(((double)balance * (double)VolumeSetting) / 50.0));
         right = (card8)VolumeSetting;
         left  = (card8)right - value;
      }

      if(Mixer->setVolume(left,right) == false) {
         std::cerr << "WARNING: QAudioMixer::setVolumeOnDevice() - AudioMixer::setVolume() failed!" << std::endl;
      }
      updateText(left, right);
   }
   else {
      Mixer->setVolume(0,0);
      updateText(0, 0);
   }
}


// ###### Changed balance ###################################################
void QAudioMixer::balance(int value)
{
   BalanceSetting = (integer)value;
   setVolumeOnDevice();
}


// ###### Changed volume ####################################################
void QAudioMixer::volume(int value)
{
   VolumeSetting = (integer)value;
   setVolumeOnDevice();
}


// ###### Update volume #####################################################
void QAudioMixer::updateText(const card8 left, const card8 right)
{
   if(Mute->isChecked() == false) {
      char str[32];
      snprintf((char*)&str,sizeof(str),
               "Left %d %%  /  Right %d %%",(int)left,(int)right);
      Values->setText((char*)&str);
   }
   else {
      Mixer->setVolume(0,0);
      Values->setText("-- Device Muted --");
   }
}


// ###### Mute ##############################################################
void QAudioMixer::mute()
{
   setVolumeOnDevice();
}


// ###### Center balance slider #############################################
void QAudioMixer::centerBalance()
{
   Balance->setValue(50);
}


// ###### Close QAudioMixer #################################################
void QAudioMixer::closeEvent(QCloseEvent* event)
{
   emit closeAudioMixer();
}
