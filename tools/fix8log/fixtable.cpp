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

