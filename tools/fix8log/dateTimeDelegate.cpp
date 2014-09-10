//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

/********************************************************************
File		nameDelegte.cpp
Descript:	USed to paint tab  column for ModelView

Protject:	ClassMaster

Author:		David N Boosalis
Date:		Jan 4, 2011 Allrights reserverd by author
*******************************************************************/
#include "dateTimeDelegate.h"
#include <QDateTime>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <dateTimeDelegate.h>

GUI::Globals::TimeFormat DateTimeDelegate::timeFormat =
        GUI::Globals::HHMMSS;
DateTimeDelegate::DateTimeDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{
    configPixmap= QPixmap(":/images/svg/config.svg");

}
void DateTimeDelegate::setTimeFormat(GUI::Globals::TimeFormat tf)
{
    timeFormat = tf;
}

QSize	DateTimeDelegate::sizeHint( const QStyleOptionViewItem & option,
                                    const QModelIndex & index ) const
{
    QSize s = QStyledItemDelegate::sizeHint(option,index);
   return s;
} 

/****************************************************************/
void DateTimeDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QString str;
    //QStyledItemDelegate::paint(painter,option,index);
    painter->save();
    QDateTime dt = index.data(Qt::UserRole + 2).toDateTime();
    switch(timeFormat) {
    case GUI::Globals::DAYMONYRHHMMSS:
        str = dt.toString("ddd-mm-yy hh:mm:ss");
        break;
    case GUI::Globals::DAYMMMHHMMSS:
        str = dt.toString("ddd-mm hh:mm:ss");
        break;
    case GUI::Globals::HHMMSS:
        str = dt.toString("hh:mm:ss");
        break;
    case GUI::Globals::HHMM:
        str = dt.toString("hh:mm");
        break;
    default:
        qWarning() << "DateTimeDeleaget = format unknown" << __FILE__ << __LINE__;
        str = dt.toString("hh:mm:ss");
    }

    QRect rect = option.rect;
    /*
    QRect pixRect;
    pixRect.setX(rect.x()*1.1);
    pixRect.setY(rect.y() + rect.height()*.1);
    pixRect.setHeight(rect.height() *.80);
    pixRect.setWidth(pixRect.height());

    painter->setClipRect(option.rect);
    painter->drawPixmap(pixRect,configPixmap);
    */
    painter->drawText(rect,Qt::AlignVCenter,str);
    painter->restore();
}
