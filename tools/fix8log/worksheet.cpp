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
#include "fixtableverticaheaderview.h"
#include "fixmimedata.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include "tableschema.h"
#include "worksheetmodel.h"
#include <QDebug>
#include <QMap>
#include <QQuickView>
#include <QtWidgets>
#include <iostream>
#include <string.h>
#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
using namespace FIX8;


WorkSheet::WorkSheet(QWidget *parent ) : QWidget(parent),
    senderMenu(0),cancelLoad(false),tableSchema(0),messageList(0),
  currentRow(-1)
{
    build();
    _model = new WorkSheetModel(this);
    fixTable->setWorkSheetModel(_model);
    sm = fixTable->selectionModel();
    fixTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*
    for(int i=0;i<NumColumns;i++) {
        headerItem[i] = new QStandardItem(headerLabel[i]);
        _model->setHorizontalHeaderItem(i,headerItem[i]);
        if (i==WorkSheet::SendingTime)
            headerItem[i]->setToolTip("Right click to select time format");
    }

    dateTimeDelegate = new DateTimeDelegate(this);
    fixTable->setItemDelegateForColumn(FixTable::SendingTime,
                                       dateTimeDelegate);
*/
    FixHeaderView *fixHeader = qobject_cast <FixHeaderView *> (fixTable->horizontalHeader());
    connect(fixHeader,SIGNAL(doPopup(int,QPoint)),
            this,SLOT(popupHeaderMenuSlot(int,const QPoint &)));
    fixHeader->setSectionResizeMode(QHeaderView::Interactive);
    fixHeader->setStretchLastSection(true);
    fixHeader->setSectionsMovable(true);
    fixHeader->setSortIndicatorShown(true);
    setTimeFormat(GUI::Globals::timeFormat);

}
bool WorkSheet::copyFrom(WorkSheet &oldws)
{
    QString qstr;
    WorkSheetModel *oldModel;
    oldModel = oldws.getModel();
    if (oldModel) {
        showLoadProcess(true,oldModel->rowCount());
        _model = oldModel->clone(cancelLoad);
        if(!_model || cancelLoad) {
            //emit terminateCopy(this);
            showLoadProcess(false);
            return false;
        }
        fixTable->setWorkSheetModel(_model);
    }
    fixFileName = oldws.getFileName();
    tableSchema = oldws.tableSchema;
    setTableSchema(tableSchema);
    QString str = oldws.getFileName();
    QFileInfo fi(str);
    senderMenu = new QMenu(this);
    senderMenu->setTitle(fi.fileName());
    if (!oldws.senderActionGroup) {
        fixTable->setWorkSheetModel(_model);
        showLoadProcess(false);
        return false;
    }
    QList<QAction *> oldList = oldws.senderActionGroup->actions();
    QListIterator <QAction *> aiter(oldList);
    QAction *oldSelectAllA = oldws.showAllSendersA;
    QAction *oldA;
    while(aiter.hasNext()) {
        oldA = aiter.next();
        if (oldA != oldSelectAllA) {
            QAction *action = new QAction(oldA->text(),this);
            action->setCheckable(true);
            action->setChecked(oldA->isChecked());
            senderActionGroup->addAction(action);
            senderMenu->addAction(action);
        }
    }
    showAllSendersA = new QAction("Show All",this);
    senderActionGroup->addAction(showAllSendersA);
    senderMenu->addAction(showAllSendersA);
    WorkSheetData wsd = oldws.getWorksheetData();
    setWorkSheetData(wsd);
    showLoadProcess(false);
    return true;
}
WorkSheet::WorkSheet(WorkSheetModel *model,
                     const WorkSheetData &wsd,QWidget *parent):
    QWidget(parent),
    senderMenu(0),cancelLoad(false),origWSD(wsd),tableSchema(0),messageList(0),
    currentRow(-1)

