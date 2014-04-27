#ifndef MESSAGEAREA_H
#define MESSAGEAREA_H

#include <QWidget>
class QLabel;
class QStackedLayout;
class QStandardItemModel;
class QTableView;
class MessageFieldList;

class MessageArea : public QWidget
{

public:
    explicit MessageArea(QWidget *parent = 0);
    void setMessageFieldList(MessageFieldList *, int seqNum, QString &msgType);
signals:

public slots:
    QStackedLayout *stackLayout;
    QTableView     *tableView;
    QStandardItemModel *model;
    QWidget *infoArea;
    QLabel  *seqNumV;
    QLabel  *messageTypeV;
};

#endif // MESSAGEAREA_H
