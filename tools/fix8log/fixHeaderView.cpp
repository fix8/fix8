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

#include <QDebug>
#include "fixHeaderView.h"

FixHeaderView::FixHeaderView(QWidget * parent):
  QHeaderView(Qt::Horizontal,parent),startTimeCol(1)
{

  _model = (QStandardItemModel *) model();
  //filterPixmap = QPixmap(":/images/filter.png").scaled(20,20);
  configurePixmap = QPixmap(":/images/svg/config.svg");
  styleOption = new QStyleOptionHeader();
  initStyleOption(styleOption);
  setSectionsClickable(true);
}
FixHeaderView::~FixHeaderView()
{
  //delete styleOtion;
}
void FixHeaderView::setStartTimeCol(int stc)
{
    startTimeCol = stc;
}
/* no worky, cant get pixmap to draw before or after painSection
void FixHeaderView::paintSection(QPainter * painter, const QRect & rect, int index) const
{
    QPixmap pm = configurePixmap.scaledToHeight(rect.height()*.80);
    int x = rect.x() + 2;
    int y = rect.y() + (rect.height() - pm.height())/2;
    if (index == startTimeCol) {
        qDebug() << "\tDRAW CONFIG PM at: " << x << y << pm.height();
        //QHeaderView::paintSection(painter,rect,index);
        painter->drawPixmap(rect,pm);
    }
    else
        QHeaderView::paintSection(painter,rect,index);

}
*/
void FixHeaderView::setFilterModeOn(bool on)
{
  filterModeOn = on;
  update();
}
void FixHeaderView::mousePressEvent(QMouseEvent *me)
{
  QModelIndex index;
  if (me->button() == Qt::RightButton) {
    index = indexAt(me->pos());
    int logicalIndex = logicalIndexAt(me->pos());
    if (logicalIndex >  0) { // want double click for column 0
                               // as it toggles filter

      me->accept();
      emit doPopup(logicalIndex,me->globalPos());
    }
  }
  else
    QHeaderView::mousePressEvent(me);
}

void FixHeaderView::mouseDoubleClickEvent(QMouseEvent *me)
{
  QModelIndex index;
  
  qint32 logicalIndex = logicalIndexAt(me->pos());
  if (me->button() == Qt::RightButton) {
    index = indexAt(me->pos());
    if (logicalIndex ==  0) {
       me->accept();
       emit column0DoubleClicked(logicalIndex,me->globalPos());      
    }
  }
  else
    QHeaderView::mouseDoubleClickEvent(me);
}
