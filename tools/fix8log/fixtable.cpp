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

#include "fixHeaderView.h"
#include "fixmimedata.h"
#include "fixtable.h"
#include "globals.h"
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringList>
#include <QStyleOptionViewItemV4>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fix8/f8includes.hpp>
#include <message.hpp>
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
using namespace FIX8;
QString FixTable::headerLabel[] =
{tr("SeqNum"),tr("SenderCompID"),tr("TargetCompID"),tr("SendTime"),
 tr("BeginStr"), tr("BodyLength"),tr("CheckSum"),tr("EncryptMethod"),
 tr("HeartBtInt"),tr("Message Type")};

/*
FixTable::FixTable(const FixTable &ft):QTableView(),dataFile(0)
{
    bgColorStart.setRgb(251,185,6);
    bgColorEnd.setRgb(244,211,122);
    emptyStr1 = tr("No");
    emptyStr2 = tr("File Loaded");
    emptyFont   = font();
    emptyFont.setBold(true);
    emptyFont.setPointSize(emptyFont.pointSize() + 8);
    emptyStrColor.setRgb(239,237,213);

    setMouseTracking(true);
    QStringList strList;
    _model = new QStandardItemModel(this);
    setModel(_model);
    for(int i=0;i<NumColumns;i++) {
        headerItem[i] = new QStandardItem(headerLabel[i]);
        _model->setHorizontalHeaderItem(i,headerItem[i]);
    }
    setShowGrid(true);
    setGridStyle(Qt::SolidLine);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
    //QPalette pal = palette();
    //pal.setColor(QPalette::AlternateBase,QColor(10,40,220,75));
    //setPalette(pal);

    resize(sizeHint());
    QHeaderView *horHeader = horizontalHeader();
    horHeader->setSectionResizeMode(QHeaderView::Interactive);
    horHeader->setStretchLastSection(true);
    horHeader->setSectionsMovable(true);
    horHeader->setSortIndicatorShown(true);
}
*/
FixTable::FixTable(QUuid &wid, QUuid &wsid,QWidget *p):
    QTableView(p),windowID(wid),worksheetID(wsid)

{
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    viewport()->setAcceptDrops(true);
    bgColorStart.setRgb(0,0,114);
    bgColorEnd.setRgb(13,13,15);
    emptyStr1 = tr("No");
    emptyStr2 = tr("Data");
    emptyFont   = font();
    emptyFont.setBold(true);
    emptyFont.setPointSize(emptyFont.pointSize() + 8);
    emptyStrColor.setRgb(239,237,213);
    fixHeader = new FixHeaderView();
    setHorizontalHeader(fixHeader);
    setMouseTracking(true);
    QStringList strList;
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    resize(sizeHint());
}
void FixTable::setWindowID(QUuid &uuid)
{
    windowID = uuid;
}
/******************************************************************/
FixTable::~FixTable()
{
    qDebug() << "Delte Fix Table" << __FILE__ << __LINE__;
}
/******************************************************************/
void FixTable::resizeEvent(QResizeEvent *re)
{
    int offset;
    QFontMetrics fm(emptyFont);
    QSize s = re->size();
    int centerX = s.width();
    int centerY = s.height()/2;

    emptyX1 = (centerX - fm.width(emptyStr1))/2;
    emptyX2 = (centerX - fm.width(emptyStr2))/2;
    offset = fm.height()/2;
    emptyY1 = centerY - offset;
    emptyY2 = centerY + offset;
    center.setX((int) (s.width()/2));
    center.setY((int) (s.height()/2));
    grad.setFocalPoint(center);
    grad.setCenter(center);
    grad.setColorAt(0,bgColorStart);
    grad.setColorAt(1,bgColorEnd);
    grad.setRadius(s.height()*.55);

    QTableView::resizeEvent(re);

}
/******************************************************************/
void  FixTable::mousePressEvent(QMouseEvent *me)
{
    QModelIndex index;
    if (me->button() == Qt::RightButton) {
        index = indexAt(me->pos());
        if (index.isValid()) {
            me->accept();
            emit doPopup(index,me->globalPos());
        }
    }
    else if (me->button() == Qt::LeftButton) {
        Qt::KeyboardModifiers km = me->modifiers();
        if (km && Qt::ControlModifier)
            dragStartPosition = me->pos();
        else
            QTableView::mousePressEvent(me);
    }
    else
        QTableView::mousePressEvent(me);
}
void  FixTable::mouseMoveEvent(QMouseEvent *event)
{
    Qt::KeyboardModifiers km = event->modifiers();
    if (!(event->buttons() & Qt::LeftButton) || !(km && Qt::ControlModifier))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    drag->setPixmap(QPixmap(":/images/svg/spreadsheetCopy.svg"));
    QFileInfo fileInfo("/home/david/hello.txt");
    QUrl url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());

    FixMimeData *fmd = new FixMimeData();
    fmd->windowID = windowID;
    fmd->worksheetID = worksheetID;
    fmd->setUrls(QList<QUrl>() << url);
    //QMimeData *mimeData = new QMimeData;
    fmd->model = (QStandardItemModel *) model();
    drag->setMimeData(fmd);
    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
}
void FixTable::paintEvent(QPaintEvent *pe)
{
    QTableView::paintEvent(pe);
    if (!model()) {
        qWarning() << "no model" << __FILE__;
        return;
    }
    int numRows = model()->rowCount();

    if (numRows < 1) {
        QPainter painter(viewport());
        QBrush brush(grad);
        painter.setBrush(brush);
        QRect r(0,0,width(),height());
        painter.fillRect(r,brush);

        // painter.setFont(emptyFont);
        painter.setFont(emptyFont);
        painter.setPen(emptyStrColor);
        painter.drawText(emptyX1,emptyY1,emptyStr1);
        painter.drawText(emptyX2,emptyY2,emptyStr2);
    }
}
QSize FixTable::sizeHint () const
{
    QSize s(800,700);
    return s;
}
void FixTable::dragEnterEvent(QDragEnterEvent *event)
{
    if ( event->source() == this ) // same table, move entry
    {
        //event->setDropAction( Qt::MoveAction );
        event->ignore();
    }
    else // different table, add entry
    {
        printf("Different table\n");
        event->acceptProposedAction();
    }
}
void FixTable::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
    //event->accept();
}
void FixTable::dropEvent(QDropEvent *event)
{
    FixMimeData *mimeData =  (FixMimeData *) event->mimeData();
    if (mimeData) {
        emit modelDropped(mimeData);
    }
}
