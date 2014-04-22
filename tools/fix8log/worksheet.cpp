#include "messagefield.h"
#include "worksheet.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include <QDebug>
#include <QtWidgets>
#include <iostream>
#include <string.h>
#include <fix8/f8includes.hpp>
#include <field.hpp>

#include <message.hpp>

#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
using namespace FIX8;
QString WorkSheet::headerLabel[] =
{tr("SeqNum"),tr("SenderCompID"),tr("TargetCompID"),tr("SendTime"),
 tr("BeginStr"), tr("BodyLength"),tr("CheckSum"),tr("EncryptMethod"),
 tr("HeartBtInt"),tr("Message Type")};

WorkSheet::WorkSheet(QWidget *parent ) : QWidget(parent)
{
    stackLayout = new QStackedLayout(this);
    setLayout(stackLayout);
    splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setObjectName("messageSplitter");
    fixTable = new FixTable();
    messageArea = new MessageArea(this);
    splitter->addWidget(fixTable);
    splitter->addWidget(messageArea);
    stackLayout->addWidget(splitter);
    _model = new QStandardItemModel();
    fixTable->setModel(_model);
    connect(fixTable,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    for(int i=0;i<NumColumns;i++) {
        headerItem[i] = new QStandardItem(headerLabel[i]);
        _model->setHorizontalHeaderItem(i,headerItem[i]);
    }
    QHeaderView *horHeader = fixTable->horizontalHeader();
    horHeader->setSectionResizeMode(QHeaderView::Interactive);
    horHeader->setStretchLastSection(true);
    horHeader->setSectionsMovable(true);
    horHeader->setSortIndicatorShown(true);

}
WorkSheet::WorkSheet(WorkSheet &oldws,QWidget *parent):QWidget(parent)
{
    stackLayout = new QStackedLayout(this);
    setLayout(stackLayout);
    splitter = new QSplitter(Qt::Horizontal,this);
    fixTable = new FixTable();
    connect(fixTable,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));

    messageArea = new MessageArea(this);
    splitter->addWidget(fixTable);
    splitter->addWidget(messageArea);
    stackLayout->addWidget(splitter);

    QStandardItemModel *oldModel;
    _model = oldws.getModel();
    fixFileName = oldws.getFileName();
    fixTable->setModel(_model);
    QHeaderView *horHeader = fixTable->horizontalHeader();
    horHeader->setSectionResizeMode(QHeaderView::Interactive);
    horHeader->setStretchLastSection(true);
    horHeader->setSectionsMovable(true);
    horHeader->setSortIndicatorShown(true);

    qDebug() << "copy old model to new work sheet" << __FILE__;

}
WorkSheet::~WorkSheet()
{
    qDebug() << "WokSheet Delete" << __FILE__ << __LINE__;
}

