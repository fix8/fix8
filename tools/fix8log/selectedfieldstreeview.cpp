#include "selectedfieldstreeview.h"
#include <QResizeEvent>
#include <QPainter>
SelectedFieldsTreeView::SelectedFieldsTreeView(QWidget *parent) :
    QTreeView(parent)
{
    bgColorStart.setRgb(93,133,79,200);
    bgColorEnd.setRgb(71,86,68,150);
    //anounceFG = Qt::black;
    //anounceBG = QColor(255,189,11,180);
    emptyStr1 = tr("No Fields");
    emptyStr2 = tr("Selected");
    emptyFont   = font();
    emptyFont.setBold(true);
    emptyFont.setPointSize(emptyFont.pointSize() + 8);
    // anounceFont.setPointSize(emptyFont.pointSize() -4);
    emptyStrColor.setRgb(239,237,213);
    setAutoFillBackground(false);
    QPalette pal = palette();
    pal.setColor(QPalette::AlternateBase,QColor(10,40,220,75));
    setPalette(pal);
}
void SelectedFieldsTreeView::resizeEvent(QResizeEvent *re)
{
    int offset;
    QFontMetrics fm(emptyFont);
    QSize s = re->size();
    int centerX = s.width()/2;
    int centerY = s.height()/2;

    emptyX1 = centerX - (fm.width(emptyStr1))/2;
    emptyX2 = centerX - (fm.width(emptyStr2))/2;
    offset = fm.height()/2;
    emptyY1 = centerY - offset;
    emptyY2 = centerY + offset;
    center.setX((int) (s.width()/2));
    center.setY((int) (s.height()/2));
    grad.setFocalPoint(center);
    grad.setCenter(center);
    grad.setColorAt(0,bgColorStart);
    grad.setColorAt(1,bgColorEnd);
    grad.setRadius(s.height()*.75);
    setUniformRowHeights(true);

    int xoffset = 30;
    int yoffset = 15;
}
void SelectedFieldsTreeView::paintEvent(QPaintEvent *pe)
{
    QTreeView::paintEvent(pe);

    int numRows = model()->rowCount();

    QPainter painter(viewport());

    if (numRows < 1) {
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
