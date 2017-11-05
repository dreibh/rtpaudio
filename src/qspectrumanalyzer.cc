// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QSpectrumAnalyzer implementation                                 ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2018 by Thomas Dreibholz            ####
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


#include "tdsystem.h"
#include "qspectrumanalyzer.h"
#include "audiodevice.h"
#include "audiodebug.h"
#include "audioconverter.h"
#include "timedthread.h"


#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qmainwindow.h>



// ###### Constructor #######################################################
QSpectrumDisplay::QSpectrumDisplay(QWidget*        parent,
                                   const cardinal* array,
                                   const cardinal  bars,
                                   cardinal&       max,
                                   const bool      drawAverageLine)
   : QWidget(parent), Max(max)
{
   Array           = array;
   Bars            = bars;
   DrawAverageLine = drawAverageLine;
}


// ###### Destructor ########################################################
QSpectrumDisplay::~QSpectrumDisplay()
{

}


// ###### Paint event slot ##################################################
void QSpectrumDisplay::paintEvent(QPaintEvent*)
{
   QPainter painter;
   const cardinal x = 0;
   const cardinal y = 0;
   const cardinal w = width();
   const cardinal h = height() - 3;

   painter.begin(this);
   painter.fillRect(x,y+h,w,3,QColor(10,10,10));

   // ====== Calculate maximum value ========================================
   cardinal values[Bars];
   Max = (cardinal)(double)(Max * 0.98);
   if(Max < 1) {
      Max = 1;
   }
   for(cardinal i = 0;i < Bars;i++) {
      values[i] = Array[i];
      if(values[i] > Max)
         Max = values[i];
   }

   // ====== Draw Bars ======================================================
   cardinal step = w / Bars;
   cardinal sx   = x;
   for(cardinal i = 0;i < Bars;i++) {
      const cardinal value = (values[i] * h) / Max;
      drawBar(&painter,sx,y,step,h,value);
      sx += step;
   }

   // ====== Draw average line ==============================================
   if(DrawAverageLine) {
      step = w / Bars;
      sx   = x;
      painter.setPen(QPen(QColor(80,80,255),3));
      QPainterPath path;
      for(cardinal i = 0;i < Bars;i += AverageSteps) {
         cardinal sum = values[i];
         cardinal j;
         for(j = 1;j < AverageSteps;j++) {
            if((i + j >= Bars))
               break;
            sum += values[i + j];
         }
         sum /= AverageSteps;

         if(i == 0) {
            path.moveTo(sx,y + h - sum);
         } else {
            path.lineTo(sx,y + h - sum);
         }
         sx += AverageSteps * step;
         if(sx > Bars * step) {
            sx = Bars * step;
         }
         path.lineTo(sx,y + h - sum);
      }
      painter.drawPath(path);
   }

   painter.end();
}


// ###### Draw spectrum bar #################################################
void QSpectrumDisplay::drawBar(QPainter*      painter,
                               const cardinal x,
                               const cardinal y,
                               const cardinal width,
                               const cardinal height,
                               const cardinal barValue)
{
   cardinal value = barValue;
   if(barValue > height)
      value = height;

   // ====== Remove area above bar =========================================
   painter->fillRect(x,y,width,height - value,
                     palette().color(QPalette::Background));

   const cardinal y1 = y + height - value;
   const cardinal h1 = value / BarColors;

   // ====== Draw the bar ===================================================
   cardinal r = 255;
   cardinal g = 0;
   cardinal i;
   for(i = 0;i < BarColors - 1;i++) {
      r -= (255 / BarColors);
      g += (255 / BarColors);
      painter->fillRect(x,y1 + i * h1,width,h1,QColor(r,g,0));
   }
   painter->fillRect(x,y1 + i * h1,width,y + height - (y1 + i * h1),QColor(0,255,0));
}