{
    build();
    _model = model;
    fixTable->setWorkSheetModel(_model);
    //dateTimeDelegate = new DateTimeDelegate(this);
    //fixTable->setItemDelegateForColumn(FixTable::SendingTime,
    //                                    dateTimeDelegate);
    fixTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(fixTable,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    /*
    for(int i=0;i<NumColumns;i++) {
        headerItem[i] = new QStandardItem(headerLabel[i]);
        if (i==WorkSheet::SendingTime)
            headerItem[i]->setToolTip("Right click to select time format");
    }
    */
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
    QByteArray ba = wsd.messageHeaderState;
    messageArea->setHeaderState(ba);
    messageArea->setExpansion(wsd.fieldsExpansionType);
    //QModelIndex mi = _model->index(selectedRow,0);
    qWarning() << "REWORK THIS MsgSeqNum" << __FILE__ << __LINE__;
    /*
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
    */
}
WorkSheet::~WorkSheet()
{
    qDebug() << "Delete Work SHeet" << __FILE__ << __LINE__;
    if (_model) {
        _model->clear();
        delete _model;
    }
    if (messageList)
        qDeleteAll(*messageList);
    if (senderMenu) {
        delete senderMenu;
        senderMenu = 0;
    }
    if (senderActionGroup) {
        qDebug() << "DELETE ACTION GROUP " << __FILE__ << __LINE__;
        QList <QAction *> actionList = senderActionGroup->actions();
        qDeleteAll(actionList);
        delete senderActionGroup;
        senderActionGroup = 0;
    }
}
bool WorkSheet::loadCanceled()
{
    return cancelLoad;
}
TableSchema * WorkSheet::getTableSchema()
{
    return tableSchema;
}

void WorkSheet::setTableSchema(TableSchema *ts)
{
    tableSchema = ts;
    _model->clear();
    QHeaderView *horHeader =  fixTable->horizontalHeader();
    QAbstractItemModel * headerModel = horHeader->model();
    headerModel->removeColumns(0,headerModel->columnCount());
    _model->setTableSchema(*tableSchema);
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
    fixTable->setAnouncement("Schema Set To: " + tableSchema->name);
}
/* don't need this method anymore done by worksheetmodel */
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
    senderActionGroup = new QActionGroup(this);
    senderActionGroup->setExclusive(false);
    connect(senderActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(senderActionGroupSlot(QAction *)));
    cancelReason = OK;
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
    else {
        connect(qmlObject,SIGNAL(cancel()),this,SLOT(cancelLoadSlot()));
    }
    splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setObjectName("messageSplitter");
    fixTable = new FixTable(windowID,uuid,this);
    connect(fixTable,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    connect(fixTable,SIGNAL(doPopup(const QModelIndex &,const QPoint &)),this,SLOT(doPopupSlot(const QModelIndex &,const QPoint &)));
    connect(fixTable,SIGNAL(modelDropped(FixMimeData*)),
            this,SLOT(modelDroppedSlot(FixMimeData *)));
    messageArea = new MessageArea(this);
    splitter->addWidget(fixTable);
    splitter->addWidget(messageArea);
    stackLayout->insertWidget(0,splitter);
    stackLayout->insertWidget(1,progressWidget);
    stackLayout->setCurrentIndex(0);
}
QUuid WorkSheet::getID()
{
    return uuid;
}
QMessageList *WorkSheet::getMessageList()
{
    QMessageList *ml = 0;
    if (_model)
        ml = _model->getMessageList();
    return ml;
}
void WorkSheet::terminate()
{
    // in case loadFile is going on
    showLoadProcess(false);
    cancelLoad = true;
    cancelReason = TERMINATED;
    senderMenu = 0;
}
/* THis method not used anymore */
bool WorkSheet::loadFileName(QString &fileName,
                             QList <GUI::ConsoleMessage> &msgList,
                             quint32 &returnCode)
{
    fixFileName = fileName;
    QFileInfo fi(fixFileName);
    bool bstatus;
    msg_type mt;
    msg_seq_num snum;
    QString str;
    QString errorStr;
    QString name;
    TEX::SenderCompID scID;
    QString qstr;
    QFile dataFile(fileName);
    cancelReason = OK; // clear cancel reason

    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,5);
    bstatus =  dataFile.open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::ConsoleMessage message(tr("Failed to open file: ") + fileName);
        msgList.append(message);
        _model->clear();
        returnCode = OPEN_FAILED;
        return false;
    }
    QByteArray ba;
    int linecount=0;
    while(!dataFile.atEnd()) {
        ba = dataFile.readLine();
        linecount++;
    }
    showLoadProcess(true,linecount);
    dataFile.seek(0);
    int i=0;
    QElapsedTimer myTimer;
    int nMilliseconds;
    qint32 fileSize = dataFile.size();


    myTimer.start();
    QMap <QString, qint32> senderMap; // <sender id, numofoccurances>
    messageList = new QMessageList();
    while(!dataFile.atEnd()) {
        if (cancelLoad) {
            dataFile.close();
            if (cancelReason == TERMINATED)  // set from terminate
                returnCode = TERMINATED;
            return false;
        }
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
            if (i%100 == 0) { // every 100 iterations allow gui to process events
                if (cancelLoad) {
                    showLoadProcess(false);
                    returnCode = CANCEL;
                    return false;
                }
                qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,3);
            }
            if (senderMap.contains(sid)) {
                qint32 numOfTimes = senderMap.value(sid);
                numOfTimes++;
                senderMap.insert(sid,numOfTimes);
            }
            else {
                senderMap.insert(sid,1);
            }
            //qDebug() << "SEQ NUM = " << snum()  << "sid = " << sid << __FILE__ << __LINE__;
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
    QList<qint32> valueList = senderMap.values();
    qSort(valueList.begin(), valueList.end(),qGreater<int>());

    QListIterator <qint32> valueIter(valueList);
    QStringList keyList;
    while(valueIter.hasNext()) {
        qint32 value = valueIter.next();
        QMap<QString, qint32>::const_iterator imap = senderMap.constBegin();
        while (imap != senderMap.constEnd()) {
            if (imap.value() == value) {
                keyList.append(imap.key());
                break;
            }
            ++imap;
        }
    }

    int k = 0;
    if (senderMenu) {
        senderMenu->clear();
    }
    else
        senderMenu = new QMenu(this);
    // only support up to 6 colors for filtering by senderID
    if (valueList.count() > 1) {
        int maxItems = valueList.count();
        if (maxItems >7)
            maxItems = 7;
        for( int i=0; i<maxItems; i++) {
            QString key = keyList.at(i);
            if (i!=0) // first one is special, it shows up the most, so it has no color assigned to it
                messageList->senderColorMap.insert(key,QMessageList::senderColors[i-1]);
            QAction *action = new QAction(key,this);
            action->setCheckable(true);
            action->setChecked(true);
            senderActionGroup->addAction(action);

            senderMenu->addAction(action);
        }
    }
    showAllSendersA = new QAction("Show All",this);
    senderActionGroup->addAction(showAllSendersA);
    senderMenu->addAction(showAllSendersA);
    _model->setMessageList(messageList,cancelLoad);
    qstr = QString::number(_model->rowCount()) + tr(" Messages were read from file: ") + fileName;
    msgList.append(GUI::ConsoleMessage(qstr));
    showLoadProcess(false);
    returnCode = OK;
    return true;
}
void WorkSheet::senderActionGroupSlot(QAction *action)
{
    bool    isChecked;
    QAction *act;
    QString str;
    QStringList filterList;
    QList<QAction *> actions =  senderActionGroup->actions();
    QListIterator <QAction *> iter(actions);
    if (action == showAllSendersA) {
        if (actions.count() < 1) {
            fixTable->setSenderIDFilter(filterList);
            return;
        }
        while(iter.hasNext()) {
            act = iter.next();
            if (act != showAllSendersA)
                act->setChecked(true);
        }
    }
    else {
        while(iter.hasNext()) {
            act = iter.next();
            if (!act->isChecked() && act != showAllSendersA)
                filterList << act->text();
        }
    }
    fixTable->setSenderIDFilter(filterList);
    GUI::ConsoleMessage msg;
    if (filterList.count() >= (actions.count() -1))
        msg = GUI::ConsoleMessage("All Sender Types Filtered Out !",GUI::ConsoleMessage::WarningMsg);
    else if (filterList.count() > 0)
        msg = GUI::ConsoleMessage("Filtering out messages with these Sender IDs:" + filterList.join(','));
    else
        msg = GUI::ConsoleMessage("Turned off filtering by sender id");
    emit sendMessage(msg);

}
WorkSheetModel *WorkSheet::getModel()
{
    return _model;
}
QString WorkSheet::getFileName()
{
    return  fixFileName;
}
QMenu * WorkSheet::getSenderMenu()
{
    return senderMenu;
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
    QModelIndex mi2 = sm->currentIndex();
    currentRow = mi2.row();
    qDebug() << "SM CURRENT INDEX = " << mi2.row();
    QMessage *message;
    int row = mi.row();
    if (!_model) {
        qWarning() << "ERROR - MODEL IS NULL" << __FILE__ << __LINE__;
        emit rowSelected(currentRow);
        return;
    }
    for (int i=0;i<  _model->columnCount();i++) {
        QModelIndex otherIndex = _model->index(row,0);
        if (otherIndex.isValid()) {
            QVariant var = _model->data(otherIndex,Qt::UserRole + 1);
            if (var.isValid()) {
                message = (QMessage *) var.value<void *>();
                messageArea->setMessage(message);
                emit rowSelected(currentRow);
                return;
            }
        }
    }
    messageArea->setMessage(0);
    emit rowSelected(currentRow);

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
    wsd.headerExpanded = messageArea->getExpansionState(MessageArea::HeaderItem);
    wsd.fieldsExpanded = messageArea->getExpansionState(MessageArea::FieldsItem);
    wsd.trailerExpanded = messageArea->getExpansionState(MessageArea::TrailerItem);
    wsd.messageHeaderState = messageArea->getHeaderState();
    wsd.fieldsExpansionType = messageArea->getExpansion();
    wsd.searchStr = searchString;


    return wsd;
}
void WorkSheet::cancelLoadSlot()
{
    cancelLoad = true;
    stackLayout->setCurrentIndex(0);
}
void WorkSheet::showLoadProcess(bool isBeingLoaded, int numRecords)
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
            str2 = "(Number of records: " + QString::number(numRecords)  +")";
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
    qWarning() << "FIX OR REMOVE THIS POPUP MENU" << __FILE__ << __LINE__;
    /*
    if (col == FixTable::SendingTime) {
        timeFormatMenu->popup(pt);
    }
    */
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
void WorkSheet::setMessageAreaExpansion(MessageArea::TreeItem ti, bool expanded)
{
    messageArea->setItemExpaned(ti,expanded);
}

bool WorkSheet::getMessageExpansionState(MessageArea::TreeItem ti)
{
    return messageArea->getExpansionState(ti);
}
void WorkSheet::doPopupSlot(const QModelIndex &mi,const QPoint &pt)
{
    qDebug() << "Work Sheet do popup" << __FILE__ << __LINE__;
    emit doPopup(mi,pt);
}
void WorkSheet::setWorkSheetData(const WorkSheetData &wsd)
{
   splitter->restoreState(wsd.splitterState);
   fixTable->horizontalHeader()->restoreState(wsd.headerState);
   QByteArray ba = wsd.messageHeaderState;
   messageArea->setHeaderState(ba);
   messageArea->setExpansion(wsd.fieldsExpansionType);
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
