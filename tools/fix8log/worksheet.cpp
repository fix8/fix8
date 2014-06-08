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

#include <memory>
#include "messagefield.h"
#include "worksheet.h"
#include "dateTimeDelegate.h"
#include "fixHeaderView.h"
#include "fixmimedata.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include "tableschema.h"
#include <QDebug>
#include <QQuickView>
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
{tr("SeqNum"),tr("SenderCompID "),tr("TargetCompID"), QString(QChar(0x2935)) + "  SendTime"  ,
 tr("BeginStr"), tr("BodyLength"),tr("CheckSum"),tr("EncryptMethod"),
 tr("HeartBtInt"),tr("Message Type")};


WorkSheet::WorkSheet(QWidget *parent ) : QWidget(parent),cancelLoad(false),linecount(0),tableSchema(0)
{
    build();
    _model = new QStandardItemModel();
    fixTable->setModel(_model);
    fixTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for(int i=0;i<NumColumns;i++) {
        headerItem[i] = new QStandardItem(headerLabel[i]);
        _model->setHorizontalHeaderItem(i,headerItem[i]);
        if (i==WorkSheet::SendingTime)
            headerItem[i]->setToolTip("Right click to select time format");
    }
    dateTimeDelegate = new DateTimeDelegate(this);
    fixTable->setItemDelegateForColumn(FixTable::SendingTime,
                                       dateTimeDelegate);
    FixHeaderView *fixHeader = qobject_cast <FixHeaderView *> (fixTable->horizontalHeader());
    connect(fixHeader,SIGNAL(doPopup(int,QPoint)),
            this,SLOT(popupHeaderMenuSlot(int,const QPoint &)));
    fixHeader->setSectionResizeMode(QHeaderView::Interactive);
    fixHeader->setStretchLastSection(true);
    fixHeader->setSectionsMovable(true);
    fixHeader->setSortIndicatorShown(true);
    setTimeFormat(GUI::Globals::timeFormat);

}
WorkSheet::WorkSheet(WorkSheet &oldws,QWidget *parent):
    QWidget(parent),cancelLoad(false),linecount(0)
{
    build();
    QStandardItemModel *oldModel;
    _model = oldws.getModel();
    fixFileName = oldws.getFileName();
    tableSchema = oldws.tableSchema;
    fixTable->setModel(_model);
    dateTimeDelegate = new DateTimeDelegate(this);
    fixTable->setItemDelegateForColumn(FixTable::SendingTime,
                                       dateTimeDelegate);
    FixHeaderView *fixHeader =  qobject_cast <FixHeaderView *> (fixTable->horizontalHeader());
    connect(fixHeader,SIGNAL(doPopup(int,QPoint)),
            this,SLOT(popupHeaderMenuSlot(int,const QPoint &)));
    fixHeader->setSectionResizeMode(QHeaderView::Interactive);
    fixHeader->setStretchLastSection(true);
    fixHeader->setSectionsMovable(true);
    fixHeader->setSortIndicatorShown(true);
    setTimeFormat(GUI::Globals::timeFormat);
    setTableSchema(tableSchema);

}
WorkSheet::WorkSheet(QStandardItemModel *model,
                     const WorkSheetData &wsd,QWidget *parent):
    QWidget(parent),cancelLoad(false),linecount(0),origWSD(wsd),tableSchema(0)
{
    build();
    _model = model;
    fixTable->setModel(_model);
    dateTimeDelegate = new DateTimeDelegate(this);
    fixTable->setItemDelegateForColumn(FixTable::SendingTime,
                                       dateTimeDelegate);


    fixTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(fixTable,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    for(int i=0;i<NumColumns;i++) {
        headerItem[i] = new QStandardItem(headerLabel[i]);
        if (i==WorkSheet::SendingTime)
            headerItem[i]->setToolTip("Right click to select time format");
    }
    FixHeaderView *fixHeader =  qobject_cast <FixHeaderView *> (fixTable->horizontalHeader());
    connect(fixHeader,SIGNAL(doPopup(int,QPoint)),
            this,SLOT(popupHeaderMenuSlot(int,const QPoint &)));
    fixHeader->setSectionResizeMode(QHeaderView::Interactive);
    fixHeader->setStretchLastSection(true);
    fixHeader->setSectionsMovable(true);
    fixHeader->setSortIndicatorShown(true);
    fixHeader->restoreState(wsd.headerState);
    fixFileName = wsd.fileName;
    int selectedRow = wsd.selectedRow;
    if (selectedRow < 0)
        selectedRow = 1;
    fixTable->selectRow(selectedRow);
    QModelIndex mi = _model->index(selectedRow,0);
    QModelIndex otherIndex = _model->index(selectedRow,MsgSeqNum);
    QVariant var = mi.data(Qt::UserRole+1);
    QString str =  _model->data(otherIndex).toString();
    int seqN = str.toInt();
    MessageFieldList *mfl = (MessageFieldList *) var.value<void *>();
    if (!mfl)
        qWarning() << "Warning No Message Fields Found for row" << __FILE__ << __LINE__;
    otherIndex = _model->index(selectedRow,MessageType);
    str =  _model->data(otherIndex).toString();
    messageArea->setMessageFieldList(mfl,seqN,str);
    // update to get time format displayed
    setTimeFormat(GUI::Globals::timeFormat);
}
void WorkSheet::setTableSchema(TableSchema *ts)
{
    tableSchema = ts;
    _model->clear();
    QHeaderView *horHeader =  fixTable->horizontalHeader();
    QAbstractItemModel * headerModel = horHeader->model();
    headerModel->removeColumns(0,headerModel->columnCount());
    if (!tableSchema) {
        qWarning() << "ERROR - Table Schema IS NULL" << __FILE__ << __LINE__;
        return;
    }
    if (!tableSchema->fieldList) {
        qWarning() << "ERROR - Table Schema Field List IS NULL" << __FILE__ << __LINE__;
        return;
    }
    if (tableSchema->fieldList->count() < 1) {
        qWarning() << "ERROR - Table Schema Field List has 0 fields" << __FILE__ << __LINE__;
        return;
    }
    _model->setColumnCount(tableSchema->fieldList->count());
    buildHeader();
    fixTable->setAnouncement("Schema Set To: " + tableSchema->name);
}
void WorkSheet::buildHeader()
{
    QStandardItem *hi;
    QBaseEntryList *fieldList;
    QBaseEntry *field;
    if (!tableSchema)
        return;
    if (!tableSchema->fieldList)
        return;
    if (tableSchema->fieldList->count() < 1)
        return;

    fieldList = tableSchema->fieldList;
    headerItems.clear();

    QListIterator <QBaseEntry *> iter(*fieldList);
    headerItems.resize(fieldList->count());
    int i = 0;
    while(iter.hasNext()) {
        field = iter.next();
        hi = new QStandardItem(field->name);
        _model->setHorizontalHeaderItem(i,hi);
        i++;
    }
}
void WorkSheet::build()
{
    setAcceptDrops(true);
    uuid = QUuid::createUuid();
    timeFormatMenu = new QMenu(this);
    timeActionGroup = new QActionGroup(this);
    connect(timeActionGroup,SIGNAL(triggered (QAction *)),
            this,SLOT(timeFormatSelectedSlot(QAction *)));
    for(int i = 0;i<GUI::Globals::NumOfTimeFormats;i++) {
        timeFormatActions[i] = new QAction(GUI::Globals::timeFormats[i],this);
        timeFormatActions[i]->setCheckable(true);
        timeActionGroup->addAction(timeFormatActions[i]);
        timeFormatMenu->addAction(timeFormatActions[i]);
    }
    timeFormatActions[GUI::Globals::timeFormat]->setChecked(true);

    stackLayout = new QStackedLayout(this);
    setLayout(stackLayout);
    progressView = new QQuickView(QUrl("qrc:qml/loadProgress.qml"));
    progressView->setResizeMode(QQuickView::SizeRootObjectToView);
    progressWidget = QWidget::createWindowContainer(progressView,this);
    qmlObject = progressView->rootObject();
    if (!qmlObject) {
        qWarning() << "qml root object not found" << __FILE__ << __LINE__ ;
    }
    else
        connect(qmlObject,SIGNAL(cancel()),this,SLOT(cancelLoadSlot()));
    splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setObjectName("messageSplitter");
    fixTable = new FixTable(windowID,uuid,this);
    connect(fixTable,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    connect(fixTable,SIGNAL(modelDropped(FixMimeData*)),
            this,SLOT(modelDroppedSlot(FixMimeData *)));
    messageArea = new MessageArea(this);
    splitter->addWidget(fixTable);
    splitter->addWidget(messageArea);
    stackLayout->insertWidget(0,splitter);
    stackLayout->insertWidget(1,progressWidget);
    stackLayout->setCurrentIndex(0);
}
WorkSheet::~WorkSheet()
{
    //qDebug() << "WokSheet Delete" << __FILE__ << __LINE__;
}
QUuid WorkSheet::getID()
{
    return uuid;
}
bool WorkSheet::loadFileName(QString &fileName,
                             QList <GUI::ConsoleMessage> &msgList,
                             quint32 &returnCode)
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
    QString key;
    QString qstr;
    QString seqNumStr;
    QString senderID;
    QVariant returnedValue;
    stackLayout->setCurrentIndex(1);
    QFile *dataFile = new QFile(fileName);
    QList<QStandardItem *> itemList;
    bstatus =  dataFile->open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::ConsoleMessage message(tr("Failed to open file: ") + fileName);
        msgList.append(message);
        delete dataFile;
        showLoadProcess(false);
        returnCode = FILE_NOT_FOUND;
        return false;
    }
    int i=0;
    //_model = (QStandardItemModel *)fixTable->model();
    QString errorStr;
    //QTime myTimer;
    QElapsedTimer myTimer;
    linecount = 0;

    qint32 fileSize = dataFile->size();
    QByteArray ba;

    // get line count

    while(!dataFile->atEnd()) {
        ba = dataFile->readLine();
        linecount++;
    }
    showLoadProcess(true);
    dataFile->seek(0);
    myTimer.start();
    _model->setRowCount(linecount);
    _model->setColumnCount(NumColumns);
    // MessageItem **messageItems = new MessageItem[NumColumns];
    int colPosition = 0;
    int rowPosition = 0;
    int nMilliseconds = 0;
    //messgeTypeItem = new MessageItem(qstr);
    while(!dataFile->atEnd()) {
        itemList.clear();
        try {
            ba = dataFile->readLine();
            ba.truncate(ba.size()-1); // strip eol charactor
            std::unique_ptr <Message> msg(Message::factory(TEX::ctx(),ba.data()));
            msg->Header()->get(snum);
            const Presence& pre(msg->get_fp().get_presence());
            MessageFieldList *mlf = new MessageFieldList();
            colPosition = 0;
            /*
            for (Fields::const_iterator itr(msg->fields_begin()); itr != msg->fields_end(); ++itr)
            {
                const FieldTrait::FieldType trait(pre.find(itr->first)->_ftype);
                name =
                        QString::fromStdString(TEX::ctx().find_be(itr->first)->_name);

                //key = QString::fromStdString(TEX::ctx()._bme.at(ii)->_key);
                QVariant var = trait;
                //MessageField mf(itr->first,name,var);
                  MessageField mf(itr->first,name,0);
                mlf->append(mf);
            }
            if (i%100 == 0) { // every 100 iterations allow gui to process events
                if (cancelLoad) {
                    showLoadProcess(false);
                    returnCode = CANCEL;
                    return false;
                }
                qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,3);
            }
            QVariant userDataVar;
            userDataVar.setValue((void*)mlf);
            int num = snum();
            seqItem = new IntItem(num);
            _model->setItem(rowPosition,0,seqItem);
            colPosition++;
            seqItem->setData(userDataVar);

            bstatus = msg->Header()->get(scID);
            qstr = QString::fromStdString(scID());
            senderItem =  new QStandardItem(qstr);
            senderItem->setData(userDataVar);

            _model->setItem(rowPosition,colPosition,senderItem);
            colPosition++;

            msg->Header()->get(tcID);
            qstr = QString::fromStdString(tcID());
            targetItem =  new QStandardItem(qstr);
            targetItem->setData(userDataVar);
            _model->setItem(rowPosition,colPosition,targetItem);
            colPosition++;

            msg->Header()->get(sendTime);
            Tickval tv  = sendTime();
            QDateTime dt = QDateTime::fromTime_t(tv.secs());
            sendTimeItem = new QStandardItem(dt.toString());
            sendTimeItem->setData(userDataVar);
            sendTimeItem->setData(dt,Qt::UserRole+2);
            _model->setItem(rowPosition,colPosition,sendTimeItem);
            colPosition++;

            msg->Header()->get(beginStr);
            qstr = QString::fromStdString(beginStr());
            beginStrItem = new QStandardItem(qstr);
            beginStrItem->setData(userDataVar);
            _model->setItem(rowPosition,colPosition,beginStrItem);
            colPosition++;

            msg->Header()->get(bodyLength);
            num = bodyLength();
            bodyLengthItem = new IntItem(num);
            bodyLengthItem->setData(userDataVar);
            bodyLengthItem->setData(0,num);
            _model->setItem(rowPosition,colPosition,bodyLengthItem);
            colPosition++;

            msg->Trailer()->get(checkSum);
            num = QString::fromStdString(checkSum()).toInt();
            checkSumItem = new IntItem(num);
            checkSumItem->setData(userDataVar);
            _model->setItem(rowPosition,colPosition,checkSumItem);
            colPosition++;

            msg->Header()->get(encryptMethod);
            num = encryptMethod();
            encryptMethodItem = new QStandardItem(QString::number(num));
            encryptMethodItem->setData(userDataVar);
            _model->setItem(rowPosition,colPosition,encryptMethodItem);
            colPosition++;

            msg->Header()->get(heartBeatInt);
            num = heartBeatInt();
            heartBeatIntItem = new IntItem(num);
            heartBeatIntItem->setData(userDataVar);
            _model->setItem(rowPosition,colPosition,heartBeatIntItem);
            colPosition++;

            mt = msg->get_msgtype();
            qstr = QString::fromStdString(mt());
            messgeTypeItem = new QStandardItem(qstr);
            messgeTypeItem->setData(userDataVar);
            _model->setItem(rowPosition,colPosition,messgeTypeItem);
            rowPosition++;
            */
        }
        catch (f8Exception&  e){
            errorStr =  "Error - Invalid data in file: " + fileName + ", on  row: " + QString::number(i);
            qWarning() << "exception, row " << i;
            qWarning() << "Error - " << e.what();
            msgList.append(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
        i++;
    }
    nMilliseconds = myTimer.elapsed();
    qDebug() << "TIME TO LOAD (milliseconds) " << nMilliseconds;
    qstr = QString::number(_model->rowCount()) + tr(" Messages were read from file: ") + fileName;
    msgList.append(GUI::ConsoleMessage(qstr));
    showLoadProcess(false);
    returnCode = OK;
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
    QVariant var = mi.data(Qt::UserRole+1);
    int row = mi.row();
    if (!_model) {
        qWarning() << "ERROR - MODEL IS NULL" << __FILE__ << __LINE__;
        return;
    }
    QModelIndex otherIndex = _model->index(row,MsgSeqNum);
    QString str =  _model->data(otherIndex).toString();
    int seqN = str.toInt();
    MessageFieldList *mfl = (MessageFieldList *) var.value<void *>();
    if (!mfl)
        qWarning() << "No Message Fields Found for row" << __FILE__ << __LINE__;
    otherIndex = _model->index(row,MessageType);
    str =  _model->data(otherIndex).toString();
    messageArea->setMessageFieldList(mfl,seqN,str);
}
void WorkSheet::setAlias(QString &str)
{
    alias = str;
}
WorkSheetData WorkSheet::getWorksheetData()
{
    WorkSheetData wsd;
    wsd.fileName = fixFileName;
    QHeaderView *horHeader = fixTable->horizontalHeader();
    wsd.headerState = horHeader->saveState();
    wsd.splitterState = splitter->saveState();
    wsd.tabAlias = alias;
    QItemSelectionModel *select = fixTable->selectionModel();
    if (select && select->hasSelection()) {
        QModelIndexList indexList = select->selectedRows();
        if (indexList.count() > 0) { // always true because hasSelection == true
            QModelIndex index = indexList.first();
            wsd.selectedRow = index.row();
        }
    }
    return wsd;
}
void WorkSheet::cancelLoadSlot()
{
    cancelLoad = true;
}
void WorkSheet::showLoadProcess(bool isBeingLoaded)
{
    QVariant returnedValue;
    QString str1;
    QString str2;
    if (isBeingLoaded) {
        stackLayout->setCurrentIndex(1);
        if (!qmlObject)
            qWarning() << "Failed to set load message, qmlObject = 0"
                       << __FILE__ << __LINE__;
        else {
            str1 = "Loading : " + fixFileName;
            str2 = "(Number of records:" + QString::number(linecount) + ")";
            QMetaObject::invokeMethod (qmlObject, "setLoadFile",
                                       Q_RETURN_ARG(QVariant, returnedValue),
                                       Q_ARG(QVariant,str1),
                                       Q_ARG(QVariant,str2));
        }
    }
    else
        stackLayout->setCurrentIndex(0);
}
void WorkSheet::popupHeaderMenuSlot(int col,const QPoint &pt)
{
    if (col == FixTable::SendingTime) {
        timeFormatMenu->popup(pt);
    }
}
void WorkSheet::timeFormatSelectedSlot(QAction *action)
{
    GUI::Globals::TimeFormat tf = GUI::Globals::HHMMSS;
    if (action == timeFormatActions[GUI::Globals::DAYMONYRHHMMSS])
        tf = GUI::Globals::DAYMONYRHHMMSS;
    else if (action == timeFormatActions[GUI::Globals::DAYMMMHHMMSS])
        tf = GUI::Globals::DAYMMMHHMMSS;
    else if (action == timeFormatActions[GUI::Globals::HHMM])
        tf = GUI::Globals::HHMM;
    emit notifyTimeFormatChanged(tf);
}
void WorkSheet::setTimeFormat(GUI::Globals::TimeFormat tf)
{
    dateTimeDelegate->setTimeFormat(tf);
    fixTable->repaint();
    fixTable->update();
}
void WorkSheet::setWindowID( QUuid &uuid)
{
    windowID = uuid;
    fixTable->setWindowID(windowID);
}
void WorkSheet::modelDroppedSlot(FixMimeData *m)
{
    emit modelDropped(m);
}
WorkSheetList::WorkSheetList(QWidget *parent):QList <WorkSheet *>()
{

}
bool WorkSheetList::removeByID(const QUuid &id)
{
    WorkSheet *ws;
    QList<WorkSheet *>::iterator iter;
    for (iter = this->begin(); iter != this->end();++iter) {
        ws = *iter;
        if (ws->getID() == id) {
            erase(iter);
            return true;
        }
    }
    return false;
}
