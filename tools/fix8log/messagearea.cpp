#include "messagearea.h"
#include "messagefield.h"

#include <QtWidgets>

MessageArea::MessageArea(QWidget *parent) :
    QWidget(parent)
{
    stackLayout = new QStackedLayout();
    setLayout(stackLayout);
    model = new QStandardItemModel(this);
    QStringList headerLabels;
    headerLabels << "Field" <<  "Value";

    model->setHorizontalHeaderLabels(headerLabels);
    tableView = new QTableView(this); 
    tableView->setModel(model);
    QHeaderView *horHeader = tableView->horizontalHeader();
    horHeader->setSectionResizeMode(QHeaderView::Interactive);
    horHeader->setStretchLastSection(true);
    horHeader->setSectionsMovable(true);
    horHeader->setSortIndicatorShown(true);
    stackLayout->insertWidget(0,tableView);
}
void MessageArea::setMessageFieldList(MessageFieldList *mfl)
{
    model->removeRows(0,model->rowCount());
    if (!mfl || (mfl->count() < 1))
        return;
    QListIterator <MessageField> iter(*mfl);
    while(iter.hasNext()) {
         QList<QStandardItem *> itemList;
        MessageField mf = iter.next();
        QString str1 = QString::number(mf.first);
        QStandardItem *item1 = new QStandardItem(str1);
        QStandardItem *item2 = new QStandardItem("?");
        itemList.append(item1);
        itemList.append(item2);
        model->appendRow(itemList);
    }

}
