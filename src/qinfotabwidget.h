// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### QInfoTabWidget                                                   ####
// ####                                                                  ####
// ####           Copyright (C) 1999-2024 by Thomas Dreibholz            ####
// ####                                                                  ####
// #### Contact:                                                         ####
// ####    EMail: thomas.dreibholz@gmail.com                             ####
// ####    WWW:   https://www.nntb.no/~dreibh/rtpaudio                   ####
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


#ifndef QINFOTABWIDGET_H
#define QINFOTABWIDGET_H


#include "tdsystem.h"


#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwhatsthis.h>
#include <qlabel.h>
#include <qlist.h>
#include <qhash.h>


/**
  * This structure describes a single info string for QInfoWidget
  * and QInfoTabWidget.
  *
  * @short   InfoEntry
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
struct InfoEntry
{
   /**
     * ID for update() call.
     *
     * @see QInfoWidget#update.
     * @see QInfoTabWidget#update
     */
   const char* ID;

   /**
     * Title of the info string.
     */
   const char* Title;

   /**
     * Help text for the info string.
     */
   const char* Help;
};



/**
  * This structure is a table of InfoEntries for QInfoWidget
  * and QInfoTabWidget.
  *
  * @short   InfoTable
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
struct InfoTable
{
   /**
     * Number of entries in the table.
     */
   const cardinal   Entries;

   /**
     * List of InfoEntries.
     */
   const InfoEntry* Entry;
};



/**
  * This class is a widget for displaying sets of info strings.
  *
  * @short   QInfoWidget
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
class QInfoWidget : public QWidget
{
   Q_OBJECT

   // ====== Constructor ====================================================
   /**
     * Constructor.
     *
     * @param table InfoTable with info string descriptions.
     * @param parent Parent widget.
     */
   public:
   QInfoWidget(const InfoTable* table,
               QWidget*         parent = NULL);


   // ====== Update =========================================================
   /**
     * Update string with given ID.
     *
     * @param id String ID.
     * @param value New value.
     */
   bool update(const QString& id, const QString& value);

   /**
     * Clear all entries.
     */
   void clear();


   // ====== Private data ===================================================
   private:
   const InfoTable*       Table;
   QHash<QString,QLabel*> LabelDict;
};



/**
  * This class is a widget for displaying groups of sets of info strings.
  *
  * @short   QInfoTabWidget
  * @author  Thomas Dreibholz (thomas.dreibholz@gmail.com)
  * @version 1.0
  */
class QInfoTabWidget : public QTabWidget
{
   Q_OBJECT

   // ====== Constructor ====================================================
   /**
     * Constructor.
     *
     * @param table InfoTable with info string descriptions.
     * @param title Tab title.
     * @param pixmapName Name of pixmap for tab.
     * @param parent Parent widget.
     */
   public:
   QInfoTabWidget(const InfoTable* table,
                  const char*      title,
                  const char*      pixmapName = NULL,
                  QWidget*         parent     = NULL);


   // ====== Add InfoTable ==================================================
   /**
     * Add InfoTable to widget.
     *
     * @param table InfoTable with info string descriptions.
     * @param title Tab title.
     * @param pixmapName Name of pixmap for tab.
     */
   QInfoWidget* addTable(const InfoTable* table,
                         const char*      title,
                         const char*      pixmapName = NULL);


   // ====== Update =========================================================
   /**
     * Update string with given ID.
     *
     * @param id String ID.
     * @param value New value.
     */
   bool update(const QString& id, const QString& value);

   /**
     * Clear all entries.
     */
   void clear();


   // ====== Private data ===================================================
   private:
   QList<QInfoWidget*> InfoWidgetList;
};


#endif
