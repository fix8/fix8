/********************************************************************
File		NameDelegte.h
Descript:	USed to paint icon in Name column of ModelView

Protject:	ClassMaster

Author:		David N Boosalis
Date:		Jan 4, 2011 Copied from StateDelegate
*******************************************************************/

#ifndef DATE_TIME_DELEGATE_H
#define DATE_TIME_DELEGATE_H

#include <QStyledItemDelegate>
#include "globals.h"
class QPixmap;
/*! \brief Used solely to paint DateTime column with user selected format


*/

class DateTimeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
  DateTimeDelegate(QObject *parent = 0);
  static void setTimeFormat(GUI::Globals::TimeFormat);
  QSize	sizeHint ( const QStyleOptionViewItem & option, 
		   const QModelIndex & index ); 
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
   static GUI::Globals::TimeFormat timeFormat;
};

#endif 