bool WorkSheet::loadFileName(QString &fileName,QList <GUI::Message> &msgList)
{
    fixFileName = fileName;
    bool bstatus;
    msg_type mt;
    msg_seq_num snum;
    QString str;
    QString name;
    IntItem *seqItem;
    QStandardItem *senderItem;
    QStandardItem *targetItem;
    QStandardItem *sendTimeItem;
    QStandardItem *beginStrItem;
    IntItem *bodyLengthItem;
    IntItem  *checkSumItem;
    QStandardItem  *encryptMethodItem;
    IntItem  *heartBeatIntItem;
    QStandardItem  *messgeTypeItem;

    TEX::BeginString beginStr;
    TEX::BodyLength  bodyLength;
    TEX::CheckSum    checkSum;
    TEX::EncryptMethod encryptMethod;
    TEX::HeartBtInt    heartBeatInt;
    TEX::SenderCompID scID;
    TEX::TargetCompID tcID;
    TEX::SendingTime  sendTime;
    std::string sstr;
    QString qstr;
    QString seqNumStr;
    QString senderID;
    QFile *dataFile = new QFile(fileName);
    QList<QStandardItem *> itemList;
    bstatus =  dataFile->open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::Message message(tr("Failed to open file: ") + fileName);
        msgList.append(message);
        delete dataFile;
        return false;
    }
    int i=0;
    //_model = (QStandardItemModel *)fixTable->model();
    QString errorStr;
    while(!dataFile->atEnd()) {
        itemList.clear();
        try {
            QByteArray ba = dataFile->readLine();
            ba.truncate(ba.size()-1); // strip eol charactor
            scoped_ptr<Message> msg(Message::factory(TEX::ctx(),ba.data()));
            msg->Header()->get(snum);
            const Presence& pre(msg->get_fp().get_presence());
            MessageFieldList *mlf = new MessageFieldList();
            for (Fields::const_iterator itr(msg->fields_begin()); itr != msg->fields_end(); ++itr)
            {
                //    std::cout << "first: " << itr->first;
                //    std::cout << "  second: " <<*itr->second << std::endl;
                const FieldTrait::FieldType trait(pre.find(itr->first)->_ftype);

                name =
                        QString::fromStdString(TEX::ctx().find_be(itr->first)->_name);
                QVariant var = trait;
                MessageField mf(itr->first,name,var);
                mlf->append(mf);
                //std::string s =  *itr->second;
                // qDebug() << "Second :" << QString::fromStdString(*itr->second.);
                // qDebug() << "Trait Type:" << var.typeName()  << ", " << trait;
                //std::cout << "Trait: " << trait << std::endl;
                //const BaseMsgEntry *bme(TEX::ctx()._bme.find_ptr(itr->first));

            }
            QVariant userDataVar;
            userDataVar.setValue((void*)mlf);
            int num = snum();
            seqItem = new IntItem(num);
            seqItem->setData(userDataVar);
            itemList.append(seqItem);

            bstatus = msg->Header()->get(scID);
            qstr = QString::fromStdString(scID());
            senderItem =  new QStandardItem(qstr);
            senderItem->setData(userDataVar);
            itemList.append(senderItem);

            msg->Header()->get(tcID);
            qstr = QString::fromStdString(tcID());
            targetItem =  new QStandardItem(qstr);
            targetItem->setData(userDataVar);
            itemList.append(targetItem);

            msg->Header()->get(sendTime);
            Tickval tv  = sendTime();
            QDateTime dt = QDateTime::fromTime_t(tv.secs());
            sendTimeItem = new QStandardItem(dt.toString());
            sendTimeItem->setData(userDataVar);
            itemList.append(sendTimeItem);

            msg->Header()->get(beginStr);
            qstr = QString::fromStdString(beginStr());
            beginStrItem = new QStandardItem(qstr);
            beginStrItem->setData(userDataVar);
            itemList.append(beginStrItem);

            msg->Header()->get(bodyLength);
            num = bodyLength();
            bodyLengthItem = new IntItem(num);
            beginStrItem->setData(userDataVar);
            bodyLengthItem->setData(0,num);
            itemList.append(bodyLengthItem);

            msg->Trailer()->get(checkSum);
            num = QString::fromStdString(checkSum()).toInt();
            checkSumItem = new IntItem(num);
            checkSumItem->setData(userDataVar);
            itemList.append(checkSumItem);

            msg->Header()->get(encryptMethod);
            num = encryptMethod();
            encryptMethodItem = new QStandardItem(QString::number(num));
            encryptMethodItem->setData(userDataVar);
            itemList.append(encryptMethodItem);

            msg->Header()->get(heartBeatInt);
            num = heartBeatInt();
            heartBeatIntItem = new IntItem(num);
            heartBeatIntItem->setData(userDataVar);
            itemList.append(heartBeatIntItem);
            mt = msg->get_msgtype();
            qstr = QString::fromStdString(mt());
            messgeTypeItem = new QStandardItem(qstr);
            messgeTypeItem->setData(userDataVar);
            itemList.append(messgeTypeItem);

            _model->appendRow(itemList);
        }
        catch (f8Exception&  e){
            errorStr =  "Error - Invalid data in file: " + fileName + ", on  row: " + QString::number(i);
            qWarning() << "exception, row " << i;
            qWarning() << "Error - " << e.what();
            msgList.append(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
        i++;
    }
    qstr = QString::number(_model->rowCount()) + tr(" Messages were read from file: ") + fileName;
    msgList.append(GUI::Message(qstr));
    return true;
}
QStandardItemModel *WorkSheet::getModel()
{
    return _model;
}
QString WorkSheet::getFileName()
{
    return  fixFileName;
}
void  WorkSheet::hideColumn(int colNum, bool hideCol)
{
    if (!fixTable) {
        qWarning() << "Table is null" << __FILE__ << __LINE__;
        return;
    }
    QHeaderView *horHeader = fixTable->horizontalHeader();
    if (!horHeader) {
        qWarning() << "Horizontal Header is null" << __FILE__ << __LINE__;
        return;
    }
    if(hideCol)
        horHeader->hideSection(colNum);
    else
        horHeader->showSection(colNum);
}
void  WorkSheet::rowSelectedSlot(QModelIndex mi)
{
    QStandardItem *item = (QStandardItem *) _model->itemFromIndex(mi);
    QVariant var = mi.data(Qt::UserRole+1);
    MessageFieldList *mfl = (MessageFieldList *) var.value<void *>();
    messageArea->setMessageFieldList(mfl);
}
