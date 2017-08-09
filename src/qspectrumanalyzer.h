// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QSpectrumAnalyzer                                                ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2017 by Thomas Dreibholz            ####
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


#ifndef QSPECTRUMANALYZER_H
#define QSPECTRUMANALYZER_H


#include "tdsystem.h"
#include "spectrumanalyzer.h"


#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qgroupbox.h>
#include <qmainwindow.h>



/**
  * Constants for the timing radio buttons.
  */
const card16 QSpectrumAnalyzerTimings[] = {
   50,100,150,250,350,500,750
};


/**
  * This class is the spectrum display widget for the spectrum analyzer.
  *
  * @short   QSpectrumAnalyzer
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class QSpectrumDisplay : public QWidget
{
   // ====== Constructor/Destructor =========================================
   Q_OBJECT
   public:
   /**
     * Constructor.
     *
     * @param parent Parent widget.
     * @param array Fourier array.
     * @param bars Number of fourier bars.
     * @param drawAverageLine Draw (true) or hide (false) average line.
     */
   QSpectrumDisplay(QWidget*        parent,
                    const cardinal* array,
                    const cardinal  bars,
                    cardinal&       max,
                    const bool      drawAverageLine = true);

   /**
     * Destructor.
     */
   ~QSpectrumDisplay();


   // ====== Qt slots =======================================================
   public slots:
   /**
     * Qt slot: Paint event.
     */
   void paintEvent(QPaintEvent*);

   inline void setDrawAverageLine(const bool drawAverageLine) {
      DrawAverageLine = drawAverageLine;
   }


   // ====== Private data ===================================================
   private:
   void drawBar(QPainter*      painter,
                const cardinal x,
                const cardinal y,
                const cardinal width,
                const cardinal height,
                const cardinal barValue);

   static const cardinal BarColors    = 12;   // Number of colors per bar
   static const cardinal AverageSteps = 10;   // Number of bars per average line

   const cardinal* Array;
   cardinal        Bars;
   cardinal&       Max;
   bool            DrawAverageLine;
};


/**
  * This class is the Qt-Toolkit GUI for the spectrum analyzer.
  *
  * @short   QSpectrumAnalyzer
  * @author  Thomas Dreibholz (dreibh@iem.uni-due.de)
  * @version 1.0
  */
class QSpectrumAnalyzer : public QMainWindow
{
   // ====== Constructor/Destructor =========================================
   Q_OBJECT
   public:
   /**
     * Constructor.
     *
     * @param analyzer SpectrumAnalyzer object.
     * @param parent Parent widget.
     */
   QSpectrumAnalyzer(SpectrumAnalyzer* analyzer,
                     QWidget*          parent = NULL);

   /**
     * Destructor.
     */
   ~QSpectrumAnalyzer();


   // ====== Qt slots =======================================================
   public slots:
   /**
     * Qt slot: Called by QTimer.
     */
   void timerEvent();

   /**
     * Qt slot: Pause displaying the spectrum.
     */
   void pause(bool on);

   /**
     * Qt slot: Reset spectrum analyzer.
     */
   void reset();

   /**
     * Qt slot: Close window.
     */
   void closeWindow();

   /**
     * Qt slot: Change update interval.
     */
   void newInterval(int index);

   /**
     * Qt slot: Change draw average line status.
     */
   void drawAverageLineToggled(int status);


   // ====== Qt signals =====================================================
   signals:
   /**
     * Qt signal: Emitted, when "Close" or window's close button is clicked.
     */
   void closeSpectrumAnalyzer();


   // ====== Private data ===================================================
   private:
   void closeEvent(QCloseEvent* event);

   static const cardinal Bars         = 70;   // Number of bars
   cardinal              ArrayL[Bars];
   cardinal              ArrayR[Bars];

   cardinal              Max;
   QSpectrumDisplay*     PaintWidget1;
   QSpectrumDisplay*     PaintWidget2;
   QCheckBox*            Average;
   QPushButton*          Pause;
   QTimer*               Timer;
   card16                Timing;
   SpectrumAnalyzer*     Analyzer;
};


#endif
