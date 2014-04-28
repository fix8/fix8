
#ifndef FIX_HEADER_VIEW_H
#define FIX_HEADER_VIEW_H
#include <QtWidgets>
#include <QtWidgets>


class FixHeaderView : public QHeaderView
{
  Q_OBJECT
 public:
   FixHeaderView(QWidget * parent = 0);
  ~FixHeaderView();
  void setFilterModeOn(bool);
 protected:
  void mousePressEvent(QMouseEvent *);
  void mouseDoubleClickEvent(QMouseEvent *);
  //void resizeEvent(QResizeEvent *e);

 signals:
  void doPopup(int logIndex,const QPoint &);
  void column0DoubleClicked(qint32 logicalIndex,const QPoint &);
 private:
  QStandardItemModel *_model;
  bool filterModeOn;
  QPixmap filterPixmap;
  QIcon   filterIcon;
  QStyleOptionHeader *styleOption;
};
#endif
