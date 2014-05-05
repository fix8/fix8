
#ifndef FIX_TABLE_VIEW_H
#define FIX_TABLE_VIEW_H
class QFile;
class QKeyEvent;

#include <QDate>
#include <QMap>
#include <QRadialGradient>
#include <QTableView>
#include <QUuid>
class QHeaderView;
class FixHeaderView;
class FixMimeData;
class QStandardItem;
class QStandardItemModel;



class FixTable: public QTableView {
  Q_OBJECT
 public:
  enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
  FixTable(QUuid &windowID, QUuid &workSheetID,QWidget * parent = 0);
  //FixTable(const FixTable &);
  void setWindowID(QUuid &uuid);
  ~FixTable();
  static QString headerLabel[NumColumns];
  QSize sizeHint () const;
 protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent *event);
  void mousePressEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
 signals:
  void doPopup(const QModelIndex &,const QPoint &);
  void modelDropped(FixMimeData  *);
  //  void doHeaderPopup(GradeHeaderItem *,const QPoint &);
 private:
  QString emptyStr1;
  QString emptyStr2;
  QFont   emptyFont;
  QColor  bgColorStart;
  QColor  bgColorEnd;
  QRadialGradient grad;
  QColor  emptyStrColor;
  int     emptyX1,emptyY1;
  int     emptyX2,emptyY2;
  QPointF center;
  //QStandardItemModel *_model;
  //QStandardItem *headerItem[NumColumns];
  QFile *dataFile;
  FixHeaderView *fixHeader;
  QPoint dragStartPosition;
  QUuid windowID;
  QUuid worksheetID;
};
#endif
