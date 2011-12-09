// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QAudioMixer implementation                                       ####
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
   QGroupBox* controlGroup = new QGroupBox("AudioMixer",centralWidget);
   Q_CHECK_PTR(controlGroup);
   layout->addWidget(controlGroup);
   QGridLayout* controlLayout = new QGridLayout(controlGroup);
   Q_CHECK_PTR(controlLayout);

   // ====== Balance ========================================================
   QLabel* label1 = new QLabel("Balance:",controlGroup);
   Q_CHECK_PTR(label1);
   controlLayout->addWidget(label1,0,0);
   Balance = new QSlider(Qt::Horizontal,controlGroup);
//    0,100,5,50,   ????
   Q_CHECK_PTR(Balance);
   Balance->setTickPosition(QSlider::TicksBelow);
   Balance->setTickInterval(10);
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
//   0,100,5,50,   ????
   Q_CHECK_PTR(Volume);
   Volume->setTickPosition(QSlider::TicksBelow);
   Volume->setTickInterval(10);
   controlLayout->addWidget(Volume,1,1);
   Mute = new QPushButton("Mute",controlGroup);
   Q_CHECK_PTR(Mute);
   Mute->setCheckable(true);
   controlLayout->addWidget(Mute,1,2);
   QObject::connect(Mute,SIGNAL(clicked()),this,SLOT(mute()));
   QObject::connect(Volume,SIGNAL(valueChanged(int)),this,SLOT(volume(int)));

   // ====== Values =========================================================
   QLabel* label3 = new QLabel("Values:",controlGroup);
   Q_CHECK_PTR(label3);
   controlLayout->addWidget(label3,2,0);
   Values = new QLabel(controlGroup);
   Q_CHECK_PTR(Values);
   controlLayout->addWidget(Values,2,2,1,2);

   setCentralWidget(centralWidget);
   setWindowTitle("Audio Mixer");


   // ====== Set start values from mixer device =============================
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

      char str[32];
      snprintf((char*)&str,sizeof(str),
               "Left %d %%  /  Right %d %%",(int)left,(int)right);
      Values->setText((char*)&str);
   }
}


// ###### Destructor ########################################################
QAudioMixer::~QAudioMixer()
{
}


// ###### Update volumes on mixer device ####################################
void QAudioMixer::update()
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
         std::cerr << "WARNING: QAudioMixer::update() - Error doing AudioMixer::setVolume()!" << std::endl;
      }

      char str[32];
      snprintf((char*)&str,sizeof(str),
               "Left %d %% / Right %d %%",(int)left,(int)right);
      Values->setText((char*)&str);
   }
   else {
      Mixer->setVolume(0,0);
      Values->setText("-- Device Muted --");
   }
}


// ###### Changed balance ###################################################
void QAudioMixer::balance(int value)
{
   BalanceSetting = (integer)value;
   update();
}


// ###### Changed volume ####################################################
void QAudioMixer::volume(int value)
{
   VolumeSetting = (integer)value;
   update();
}


// ###### Mute ##############################################################
void QAudioMixer::mute()
{
   update();
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
