// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QInfoTabWidget Implementation                                    ####
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


#include "tdsystem.h"
#include "qinfotabwidget.h"


#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qlist.h>


// ###### Constructor #######################################################
QInfoWidget::QInfoWidget(const InfoTable* table,
                         QWidget*         parent)
   : QWidget(parent)
{
   Table = table;

   // ====== Initialize layout ==============================================
   QGridLayout* layout = new QGridLayout(this);
   Q_CHECK_PTR(layout);
   layout->setColumnStretch(1,10);

   // ====== Create info table ==============================================
   for(cardinal i = 0;i < Table->Entries;i++) {
      QLabel* label = new QLabel(QString(Table->Entry[i].Title) + ": ",this);
      Q_CHECK_PTR(label);
      label->setMinimumSize(label->sizeHint());
      layout->addWidget(label,i,0);
      label->setWhatsThis(Table->Entry[i].Help);

      QLabel* value = new QLabel("--- N/A ---",this);
      Q_CHECK_PTR(value);
      value->setMinimumSize(value->sizeHint());
      layout->addWidget(value,i,1);
      value->setWhatsThis(Table->Entry[i].Help);

      LabelDict.insert(Table->Entry[i].ID,value);
   }
}


// ###### Update ############################################################
bool QInfoWidget::update(const QString& id, const QString& value)
{
   QHash<QString,QLabel*>::iterator found = LabelDict.find(id);
   if(found != LabelDict.end()) {
      (*found)->setText(value);
      return(true);
   }
   return(false);
}


// ###### Clear all entries #################################################
void QInfoWidget::clear()
{
   for(QHash<QString,QLabel*>::iterator iterator = LabelDict.begin();
       iterator != LabelDict.end(); ++iterator) {
      (*iterator)->setText("N/A");
   }
}


// ###### Constructor #######################################################
QInfoTabWidget::QInfoTabWidget(const InfoTable* table,
                               const char*      title,
                               const char*      pixmapName,
                               QWidget*         parent)
   : QTabWidget(parent)
{
   addTable(table,title,pixmapName);
}


// ###### Add InfoTable #####################################################
QInfoWidget* QInfoTabWidget::addTable(const InfoTable* table,
                                      const char*      title,
                                      const char*      pixmapName)
{
   QInfoWidget* infoWidget = new QInfoWidget(table,this);
   Q_CHECK_PTR(infoWidget);
   addTab(infoWidget,QPixmap(pixmapName),title);
   InfoWidgetList.append(infoWidget);
   return(infoWidget);
}


// ###### Update ############################################################
bool QInfoTabWidget::update(const QString& id, const QString& value)
{
   cardinal count = 0;

   for(QList<QInfoWidget*>::iterator iterator = InfoWidgetList.begin();
       iterator != InfoWidgetList.end(); ++iterator) {
      QInfoWidget* infoWidget = *iterator;
      if(infoWidget->update(id,value)) {
         count++;
      }
   }
   if(count == 0) {
      std::cerr << "QInfoTabWidget::update() - Unknown ID " << id.toUtf8().constData() << "!" << std::endl;
      return(false);
   }
   return(true);
}


// ###### Clear all entries #################################################
void QInfoTabWidget::clear()
{
   for(QList<QInfoWidget*>::iterator iterator = InfoWidgetList.begin();
       iterator != InfoWidgetList.end(); ++iterator) {
      QInfoWidget* infoWidget = *iterator;
      infoWidget->clear();
   }
}
