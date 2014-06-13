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
#include <fix8/f8includes.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
using namespace FIX8;

#include <QDebug>
#include <QList>
WorkSheetModel::WorkSheetModel(QObject *parent) :
    QStandardItemModel(parent),tableSchema(0),messageList(0)
{
}
void WorkSheetModel::setTableSchema(TableSchema &ts)
{
    //TraitHelper tr;
    tableSchema = &ts;
    QStandardItem *hi;
    QBaseEntryList *fieldList;
    QBaseEntry *field;
    qDebug() << "WORK SHEET SET TABLE SCHEMA " << __FILE__ << __LINE__;
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
    //generateData();
}
void WorkSheetModel::setMessageList( QList <Message *> *ml)
{
    messageList = ml;
    removeRows(0,rowCount());
    if (!messageList) {
        qWarning() << "Warning - messagelist == 0" << __FILE__ << __LINE__;
        return;
    }

    Message *message;
    QListIterator <Message *> iter(*messageList);

    while(iter.hasNext()) {
        message = iter.next();
        QString str = QString::fromStdString(message->get_msgtype());
        //qDebug() << "HAVE MESSAGE TYPE: " << str << __FILE__ << __LINE__;
    }

   // generateData();
}
void WorkSheetModel::generateData()
{
    QString name;
    QString mbName;
    Message    *message;
    MessageBase *header;
    MessageBase *trailer;
    BaseField  *baseField;
    QBaseEntry *tableHeader;
    GroupBase  *groupBase;
    int fieldID;
    if (!tableSchema) {
        qWarning() << "Unable to generate data -  table schema is null" << __FILE__ << __LINE__;
        setColumnCount(0);
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
    // This is a list of messages read in from file
    QListIterator <Message *> mIter(*messageList);
    // this is the fields user selected that they want displayed
    QListIterator <QBaseEntry *> tableHeaderIter(*(tableSchema->fieldList));
    while(mIter.hasNext()) {
        message = mIter.next();
        header = message->Header();
         tableHeaderIter.toFront();
        while(tableHeaderIter.hasNext()) {
           tableHeader = tableHeaderIter.next();
           fieldID = tableHeader->ft->_fnum;
           BaseField *bf = header->get_field(fieldID);
           if (bf) {
               FieldTrait::FieldType ft =  bf->get_underlying_type();
               if (FieldTrait::is_int(ft)) {
                   int ival(static_cast<Field<int, 0>*>(bf)->get());
                   qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
               }
           }
        }
    }

    /*
    while(mIter.hasNext()) {
        message = mIter.next();
        header = message->Header();

        const Presence& preh(header->get_fp().get_presence());
        // for every field in the header, lets see if there are any we want
        int ii = 0;
        for (Fields::const_iterator itr(header->fields_begin()); itr != header->fields_end(); ++itr)
        {
            // rewind list of items we want to front
            tableHeaderIter.toFront();
            const FieldTrait::FieldType trait(preh.find(itr->first)->_ftype);
            name = QString::fromStdString(TEX::ctx().find_be(itr->first)->_name);
            // loop through all fields we want, and see if they are in the given header
            while(tableHeaderIter.hasNext()) {
                tableHeader = tableHeaderIter.next();
                fieldID = tableHeader->ft->_fnum;
                BaseField *bf = header->get_field(fieldID);
                if (bf) {
                    FieldTrait::FieldType ft =  bf->get_underlying_type();
                    if (FieldTrait::is_int(ft)) {
                        int ival(static_cast<Field<int, 0>*>(bf)->get());
                        qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                    }
                }
            }
        }
    }

            Groups groups = message->get_groups();
            //qDebug() << "Num Of Groups = "  << groups.size();
            std::map<unsigned short,GroupBase *>::iterator iterGrps;
            for(iterGrps = groups.begin(); iterGrps != groups.end(); iterGrps++) {
                groupBase = iterGrps->second;
                int size = groupBase->size();
                //qDebug() << "\tHave Group, size = " << size;
                for(int i=0;i<size;i++) {
                    MessageBase *mb = groupBase->get_element(i);
                    mbName = QString::fromStdString(mb->get_msgtype());
                    //qDebug() << "\t\tHave Message Named: " + mbName;
                }

            for(std::vector<MessageBase *>::iterator mbiter = groupBase->_msgs.begin();mbiter!= groupBase->_msgs.end();++mbiter)
                MessageBase *messageBase = *mbiter;
            }


            for (Fields::const_iterator gfi( gfi != groupBase->fields_end(); ++gfi)
                 name = QString::fromStdString(TEX::ctx().find_be(gfi->first)->_name;

            }
            const Presence& pre(message->get_fp().get_presence());
            //qDebug() << "\nNEW MESSAGE STARTS**********************************" << __FILE__ << __LINE__;
            for (Fields::const_iterator itr(message->fields_begin()); itr != message->fields_end(); ++itr)
            {
                const FieldTrait::FieldType trait(pre.find(itr->first)->_ftype);
                name = QString::fromStdString(TEX::ctx().find_be(itr->first)->_name);
                //qDebug() << "\tFIELD TRAIT = " << name;
            }

        while(iter.hasNext()) {
            tableHeader = iter.next();
            fieldID = tableHeader->ft->_fnum;
            qDebug() << "Checking for field: " << fieldID << " name = " << tableHeader->name;
            //qDebug() << "\tNum of Fields in message = " << message;
            //Fields::const_iterator fi = message->get_field(fieldID);
            Fields::const_iterator fi =  message->fields_begin();
            //while(fi != message->fields_end())

            //if (fi == message->fields_end())
             BaseField *bf = message->get_field(fieldID);
             if (bf)
                qDebug() << "******************HAVE FIeld*******************";
            else
                qDebug() << "FIELD NOT FOUND - I always get this.";
        }
    */

}



/*******************************************************************
                      if (FieldTrait::is_char(ft))
                        ; //qDebug()  << "\t\t\tIS CHAR";
                    else if (FieldTrait::is_string(ft))
                        ;//qDebug()  << "\t\t\tIS STRING";
                    else if (FieldTrait::is_int(ft)) {
                      int ival(static_cast<Field<int, 0>*>(bf)->get());
                    }
                    if (FieldTrait::is_float(ft))
                       ; // qDebug()  << "\t\t\tIS FLOAT";
                    else
                        qDebug()  << "Unknown datat type " << __FILE__ << __LINE__;
**************************************************************************/
