// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QAudioMixer                                                      ####
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


#ifndef QAUDIOMIXER_H
#define QAUDIOMIXER_H


#include "tdsystem.h"
#include "audiomixer.h"


#include <qapp.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qmainwindow.h>


/**
  * This class is a Qt GUI for the audio mixer.
  *
  * @short   QAudioMixer
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class QAudioMixer : public QMainWindow
{
   // ====== Constructor/Destructor =========================================
   Q_OBJECT
   public:
   /**
     * Constructor.
     *
     * @param mixer AudioMixer object.
     * @param parent Parent widget.
     * @param name Widget name.
     */
   QAudioMixer(AudioMixer* mixer,
               QWidget*    parent = NULL,
               const char* name   = NULL);

   /**
     * Destructor.
     */
   ~QAudioMixer();


   // ====== Qt slots =======================================================
   public slots:
   /**
     * Qt slot: Change balance.
     */
   void balance(int value);

   /**
     * Qt slot: Change volume.
     */
   void volume(int value);

   /**
     * Qt slot: Center balance slider.
     */
   void centerBalance();

   /**
     * Qt slot: Mute.
     */
   void mute();


   // ====== Qt signals =====================================================
   signals:
   /**
     * Qt signal: Emitted, when "Close" or window's close button is clicked.
     */
   void closeAudioMixer();


   // ====== Private data ===================================================
   private:
   void closeEvent(QCloseEvent* event);
   void update();


   integer      VolumeSetting;
   integer      BalanceSetting;

   AudioMixer*  Mixer;
   QPushButton* Mute;
   QSlider*     Balance;
   QSlider*     Volume;
   QLabel*      Values;
};


#endif
