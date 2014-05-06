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
