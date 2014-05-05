#include "nodatalabel.h"
#include "fixmimedata.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QStandardItemModel>
NoDataLabel::NoDataLabel(QString text,QWidget *parent) :
    QLabel(text,parent)
{
    setAcceptDrops(true);
}
void NoDataLabel::dragEnterEvent(QDragEnterEvent *event)
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
void NoDataLabel::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
    //event->accept();

}
void NoDataLabel::dropEvent(QDropEvent *event)
{
    qDebug() << "Drop Event " << __FILE__ << __LINE__;
    FixMimeData *mimeData =  (FixMimeData *) event->mimeData();
    if (mimeData) {
            emit modelDropped(mimeData);
        }
}