// ###### Constructor #######################################################
QSpectrumAnalyzer::QSpectrumAnalyzer(SpectrumAnalyzer* analyzer,
                                     QWidget*          parent)
   : QMainWindow(parent)
{
   Analyzer = analyzer;
   Max      = 1;

   // ====== Central widget =================================================
   QWidget* centralWidget = new QWidget(this);
   Q_CHECK_PTR(centralWidget);
   QGridLayout* layout    = new QGridLayout(centralWidget);
   Q_CHECK_PTR(layout);
   layout->setColumnStretch(0,0);
   layout->setColumnStretch(1,10);

   // ====== Control group ==================================================
   QGroupBox* controlGroup = new QGroupBox("Control",centralWidget);
   Q_CHECK_PTR(controlGroup);
   layout->addWidget(controlGroup,0,0);
   QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
   Q_CHECK_PTR(controlLayout);

   Pause = new QPushButton("Pause");
   Q_CHECK_PTR(Pause);
   Pause->setCheckable(true);
   Pause->setChecked(false);
   controlLayout->addWidget(Pause,1,0);
   QObject::connect(Pause,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));

   QPushButton* buttonClose = new QPushButton("Close");
   Q_CHECK_PTR(buttonClose);
   controlLayout->addWidget(buttonClose);
   QObject::connect(buttonClose,SIGNAL(clicked()),this,SLOT(closeWindow()));

   Average = new QCheckBox("Average");
   Q_CHECK_PTR(Average);
   Average->setChecked(true);
   controlLayout->addWidget(Average,2,0);
   QObject::connect(Average,SIGNAL(stateChanged(int)),this,SLOT(drawAverageLineToggled(int)));

   // ====== Timing group ===================================================
   QGroupBox* radioGroupBox = new QGroupBox("Update", controlGroup);
   Q_CHECK_PTR(radioGroupBox);
   QButtonGroup* radioButtonGroup = new QButtonGroup(controlGroup);
   Q_CHECK_PTR(radioButtonGroup);
   controlLayout->addWidget(radioGroupBox,3,0);
   QVBoxLayout* radioLayout = new QVBoxLayout(radioGroupBox);
   Q_CHECK_PTR(radioLayout);

   char str[16];
   for(cardinal i = 0;i < sizeof(QSpectrumAnalyzerTimings) / sizeof(card16);i++) {
      snprintf((char*)&str,sizeof(str),
              "%1.2f s",(double)QSpectrumAnalyzerTimings[i] / 1000.0);
      QRadioButton* radio = new QRadioButton(str,radioGroupBox);
      Q_CHECK_PTR(radio);
      if(i == 1) {
         radio->setChecked(true);
      }
      radioLayout->addWidget(radio);
      radioButtonGroup->addButton(radio,i);
   }
   QObject::connect(radioButtonGroup,SIGNAL(buttonPressed(int)),this,SLOT(newInterval(int)));

   // ====== Analyzer display ===============================================
   QGroupBox* fourierGroup = new QGroupBox("Fast Fourier Spectrum Analyzer",centralWidget);
   Q_CHECK_PTR(fourierGroup);
   layout->addWidget(fourierGroup,0,1);
   QGridLayout* fourierLayout = new QGridLayout(fourierGroup);
   Q_CHECK_PTR(fourierLayout);
   PaintWidget1 = new QSpectrumDisplay(fourierGroup, ArrayL, Bars, Max);
   Q_CHECK_PTR(PaintWidget1);
   PaintWidget1->setMinimumWidth(300);
   PaintWidget1->setMinimumHeight(150);
   fourierLayout->addWidget(PaintWidget1,0,0);
   PaintWidget2 = new QSpectrumDisplay(fourierGroup, ArrayR, Bars, Max);
   Q_CHECK_PTR(PaintWidget2);
   PaintWidget2->setMinimumWidth(300);
   PaintWidget2->setMinimumHeight(150);
   fourierLayout->addWidget(PaintWidget2,1,0);

   setCentralWidget(centralWidget);
   setWindowTitle("Spectrum Analyzer");
   setStatusBar(0);

   Timer = new QTimer(this);
   Q_CHECK_PTR(Timer);
   QObject::connect(Timer,SIGNAL(timeout()),this,SLOT(timerEvent()));
   Timer->start(QSpectrumAnalyzerTimings[1]);
   Timing = QSpectrumAnalyzerTimings[1];

   reset();
}


// ###### Destructor ########################################################
QSpectrumAnalyzer::~QSpectrumAnalyzer()
{
   delete Timer;
   Timer = NULL;
}


// ###### Timer event slot ##################################################
void QSpectrumAnalyzer::timerEvent()
{
   if(Analyzer->getSpectrum((cardinal*)&ArrayL,(cardinal*)&ArrayR,Bars)) {
      update();
   }
}


// ###### Change update interval ############################################
void QSpectrumAnalyzer::newInterval(int index)
{
   if((cardinal)index < sizeof(QSpectrumAnalyzerTimings) / sizeof(card16)) {
      Timing = QSpectrumAnalyzerTimings[index];
      if(Pause->isChecked()) {
         Pause->setChecked(false);
      }
      Timer->setInterval(QSpectrumAnalyzerTimings[index]);
   }
}


// ###### Pause displaying the spectrum #####################################
void QSpectrumAnalyzer::pause(bool on)
{
   if(on) {
      Timer->stop();
   }
   else {
      Timer->start(Timing);
   }
}


// ###### Reset spectrum analyzer ###########################################
void QSpectrumAnalyzer::reset()
{
   Analyzer->sync();
   for(cardinal i = 0;i < Bars;i++) {
      ArrayL[i] = 0;
      ArrayR[i] = 0;
   }
   repaint();
}


// ###### Close window ######################################################
void QSpectrumAnalyzer::closeWindow()
{
   emit closeSpectrumAnalyzer();
}


// ###### Close window ######################################################
void QSpectrumAnalyzer::closeEvent(QCloseEvent* event)
{
   emit closeSpectrumAnalyzer();
}


void QSpectrumAnalyzer::drawAverageLineToggled(int status)
{
   PaintWidget1->setDrawAverageLine(status);
   PaintWidget2->setDrawAverageLine(status);
}
