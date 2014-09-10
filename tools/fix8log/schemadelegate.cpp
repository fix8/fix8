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
#include "schemadelegate.h"
#include "schemaitem.h"
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>

SchemaDelegate::SchemaDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{
    saveNeededPixmap = QPixmap(":/images/svg/greenPlus.svg").scaledToHeight(24);
    emptyPixmap = QPixmap(":/images/svg/xred.svg").scaledToHeight(24);
}
void SchemaDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    if (!index.isValid()) {
        qWarning() << "Invalid index" << __FILE__ << __LINE__;
        return;
    }
    painter->save();
    painter->setClipRect(option.rect);
    QStyledItemDelegate::paint(painter,option,index);
    QStandardItemModel *m = (QStandardItemModel *) index.model();
    if (!m) {
        qWarning() << "Invalid model item" << __FILE__ << __LINE__;
        return;
    }
    SchemaItem *si = (SchemaItem  *) m->itemFromIndex(index);
    QRect rect = option.rect;
    if (!si) {
        qWarning() << "State item is invalid...." << __FILE__ << __LINE__;
        return;
    }
//    QSize s = QStyledItemDelegate::sizeHint(option,index);
    int x,y;
    if (si->empty) {
        x     = rect.x() +  rect.width()- 5 - emptyPixmap.width();
        y = (rect.y() + rect.height() - emptyPixmap.height())/2;
        painter->drawPixmap(x,y,emptyPixmap);
    }
    else if (si->modified) {
        x     = rect.x() +  rect.width()- 5 -saveNeededPixmap.width() ;
        y = rect.y() + (rect.height() - saveNeededPixmap.height())/2;
        painter->drawPixmap(x,y,saveNeededPixmap);
    }
    painter->restore();
}
QSize SchemaDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option,index);
    s.setHeight(saveNeededPixmap.height()  + (saveNeededPixmap.height()*.12));
    return s;
}

