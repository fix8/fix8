#include "messagefield.h"
#include "fix8log.h"
#include "worksheet.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include <QDebug>
#include <QFile>
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

QStandardItemModel *Fix8Log::readLogFile(const QString &fileName,QString &errorStr)
{
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
    QFile dataFile(fileName);
    QList<QStandardItem *> itemList;
    QStandardItemModel *model = new QStandardItemModel();
     model->setColumnCount(WorkSheet::NumColumns);
    QStandardItem *headerItem[WorkSheet::NumColumns];
    for(int i=0;i<WorkSheet::NumColumns;i++) {
        headerItem[i] = new QStandardItem(WorkSheet::headerLabel[i]);
        if (i==WorkSheet::SendingTime)
            headerItem[i]->setToolTip("Right click to select time format");
        model->setHorizontalHeaderItem(i,headerItem[i]);
    }
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,5);

    QList <GUI::Message> msgList;
    bstatus =  dataFile.open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::Message message(tr("Failed to open file: ") + fileName);
        msgList.append(message);
        delete model;
        return 0;
    }
    int i=0;
    QElapsedTimer myTimer;
    int linecount = 0;
    int nMilliseconds;
    qint32 fileSize = dataFile.size();
    QByteArray ba;


    while(!dataFile.atEnd()) {
        ba = dataFile.readLine();
        linecount++;
    }

    dataFile.seek(0);
    model->setRowCount(linecount);
    model->setColumnCount(WorkSheet::NumColumns);
    int colPosition = 0;
    int rowPosition = 0;
    myTimer.start();
    messgeTypeItem = new QStandardItem(qstr);
    while(!dataFile.atEnd()) {
        itemList.clear();
        try {
            ba = dataFile.readLine();
            ba.truncate(ba.size()-1); // strip eol charactor
            std::unique_ptr <Message> msg(Message::factory(TEX::ctx(),ba.data()));
            msg->Header()->get(snum);
            const Presence& pre(msg->get_fp().get_presence());
            MessageFieldList *mlf = new MessageFieldList();
            colPosition = 0;
            for (Fields::const_iterator itr(msg->fields_begin()); itr != msg->fields_end(); ++itr)
            {
                const FieldTrait::FieldType trait(pre.find(itr->first)->_ftype);
                name =
                        QString::fromStdString(TEX::ctx().find_be(itr->first)->_name);
                QVariant var = trait;
                MessageField mf(itr->first,name,var);
                mlf->append(mf);
            }
            if (i%100 == 0) { // every 100 iterations allow gui to process events
                qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,3);
                if (cancelSessionRestore) {
                    qDebug() << "Does clear delete objects - if not have memory leak" << __FILE__ << __LINE__;
                    model->clear();
                    delete model;
                    dataFile.close();
                    return 0;
                }
            }

            QVariant userDataVar;
            userDataVar.setValue((void*)mlf);
            int num = snum();
            seqItem = new IntItem(num);
            model->setItem(rowPosition,0,seqItem);
            colPosition++;
            seqItem->setData(userDataVar);

            bstatus = msg->Header()->get(scID);
            qstr = QString::fromStdString(scID());
            senderItem =  new QStandardItem(qstr);
            senderItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,senderItem);
            colPosition++;

            msg->Header()->get(tcID);
            qstr = QString::fromStdString(tcID());
            targetItem =  new QStandardItem(qstr);
            targetItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,targetItem);
            colPosition++;

            msg->Header()->get(sendTime);
            Tickval tv  = sendTime();
            QDateTime dt = QDateTime::fromTime_t(tv.secs());
            sendTimeItem = new QStandardItem(dt.toString());
            sendTimeItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,sendTimeItem);
            colPosition++;

            msg->Header()->get(beginStr);
            qstr = QString::fromStdString(beginStr());
            beginStrItem = new QStandardItem(qstr);
            beginStrItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,beginStrItem);
            colPosition++;

            msg->Header()->get(bodyLength);
            num = bodyLength();
            bodyLengthItem = new IntItem(num);
            bodyLengthItem->setData(userDataVar);
            bodyLengthItem->setData(0,num);
            model->setItem(rowPosition,colPosition,bodyLengthItem);
            colPosition++;

            msg->Trailer()->get(checkSum);
            num = QString::fromStdString(checkSum()).toInt();
            checkSumItem = new IntItem(num);
            checkSumItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,checkSumItem);
            colPosition++;

            msg->Header()->get(encryptMethod);
            num = encryptMethod();
            encryptMethodItem = new QStandardItem(QString::number(num));
            encryptMethodItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,encryptMethodItem);
            colPosition++;

            msg->Header()->get(heartBeatInt);
            num = heartBeatInt();
            heartBeatIntItem = new IntItem(num);
            heartBeatIntItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,heartBeatIntItem);
            colPosition++;

            mt = msg->get_msgtype();
            qstr = QString::fromStdString(mt());
            messgeTypeItem = new QStandardItem(qstr);
            messgeTypeItem->setData(userDataVar);
            model->setItem(rowPosition,colPosition,messgeTypeItem);
            rowPosition++;
        }
        catch (f8Exception&  e){
            errorStr =  "Error - Invalid data in file: " + fileName + ", on  row: " + QString::number(i);
            qWarning() << "exception, row " << i;
            qWarning() << "Error - " << e.what();
            msgList.append(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
        i++;
    }
    nMilliseconds = myTimer.elapsed();
    qDebug() << "TIME TO LOAD = " << nMilliseconds;
    qstr = QString::number(model->rowCount()) + tr(" Messages were read from file: ") + fileName;
    msgList.append(GUI::Message(qstr));
    dataFile.close();
    return model;
}
