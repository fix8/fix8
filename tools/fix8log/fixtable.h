
#ifndef FIX_TABLE_VIEW_H
#define FIX_TABLE_VIEW_H
class QFile;
class QKeyEvent;

#include <QDate>

#include <QMap>
#include <QRadialGradient>
#include <QTableView>
class QHeaderView;
class FixHeaderView;
class QStandardItem;
class QStandardItemModel;



class FixTable: public QTableView {
  Q_OBJECT
 public:
  enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
  FixTable(QWidget * parent = 0);
  //FixTable(const FixTable &);
  ~FixTable();
  static QString headerLabel[NumColumns];
  QSize sizeHint () const;
 protected:
  void mousePressEvent(QMouseEvent *);
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
 signals:
  void doPopup(const QModelIndex &,const QPoint &);
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
  
};
#endif
