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
HOLDE
*/
//-------------------------------------------------------------------------------------------------
#include "worksheetmodel.h"
#include "intItem.h"
#include <fix8/f8includes.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"
using namespace FIX8;
#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QList>

int WorkSheetModel::senderIDRole = Qt::UserRole+2;

WorkSheetModel::WorkSheetModel(QObject *parent) :
    QStandardItemModel(parent),tableSchema(0),messageList(0)
{

}
// cancel load happens from GUI, need to check here for long operations
WorkSheetModel * WorkSheetModel::clone(const bool &cancelLoad)
{
    WorkSheetModel *wsm = new WorkSheetModel();
    if (tableSchema)
        wsm->setTableSchema(*tableSchema);
    if (messageList) {
        qDebug() << "1 SET MESSAGE LIST COUNT = " << messageList->count()  << __FILE__ << __LINE__;
        QMessageList *newMessageList = messageList->clone(cancelLoad);
        qDebug() << "2 SET MESSAGE LIST COUNT = " << messageList->count()  << __FILE__ << __LINE__;
        if (cancelLoad) {
            wsm->deleteLater();
            wsm = 0;
            return wsm;
        }
        if(newMessageList)
            qDebug() << "\tNEW MESSAGE LIST COUNT " << newMessageList->count() << __FILE__ << __LINE__;
        else
            qDebug() << "\tNEW MESSAGE LIST IS NULL" << __FILE__ << __LINE__;
        wsm->setMessageList(newMessageList,cancelLoad);
    }
    if(cancelLoad) {  // gui set this to cancel
        qDebug() << "HAVE CANCEL LOAD" << __FILE__ << __LINE__;
        wsm->deleteLater();
        wsm = 0;
    }
    return wsm;
}

void WorkSheetModel::setTableSchema(TableSchema &ts)
{
    //TraitHelper tr;
    tableSchema = &ts;
    QStandardItem *hi;
    QBaseEntryList *fieldList;
    QBaseEntry *field;
    clear();
    if (!tableSchema->fieldList) {
        qWarning() << "Field List is null" << __FILE__ << __LINE__;
        setColumnCount(0);
        return;
    }
    if (tableSchema->fieldList->count() < 1)
        qWarning() << "Field List is null" << __FILE__ << __LINE__;
    setColumnCount(tableSchema->fieldList->count());
    fieldList = tableSchema->fieldList;
    QListIterator <QBaseEntry *> iter(*fieldList);
    int i = 0;
    while(iter.hasNext()) {
        field = iter.next();
        //qDebug() << ">>>>>>>>>" << field->ft->_fnum;
        hi = new QStandardItem(field->name);
        setHorizontalHeaderItem(i,hi);
        i++;
    }
    bool fakeCancel = false;
    if (messageList && messageList->count() > 0)
        generateData(fakeCancel);
}
void WorkSheetModel::setMessageList( QMessageList *ml,const bool &cancelLoad)
{
    messageList = ml;
    removeRows(0,rowCount());
    if (!messageList) {
        qWarning() << "Warning - messagelist == 0" << __FILE__ << __LINE__;
        return;
    }
    generateData(cancelLoad);
}
QMessageList *WorkSheetModel::getMessageList()
{
    return messageList;
}

