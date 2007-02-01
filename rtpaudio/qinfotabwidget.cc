// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QInfoTabWidget Implementation                                    ####
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
#include "qinfotabwidget.h"


#include <qapp.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qdict.h>
#include <qlist.h>


// ###### Constructor #######################################################
QInfoWidget::QInfoWidget(const InfoTable* table,
                         QWidget*         parent,
                         const char*      name)
   : QWidget(parent,name)
{
   Table = table;

   // ====== Initialize layout and QWhatsThis ===============================
   QGridLayout* layout = new QGridLayout(this,Table->Entries,10);
   CHECK_PTR(layout);
   layout->setColStretch(1,10);

   WhatsThis = new QWhatsThis(this);
   CHECK_PTR(WhatsThis);

   // ====== Create info table ==============================================
   for(cardinal i = 0;i < Table->Entries;i++) {
      QLabel* label = new QLabel(QString(Table->Entry[i].Title) + ": ",this);
      CHECK_PTR(label);
      label->setMinimumSize(label->sizeHint());
      layout->addWidget(label,i,0);
      WhatsThis->add(label,Table->Entry[i].Help);

      QLabel* value = new QLabel("--- N/A ---",this);
      CHECK_PTR(value);
      value->setMinimumSize(value->sizeHint());
      layout->addWidget(value,i,1);
      WhatsThis->add(value,Table->Entry[i].Help);

      LabelDict.insert(Table->Entry[i].ID,value);
      LabelList.append(value);
   }
}


// ###### Update ############################################################
bool QInfoWidget::update(const QString& id, const QString& value)
{
   QLabel* label = LabelDict.find(id);
   if(label != NULL) {
      label->setText(value);
      return(true);
   }
   return(false);
}


// ###### Clear all entries #################################################
void QInfoWidget::clear()
{
   QLabel* label = LabelList.first();
   while(label != NULL) {
      label->setText("N/A");
      label = LabelList.next();
   }
}


// ###### Constructor #######################################################
QInfoTabWidget::QInfoTabWidget(const InfoTable* table,
                               const char*      title,
                               const char*      pixmapName,
                               QWidget*         parent,
                               const char*      name)
   : QTabWidget(parent,name)
{
   addTable(table,title,pixmapName);
}


// ###### Add InfoTable #####################################################
QInfoWidget* QInfoTabWidget::addTable(const InfoTable* table,
                                      const char*      title,
                                      const char*      pixmapName)
{
   QInfoWidget* infoWidget = new QInfoWidget(table,this);
   CHECK_PTR(infoWidget);
   addTab(infoWidget,QPixmap(pixmapName),title);
   InfoWidgetList.append(infoWidget);
   return(infoWidget);
}


// ###### Update ############################################################
bool QInfoTabWidget::update(const QString& id, const QString& value)
{
   cardinal count = 0;

   QInfoWidget* infoWidget = InfoWidgetList.first();
   while(infoWidget != NULL) {
      if(infoWidget->update(id,value)) {
         count++;
      }
      infoWidget = InfoWidgetList.next();
   }
   if(count == 0) {
      cerr << "QInfoTabWidget::update() - Unknown ID " << id << "!" << endl;
      return(false);
   }
   return(true);
}


// ###### Clear all entries #################################################
void QInfoTabWidget::clear()
{
   QInfoWidget* infoWidget = InfoWidgetList.first();
   while(infoWidget != NULL) {
      infoWidget->clear();
      infoWidget = InfoWidgetList.next();
   }
}
