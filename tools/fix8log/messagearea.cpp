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
    headerLabels << "Field" <<  "Name" << "Value";
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
        QString str1 = QString::number(mf.id);
        // Do we need to delete these items after clear to avoid memory leak ?
        QStandardItem *item1 = new QStandardItem(str1);
        QStandardItem *item2 = new QStandardItem(mf.name);
        QStandardItem *item3 = new QStandardItem();
        if (mf.variant.isValid()) {
            switch (mf.variant.type()) {
            case QVariant::Double:
                item3->setText(QString::number(mf.variant.toDouble()));
                        break;
            case QVariant::String:
                item3->setText(mf.variant.toString());
                break;
            case QVariant::Int:
                item3->setText(QString::number(mf.variant.toInt()));
                        break;
            default:
                qWarning() << "Unknown data type" << __FILE__ << __LINE__;
                item3->setText(mf.variant.toString());
            }
        }
        itemList.append(item1);
        itemList.append(item2);
        itemList.append(item3);
        model->appendRow(itemList);
    }
}