void WorkSheetModel::generateData(const bool &cancelLoad)
{
    QString name;
    QString mbName;
    QMessage   *qmessage;
    Message    *message;
    MessageBase *header;
    MessageBase *trailer;
    BaseField  *baseField;
    QBaseEntry *tableHeader;
    GroupBase  *groupBase;
    FieldTrait::FieldType ft;
    char c[60];
    int fieldID;
    int rowPos = 0;
    int colPos = 0;
    if (!tableSchema) {
        qWarning() << "Unable to generate data -  table schema is null" << __FILE__ << __LINE__;
        return;
    }
    if (!tableSchema->fieldList) {
        qWarning() << "Unable to generate data -  field list is null" << __FILE__ << __LINE__;
        setColumnCount(0);
        return;
    }
    if (!messageList) {
        qWarning() << "Unable to generate data -  message list is null" << __FILE__ << __LINE__;
        setRowCount(0);
        return;
    }

    setColumnCount(tableSchema->fieldList->count());
    QColor modBGColor; // = QColor(255,214,79,100);
    // This is a list of messages read in from file
    QListIterator <QMessage *> mIter(*messageList);
    // this is the fields user selected that they want displayed
    QListIterator <QBaseEntry *> tableHeaderIter(*(tableSchema->fieldList));
    bool modifyBackgroundColor;
    QElapsedTimer myTimer;
    myTimer.start();
    setRowCount(messageList->count());
    while(mIter.hasNext()) {
        if (rowPos%100 == 0) { // every 100 iterations allow gui to process events
            if (cancelLoad) {
                qDebug() << "CANCEL LOAD IN GENERATE DATA " << __FILE__ << __LINE__;
                return;
            }
            qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
        }
        qmessage = mIter.next();
        QString senderID = qmessage->senderID;

        if (messageList->senderColorMap.contains(qmessage->senderID) ) {
            modBGColor =messageList->senderColorMap.value(qmessage->senderID);
            modifyBackgroundColor = true;
        }
        else
            modifyBackgroundColor = false;
        QVariant var;
        var.setValue((void *) qmessage);
        message  = qmessage->mesg;
        header = message->Header();
        trailer = message->Trailer();
        tableHeaderIter.toFront();
        colPos = 0;
        bool found;
        while(tableHeaderIter.hasNext()) {
            found = false;
            tableHeader = tableHeaderIter.next();
            fieldID = tableHeader->ft->_fnum;
            BaseField *bf = header->get_field(fieldID);
            if (bf) {
                ft =  bf->get_underlying_type();
                memset(c,'\0',60);
                bf->print(c);
                if (FieldTrait::is_int(ft)) {
                    int ival(static_cast<Field<int, 0>*>(bf)->get());
                    // qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                    IntItem *intItem = new IntItem(ival);
                    intItem->setData(senderID,senderIDRole);
                    intItem->setData(var);
                    if (modifyBackgroundColor)
                        intItem->setData(modBGColor, Qt::BackgroundRole);
                    setItem(rowPos,colPos,intItem);
                    found = true;
                }

                else if (FieldTrait::is_float(ft)) {
                    qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                    double fval(static_cast<Field<double, 0>*>(bf)->get());
                    found = true;

                }

                else if (FieldTrait::is_string(ft)) {
                    memset(c,'\0',60);
                    bf->print(c);
                    QLatin1Literal ll(c);
                    QStandardItem *strItem = new QStandardItem(QString(ll));
                    strItem->setData(senderID,senderIDRole);
                    strItem->setData(var);
                    if (modifyBackgroundColor)
                        strItem->setData(modBGColor, Qt::BackgroundRole);
                    setItem(rowPos,colPos,strItem);
                    found = true;

                }
                else if (FieldTrait::is_char(ft)) {
                    QChar ch(static_cast<Field<char, 0>*>(bf)->get());
                    QString cstr = ch.decomposition();
                    QStandardItem *charItem = new QStandardItem(cstr);
                    charItem->setData(senderID,senderIDRole);
                    charItem->setData(var);
                    if (modifyBackgroundColor)
                        charItem->setData(modBGColor, Qt::BackgroundRole);
                    setItem(rowPos,colPos,charItem);
                    found = true;

                }

            }
            BaseField *bfm = message->get_field(fieldID);
            if (bfm) {
                ft =  bfm->get_underlying_type();
                memset(c,'\0',60);
                bfm->print(c);
                if (FieldTrait::is_int(ft)) {
                    int ival(static_cast<Field<int, 0>*>(bfm)->get());
                    //qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                    IntItem *intItem = new IntItem(ival);
                    intItem->setData(senderID,senderIDRole);
                    intItem->setData(var);
                    if (modifyBackgroundColor)
                        intItem->setData(modBGColor, Qt::BackgroundRole);
                    setItem(rowPos,colPos,intItem);
                    found = true;

                }
                else if (FieldTrait::is_float(ft)) {
                    qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                    double fval(static_cast<Field<double, 0>*>(bfm)->get());
                    found = true;
                }

                else if (FieldTrait::is_string(ft)) {
                    memset(c,'\0',60);
                    bfm->print(c);
                    QStandardItem *strItem = new QStandardItem(QLatin1Literal(c));
                    strItem->setData(senderID,senderIDRole);
                    strItem->setData(var);
                    if (modifyBackgroundColor)
                        strItem->setData(modBGColor, Qt::BackgroundRole);
                    setItem(rowPos,colPos,strItem);
                    found = true;
                }
                else if (FieldTrait::is_char(ft)) {
                    QChar ch(static_cast<Field<char, 0>*>(bfm)->get());
                    QString cstr = ch.decomposition();
                    QStandardItem *charItem = new QStandardItem(cstr);
                    charItem->setData(senderID,senderIDRole);
                    charItem->setData(var);
                    if (modifyBackgroundColor)
                        charItem->setData(modBGColor, Qt::BackgroundRole);
                    setItem(rowPos,colPos,charItem);
                    found = true;
                }
            }
            //else
            //   qWarning() << "BASE FIELD = NULL FOR HEADER" << __FILE__ << __LINE__;
            Groups groups = message->get_groups();
            std::map<unsigned short,GroupBase *>::iterator iterGrps;
            for(iterGrps = groups.begin(); iterGrps != groups.end(); iterGrps++) {
                groupBase = iterGrps->second;
                int size = groupBase->size();
                for(int i=0;i<size;i++) {
                    MessageBase *mb = groupBase->get_element(i);
                    mbName = QString::fromStdString(mb->get_msgtype());
                    BaseField *bfg = mb->get_field(fieldID);
                    if (bfg) {
                        ft =  bfg->get_underlying_type();
                        memset(c,'\0',60);
                        bfg->print(c);
                        if (FieldTrait::is_int(ft)) {
                            int ival(static_cast<Field<int, 0>*>(bfg)->get());
                            //qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                            IntItem *intItem = new IntItem(ival);
                            intItem->setData(senderID,senderIDRole);
                            intItem->setData(var);
                            if (modifyBackgroundColor)
                                intItem->setData(modBGColor, Qt::BackgroundRole);
                            setItem(rowPos,colPos,intItem);
                            found = true;
                        }
                        else if (FieldTrait::is_float(ft)) {
                            qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                            double fval(static_cast<Field<double, 0>*>(bfg)->get());
                            found = true;
                        }
                        else if (FieldTrait::is_string(ft)) {
                            memset(c,'\0',60);
                            bfg->print(c);
                            QStandardItem *strItem = new QStandardItem(QLatin1Literal(c));
                            strItem->setData(senderID,senderIDRole);
                            strItem->setData(var);
                            if (modifyBackgroundColor)
                                strItem->setData(modBGColor, Qt::BackgroundRole);
                            setItem(rowPos,colPos,strItem);
                            found = true;
                        }
                        else if (FieldTrait::is_char(ft)) {
                            QChar ch(static_cast<Field<char, 0>*>(bfm)->get());
                            QString cstr = ch.decomposition();
                            QStandardItem *charItem = new QStandardItem(cstr);
                            charItem->setData(senderID,senderIDRole);
                            charItem->setData(var);
                            if (modifyBackgroundColor)
                                charItem->setData(modBGColor, Qt::BackgroundRole);
                            setItem(rowPos,colPos,charItem);
                            found = true;
                        }
                    }
                    //  else
                    //     qWarning() << "BASE FIELD = NULL FOR GROUP" << __FILE__ << __LINE__;
                    // qDebug() << "\t\tHave Message Named: " + mbName;
                }
            }
            if (trailer) {
                BaseField *bft = trailer->get_field(fieldID);
                if (bft) {
                    ft =  bft->get_underlying_type();
                    memset(c,'\0',60);
                    bft->print(c);
                    if (FieldTrait::is_int(ft)) {
                        int ival(static_cast<Field<int, 0>*>(bft)->get());
                        IntItem *intItem = new IntItem(ival);
                        intItem->setData(senderID,senderIDRole);
                        intItem->setData(var);
                        if (modifyBackgroundColor)
                            intItem->setData(modBGColor, Qt::BackgroundRole);
                        setItem(rowPos,colPos,intItem);
                        found = true;
                    }
                    else if (FieldTrait::is_float(ft)) {
                        qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                        double fval(static_cast<Field<double, 0>*>(bft)->get());
                        found = true;
                    }
                    else if (FieldTrait::is_string(ft)) {
                        memset(c,'\0',60);
                        bft->print(c);
                        QStandardItem *strItem = new QStandardItem(QLatin1Literal(c));
                        strItem->setData(senderID,senderIDRole);
                        strItem->setData(var);
                        if (modifyBackgroundColor)
                            strItem->setData(modBGColor, Qt::BackgroundRole);
                        setItem(rowPos,colPos,strItem);
                        found = true;
                    }
                    else if (FieldTrait::is_char(ft)) {
                        QChar ch(static_cast<Field<char, 0>*>(bft)->get());
                        QString cstr = ch.decomposition();
                        QStandardItem *charItem = new QStandardItem(cstr);
                        charItem->setData(senderID,senderIDRole);
                        charItem->setData(var);
                        if (modifyBackgroundColor)
                            charItem->setData(modBGColor, Qt::BackgroundRole);
                        setItem(rowPos,colPos,charItem);
                        found = true;
                    }
                }
                //else
                // qWarning() << "BASE FIELD = NULL FOR TRAILER"  << __FILE__ << __LINE__;
            }
            if (!found) {
                //qDebug() << "**************** NOT FOUND **********************" << colPos << __FILE__ << __LINE__;
                // create a dummmy item, so color of row can be uniform across;
                QStandardItem *dummyItem = new QStandardItem("");
                dummyItem->setData(senderID,senderIDRole);
                dummyItem->setData(var);
                if (modifyBackgroundColor)
                    dummyItem->setData(modBGColor, Qt::BackgroundRole);
                setItem(rowPos,colPos,dummyItem);
            }
            colPos++;
        }
        rowPos++;
    }
    int nMilliseconds = myTimer.elapsed();
    qDebug() << "TIME TO LOAD = " << nMilliseconds;
}
