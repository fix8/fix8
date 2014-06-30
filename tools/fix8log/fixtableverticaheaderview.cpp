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
#include "fixtableverticaheaderview.h"
#include <QDebug>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
FixTableVerticaHeaderView::FixTableVerticaHeaderView(QWidget *parent) :
    QHeaderView(Qt::Vertical,parent),highLightOn(false)
{
    //setClickable(true);
    setSectionsClickable(true);
}
void  FixTableVerticaHeaderView::paintSection(QPainter * painter,
                                              const QRect &rect,
                                              int logicalIndex) const
{
    QStyleOptionHeader option;
    option.position = QStyleOptionHeader::Beginning;
    QHeaderView::paintSection(painter, rect, logicalIndex);
    if (!highLightOn)
        return;
    painter->save();

    initStyleOption( &option );
    painter->restore();
    qint32 ivalue;
    QVectorIterator <qint32> iter(hightlightRows);
    while(iter.hasNext()) {
        ivalue = iter.next();
        if (logicalIndex == ivalue) {
            painter->save();
            QLinearGradient gradient(rect.topLeft(),rect.topRight());
            gradient.setColorAt(0,QColor(40,98,231,100));
            gradient.setColorAt(1,QColor(87,166,231,100));
            painter->fillRect(rect,QBrush(gradient));
            //QPen pen(Qt::white);
            //painter->setPen(pen);
            QRect newRect(rect);
            newRect.setLeft(rect.left()+ 3);
            painter->drawText(newRect,Qt::AlignVCenter|Qt::AlignLeft,QString::number(logicalIndex+1));
            painter->restore();
        }
    }
}
void FixTableVerticaHeaderView::setHighlightList(QVector <qint32>list,bool turnOn)
{
    highLightOn = turnOn;
    hightlightRows = list;
    reset();
    update();
    repaint();
}
void FixTableVerticaHeaderView::turnOnSearchHighLight(bool on)
{
    highLightOn  = on;
    update();
}
/*
void FixTableVerticaHeaderView::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "MOUSE PRESS EVENT" << __FILE__ << __LINE__;
    QHeaderView::mousePressEvent(e);
}
*/
