#ifndef MESSAGEAREA_H
#define MESSAGEAREA_H

#include <QWidget>
class QStackedLayout;
class QStandardItemModel;
class QTableView;
class MessageFieldList;

class MessageArea : public QWidget
{

public:
    explicit MessageArea(QWidget *parent = 0);
    void setMessageFieldList(MessageFieldList *);
signals:

public slots:

    QStackedLayout *stackLayout;
    QTableView     *tableView;
    QStandardItemModel *model;


};

#endif // MESSAGEAREA_H
