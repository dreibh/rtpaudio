// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QAudioMixer                                                      ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2020 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@iem.uni-due.de                                  ####
// ####    WWW:   https://www.uni-due.de/~be0001/rtpaudio                ####
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


#ifndef QAUDIOMIXER_H
#define QAUDIOMIXER_H


#include "tdsystem.h"
#include "audiomixer.h"


#include <qapplication.h>
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
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
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
     */
   QAudioMixer(AudioMixer* mixer,
               QWidget*    parent = NULL);

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
     * Qt slot: Update volume.
     */
   void updateVolumeFromDevice();

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
   void setVolumeOnDevice();
   void updateText(const card8 left, const card8 right);


   integer      VolumeSetting;
   integer      BalanceSetting;

   AudioMixer*  Mixer;
   QPushButton* Mute;
   QSlider*     Balance;
   QSlider*     Volume;
   QLabel*      Values;
};


#endif
