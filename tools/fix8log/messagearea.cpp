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
    QWidget *workArea = new QWidget(this);
    QVBoxLayout *wBox = new QVBoxLayout();
    workArea->setLayout(wBox);
    tableView = new QTableView(workArea);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setModel(model);
    tableView->verticalHeader()->setVisible(false);
    QHeaderView *horHeader = tableView->horizontalHeader();
    horHeader->setSectionResizeMode(QHeaderView::Interactive);
    horHeader->setStretchLastSection(true);
    horHeader->setSectionsMovable(true);
    horHeader->setSortIndicatorShown(true);

    infoArea = new QWidget(workArea);
    QFont fnt = infoArea->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize()+2);
    infoArea->setFont(fnt);
    QPalette pal = infoArea->palette();
    QColor fgColor = pal.color(QPalette::WindowText);
    QColor bgColor = pal.color(QPalette::Window);
    pal.setColor(QPalette::WindowText,bgColor);
    pal.setColor(QPalette::Window,fgColor);
    infoArea->setAutoFillBackground(true);
    infoArea->setPalette(pal);
    QFormLayout *infoForm = new QFormLayout();
    infoArea->setLayout(infoForm);
    seqNumV = new QLabel(infoArea);
    seqNumV->setAlignment(Qt::AlignCenter);
    messageTypeV = new QLabel(infoArea);
    messageTypeV->setAlignment(Qt::AlignCenter);
    infoForm->addRow("Seq Num",seqNumV);
    infoForm->addRow("Mesg Type",messageTypeV);

    wBox->addWidget(tableView,1);
    wBox->addWidget(infoArea,0,Qt::AlignBottom);

    stackLayout->insertWidget(0,workArea);
}
void MessageArea::setMessageFieldList(MessageFieldList *mfl,int seqNum, QString &msgType)


{
    seqNumV->setText(QString::number(seqNum));
    messageTypeV->setText(msgType);
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
