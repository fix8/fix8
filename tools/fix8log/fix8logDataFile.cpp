//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

#include "messagefield.h"
#include "fix8log.h"
#include "futurereaddata.h"
#include "worksheet.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include "worksheetmodel.h"
#include <QDebug>
#include <QFile>
#include <QtWidgets>
#include <iostream>
#include <string.h>
#include <fix8/f8includes.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"

using namespace FIX8;
/*
WorkSheetModel *Fix8Log::readLogFile(const QString &fileName,QString &errorStr)
{

    bool bstatus;
    msg_type mt;
    //  msg_seq_num snum;
    QString str;
    QString name;
    QMap <QLatin1String, qint32> senderMap; // <sender id, numofoccurances>
    TEX::SenderCompID scID;
    //TEX::TargetCompID tcID;
    TEX::SendingTime  sendTime;
    std::string sstr;
    QString qstr;
    QString seqNumStr;
    QString senderID;
    QFile dataFile(fileName);
    QList<QStandardItem *> itemList;
    WorkSheetModel *model = new WorkSheetModel(this);
    qDebug() << "HERE WE ARE IN READ LOG FILE " << __FILE__ << __LINE__;
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,5);

    QList <GUI::ConsoleMessage> msgList;
    bstatus =  dataFile.open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::ConsoleMessage message(tr("Failed to open file: ") + fileName);
        msgList.append(message);
        delete model;
        return 0;
    }
    int i=0;
    QElapsedTimer myTimer;
    int linecount = 0;
    qint32 fileSize = dataFile.size();
    QByteArray ba;

    while(!dataFile.atEnd()) {
        ba = dataFile.readLine();
        linecount++;
    }

    dataFile.seek(0);
    model->setRowCount(linecount);

    int colPosition = 0;
    int rowPosition = 0;
    myTimer.start();
    QMessageList *messageList = new QMessageList();
    while(!dataFile.atEnd()) {
        itemList.clear();
        try {
            msg_seq_num snum;
            sender_comp_id senderID;
            ba = dataFile.readLine();
            ba.truncate(ba.size()-1);
            Message *msg = Message::factory(TEX::ctx(),ba.data());
            msg->Header()->get(snum);
            msg->Header()->get(senderID);

            char c[60];
            memset(c,'\0',60);
            senderID.print(c);
            QLatin1String sid(c);
            QMessage *qmessage = new QMessage(msg,sid,snum());
            qDebug() << "SEQ NUM = " << snum()  << "sid = " << sid << __FILE__ << __LINE__;
            if (senderMap.contains(sid)) {
                qint32 numOfTimes = senderMap.value(sid);
                numOfTimes++;
                senderMap.insert(sid,numOfTimes);
            }
            else
                senderMap.insert(sid,1);
            messageList->append(qmessage);
        }
        catch (f8Exception&  e){
            errorStr =  "Error - Invalid data in file: " + fileName + ", on  row: " + QString::number(i);
            qWarning() << "exception, row " << i;
            qWarning() << "Error - " << e.what();
            msgList.append(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
        i++;
    }
    bool cancelLoad = false;
    model->setMessageList(messageList,cancelLoad);

   //nMilliseconds = myTimer.elapsed();
    qstr = QString::number(model->rowCount()) + tr(" Messages were read from file: ") + fileName;
    //msgList.append(GUI::Message(qstr));
    dataFile.close();
    return model;
}
*/
void Fix8Log::readFileInAnotherThread(const QString &fileName,QString &errorStr)
{
    QFutureWatcher <FutureReadData *> *watcher = new
            QFutureWatcher <FutureReadData *>();
    connect(watcher,SIGNAL(finished()),this,SLOT(finishedReadingDataFileSlot()));
    QFuture<FutureReadData *> future =

            QtConcurrent::run(readLogFileInThread,fileName,errorStr);
    watcher->setFuture(future);
}
void Fix8Log::finishedReadingDataFileSlot()
{
    qDebug() << "Finishded future reading..";
    QFutureWatcher<FutureReadData *> *watcher =
            (QFutureWatcher<FutureReadData *> *)sender();
    FutureReadData *frd = watcher->result();
}

FutureReadData * readLogFileInThread(const QString &fileName,QString &errorStr)
{
    bool bstatus;
    msg_type mt;
    msg_seq_num snum;
    QString str;
    QString name;

/*
    TEX::CheckSum    checkSum;
    TEX::EncryptMethod encryptMethod;
    TEX::HeartBtInt    heartBeatInt;
    TEX::SenderCompID scID;
    TEX::TargetCompID tcID;
    TEX::SendingTime  sendTime;
    */
    std::string sstr;
    QString qstr;
    QString seqNumStr;
    QString senderID;
    QFile dataFile(fileName);
    QList<QStandardItem *> itemList;
    FutureReadData *frd = new FutureReadData();
    QStandardItemModel *model = new QStandardItemModel();
    qDebug() << "Reqwork this, num of columns..." << __FILE__ << __LINE__;

    QList <GUI::ConsoleMessage> msgList;
    bstatus =  dataFile.open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::ConsoleMessage message("Failed to open file: " + fileName);
        msgList.append(message);
        delete model;
        return frd;
    }
    int i=0;
    QElapsedTimer myTimer;
    int linecount = 0;
    qint32 fileSize = dataFile.size();
    QByteArray ba;
    while(!dataFile.atEnd()) {
        ba = dataFile.readLine();
        linecount++;
    }

    dataFile.seek(0);
    model->setRowCount(linecount);
    qDebug() << "rework this, ..." << __FILE__ << __LINE__;
    // model->setColumnCount(WorkSheet::NumColumns);
    int colPosition = 0;
    int rowPosition = 0;
    myTimer.start();
    qDebug() << "FIX THIS < DATA  GETTING READ IN..." << __FILE__ << __LINE__;
    /*
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
            sendTimeItem->setData(dt,Qt::UserRole+2);
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
            msgList.append(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
        i++;
    }
    */

    qstr = QString::number(model->rowCount()) + " Messages were read from file: " + fileName;
    msgList.append(GUI::ConsoleMessage(qstr));
    dataFile.close();
    return frd;
}
