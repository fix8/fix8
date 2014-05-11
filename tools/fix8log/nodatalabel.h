#ifndef NODATALABEL_H
#define NODATALABEL_H

#include <QLabel>
class QStandardItemModel;
class FixMimeData;
class NoDataLabel : public QLabel
{
    Q_OBJECT
public:
    explicit NoDataLabel(QString text,QWidget *parent = 0);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
signals:
  void modelDropped(FixMimeData  *);
};

#endif // NODATALABEL_H
