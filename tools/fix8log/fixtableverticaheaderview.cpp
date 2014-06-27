#include "fixtableverticaheaderview.h"
#include <QDebug>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
FixTableVerticaHeaderView::FixTableVerticaHeaderView(QWidget *parent) :
    QHeaderView(Qt::Vertical,parent),highLightOn(false)
{
}
void  FixTableVerticaHeaderView::paintSection(QPainter * painter,
                                              const QRect &rect,
                                              int logicalIndex) const
{
    painter->save();
    QStyleOptionHeader option;
    initStyleOption( &option );

    option.position = QStyleOptionHeader::Beginning;
    QHeaderView::paintSection(painter, rect, logicalIndex);
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
    qDebug() << "FIXTABLE_VERTICAL HEADER SET LIST:" << list << __FILE__ << __LINE__;
    hightlightRows = list;
    repaint();
    update();
}
void FixTableVerticaHeaderView::turnOnSearchHighLight(bool on)
{
    highLightOn  = on;
    repaint();
}
