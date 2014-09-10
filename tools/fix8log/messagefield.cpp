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

#include <QApplication>
#include <QDebug>
FieldUse::FieldUse():isDefault(false)
{

}
FieldUseList::FieldUseList() : QList<FieldUse *> ()
{

}
FieldUse *FieldUseList::findByName(QString &name)
{
    FieldUse *fe = 0;
    QListIterator<FieldUse *>iter(*this);
    while(iter.hasNext()) {
        fe = iter.next();
        if (fe->name == name)
            return fe;
    }
    return 0;
}
BaseEntryList::BaseEntryList():QList <BaseEntry *>()
{

}
BaseEntry *BaseEntryList::findByName(QString &name)
{
    BaseEntry *be = 0;
    QListIterator <BaseEntry *> iter(*this);
    while(iter.hasNext()) {
        be = iter.next();
        if (be->_name == name)
            return be;
    }
    return 0;
}

QBaseEntry::QBaseEntry():baseEntryList(0)
{

}

QBaseEntry::QBaseEntry(const BaseEntry &be):ft(0),baseEntryList(0)
{
    name = QString::fromLatin1(be._name);

}
QBaseEntry::QBaseEntry(const QBaseEntry *qbe)
{
    baseEntryList = 0;
    if (!qbe)
        return;
    name = qbe->name;
    if (qbe->ft)
        ft   = new FieldTrait(*(qbe->ft));
    QBaseEntry *child;
    if (qbe->baseEntryList) {
        baseEntryList = new QBaseEntryList();
        qDebug() << "*** COPY CONSTRUCTEOR QBE ****, coune of list = " << qbe->baseEntryList->count();
        QListIterator<QBaseEntry *> iter(*(qbe->baseEntryList));
        while(iter.hasNext()) {
            child = iter.next();
            baseEntryList->append(new QBaseEntry(*child));
        }
    }
}
QBaseEntry::QBaseEntry(const QBaseEntry &qbe)
{
    name = qbe.name;
    if (qbe.ft)
        ft   = new FieldTrait(*(qbe.ft));
    QBaseEntry *child;
    baseEntryList = qbe.baseEntryList;
    /*
    if (qbe.baseEntryList) {
        baseEntryList = new QBaseEntryList();
        qDebug() << "*** COPY CONSTRUCTEOR QBE ****, coune of list = " << qbe.baseEntryList->count();
        QListIterator<QBaseEntry *> iter(*(qbe.baseEntryList));
        while(iter.hasNext()) {
            child = iter.next();
            QBaseEntry *qbeNew = new QBaseEntry(child);
            baseEntryList->append(qbeNew);
        }
    }
    */
}
void QBaseEntry::print(QString &str)
{
    qDebug() << str << name;
    QString newStr = str;
    newStr.append("\t");
    if (baseEntryList) {
        QListIterator <QBaseEntry *> iter(*baseEntryList);
        while (iter.hasNext()) {
            QBaseEntry *child = iter.next();
            child->print(newStr);
        }
    }
}

QBaseEntryList::QBaseEntryList() :QList <QBaseEntry *>()
{

}
QBaseEntryList::QBaseEntryList(const QBaseEntryList &bel):QList <QBaseEntry *>()
{
    QBaseEntry *be;
    QListIterator <QBaseEntry *> iter(bel);
    while(iter.hasNext()) {
        be = iter.next();
        QBaseEntry *beNew = new QBaseEntry(*be);
        append(beNew);
    }
}
bool QBaseEntryList::operator==( const QBaseEntryList &qbel)
{
    QBaseEntry *qbe;
    QBaseEntry *oldQBE;
    if (qbel.count() != count())
        return false;
    QListIterator<QBaseEntry *> iter(qbel);
    while(iter.hasNext()) {
        qbe = iter.next();
        oldQBE = findByName(qbe->name);
        if (!oldQBE)
            return false;
    }
    return true;
}
QBaseEntryList & QBaseEntryList::operator=( const QBaseEntryList &rhs)
{
    QBaseEntry *qbe;
    QBaseEntry *newQBE;
    if (this == &rhs)
        return *this;
    if (rhs.count() < 1)
        return *this;
    //qDebug() << "RHS COUNT = " << rhs.count() << __FILE__ << __LINE__;
    QListIterator <QBaseEntry *>iter(rhs);
    while(iter.hasNext()) {
        qbe = iter.next();
        newQBE = new QBaseEntry(*qbe);
        this->append(newQBE);
    }
    return *this;
}

QBaseEntryList *QBaseEntryList::clone()
{
    QBaseEntry *qbe;
    QBaseEntry *nqbe;
    QBaseEntryList *qbel = new QBaseEntryList();
    QListIterator <QBaseEntry *> iter(*this);
    while(iter.hasNext()) {
        qbe = iter.next();
        nqbe = new QBaseEntry(*qbe);
        if (qbe->baseEntryList) {
            nqbe->baseEntryList = new QList<QBaseEntry *>();
            QListIterator <QBaseEntry *> iter2(*qbe->baseEntryList);
            while(iter2.hasNext()) {
                QBaseEntry *qbe2 = iter2.next();
                QBaseEntry *qbe3 = new QBaseEntry(*qbe2);
                nqbe->baseEntryList->append(qbe3);
            }
        }
        qbel->append(nqbe);
    }
    return qbel;
}
QStringList QBaseEntryList::getFieldNames()
{
    QStringList fieldNames;
    QBaseEntry *qbe;
    QListIterator <QBaseEntry *> iter(*this);
    while(iter.hasNext()) {
        qbe = iter.next();
        fieldNames << qbe->name;
    }
    return fieldNames;
}

QBaseEntry * QBaseEntryList::findByName(QString &name)
{
    QBaseEntry *qbe;
    QListIterator <QBaseEntry *> iter(*this);
    while(iter.hasNext()) {
        qbe = iter.next();
        if (qbe->name == name)
            return qbe;
    }
    return 0;
}
void QBaseEntryList::removeByName(QString &name)
{
    QBaseEntry *be;
    QList<QBaseEntry *>::iterator iter;

    for (iter = this->begin(); iter != this->end();++iter) {
        be = *iter;
        if (be->name == name) {
            //qDebug() << "REMVE BY NAME" << __FILE__ << __LINE__;
            erase(iter);
            break;
        }
    }
}
void QBaseEntryList::print()
{
    QBaseEntry *be;
    QList<QBaseEntry *>::iterator iter;
    QString str;
    for (iter = this->begin(); iter != this->end();++iter) {
        be = *iter;
        be->print(str);
    }
}

MessageField::MessageField(QString &Key, QString &Name,QBaseEntryList *QBEL):
    key(Key),name(Name),qbel(QBEL)
{

}
MessageField::MessageField(QString &Key, QString &Name):
    key(Key),name(Name),qbel(0)
{

}

MessageFieldList::MessageFieldList() : QList<MessageField*>()
{

}
QMessage::QMessage(Message *m,QLatin1String sid,std::function<const F8MetaCntx&()> ctxF):mesg(m),senderID(sid),ctxFunc(ctxF)
{
    seqID = -1;
}
QMessage::QMessage(Message *m,QLatin1String sid, int seq,std::function<const F8MetaCntx&()> ctxF):mesg(m),senderID(sid),
    seqID(seq),ctxFunc(ctxF)
{
    MessageBase *header;
    MessageBase *trailer;
    GroupBase  *groupBase;
    char c[60];
    BaseField *bf;
    FieldTrait::FieldType ft;
    QVariant var;
    QString str;
    QString name;
    if (!mesg)
        return;
    header = mesg->Header();
    trailer = mesg->Trailer();
    for (Fields::const_iterator itr(header->fields_begin()); itr != header->fields_end(); ++itr)
    {
        //const FieldTrait::FieldType trait(pre.find(itr->first)->_ftype);
        name = QString::fromStdString(ctxFunc().find_be(itr->first)->_name);
        bf = itr->second;
        ft =  bf->get_underlying_type();
        if (FieldTrait::is_int(ft)) {
            int ival(static_cast<Field<int, 0>*>(bf)->get());
            var = ival;
            //qDebug() << "1 MAP INSERT name= " << name << "value = " << var << __FILE__ << __LINE__;
            map.insert(name,var);
        }
        else if (FieldTrait::is_float(ft)) {
            double fval(static_cast<Field<double, 0>*>(bf)->get());
            var = fval;
            //qDebug() << "2 MAP INSERT name= " << name << "value = " << var << __FILE__ << __LINE__;
            map.insert(name,var);
        }
        else {
            memset(c,'\0',60);
            bf->print(c);
            str =  QString::fromLatin1(c);
            //qDebug() << "3 MAP INSERT name= " << name << "value = " << str << __FILE__ << __LINE__;
            map.insert(name,str);
        }
    }
    for (Fields::const_iterator itr(mesg->fields_begin()); itr != mesg->fields_end(); ++itr)
    {
        if (mesg->get_fp().is_group(itr->first)) {
            name = QString::fromStdString(ctxFunc().find_be(itr->first)->_name);
            GroupBase *gb (mesg->find_group(itr->first));
            if (gb)
                generateItems(gb);
        }
        else {
            name = QString::fromStdString(ctxFunc().find_be(itr->first)->_name);
            bf = itr->second;
            ft =  bf->get_underlying_type();
            if (FieldTrait::is_int(ft)) {
                int ival(static_cast<Field<int, 0>*>(bf)->get());
                var = ival;
                map.insert(name,var);
            }
            if (FieldTrait::is_float(ft)) {
                double fval(static_cast<Field<double, 0>*>(bf)->get());
                var = fval;
                map.insert(name,var);
            }
            else {
                memset(c,'\0',60);
                bf->print(c);
                str =  QString::fromLatin1(c);
                map.insert(name,str);
            }
        }
    }
    for (Fields::const_iterator itr(trailer->fields_begin()); itr != trailer->fields_end(); ++itr)
    {

        name = QString::fromStdString(ctxFunc().find_be(itr->first)->_name);
        bf = itr->second;
        ft =  bf->get_underlying_type();
        if (FieldTrait::is_int(ft)) {
            int ival(static_cast<Field<int, 0>*>(bf)->get());
            var = ival;
            map.insert(name,var);
        }
        if (FieldTrait::is_float(ft)) {
            double fval(static_cast<Field<double, 0>*>(bf)->get());
            var = fval;
            map.insert(name,var);
        }
        else {
            memset(c,'\0',60);
            bf->print(c);
            str =  QString::fromLatin1(c);
            map.insert(name,str);
        }
    }
}
QMessage::QMessage(const QMessage &qm)
{
    senderID = qm.senderID;
    seqID = qm.seqID;
    if (qm.mesg)
        mesg = qm.mesg->clone();
    map = qm.map;
    ctxFunc = qm.ctxFunc;
}
void QMessage::generateItems(GroupBase *gb)
{
    char c[60];
    QString str;
    QString name;
    BaseField *bf;
    QVariant var;
    FieldTrait::FieldType ft;
    int  gbSize = gb->size();
    for(int k=0;k<gbSize;k++) {
        MessageBase *mb = gb->get_element(k);
        for (Fields::const_iterator itr(mb->fields_begin());itr != mb->fields_end(); ++itr) {
            if (mb->get_fp().is_group(itr->first)) {
                name = QString::fromStdString(ctxFunc().find_be(itr->first)->_name);
                GroupBase *gb (mb->find_group(itr->first));
                if (gb)
                {
                    generateItems(gb);
                }
            }
            else {
                name = QString::fromStdString(ctxFunc().find_be(itr->first)->_name);
                bf = itr->second;
                ft =  bf->get_underlying_type();
                if (FieldTrait::is_int(ft)) {
                    int ival(static_cast<Field<int, 0>*>(bf)->get());
                    var = ival;
                    map.insert(name,var);
                    //qDebug() << "HERE IN GENERATE: add item to map" << name << __FILE__ << __LINE__;
                }
                if (FieldTrait::is_float(ft)) {
                    double fval(static_cast<Field<double, 0>*>(bf)->get());
                    var = fval;
                    map.insert(name,var);
                }
                else {
                    memset(c,'\0',60);
                    bf->print(c);
                    str = QString::fromLatin1(c);
                    var = str;
                    map.insert(name,var);
                }
            }
        }
    }
}

QColor QMessageList::senderColors[] = {QColor(255,214,79,100),QColor(151,255,81,100),
                                       QColor(79,255,211,100),QColor(80,121,255,100),
                                       QColor(110,77,255,100),QColor(255,73,195,100)};

QMessageList::QMessageList():QList <QMessage *>()
{

}
QMessageList::QMessageList(const QMessageList &list): QList<QMessage *>(list)
{

}

QMessageList * QMessageList::clone(const bool &cancel)
{
    QMessage *message;
    QMessageList *qml = new QMessageList();
    //qDebug() << "!!! MESSAGELIST CLONE: count = " << count() << __FILE__ << __LINE__;
    qml->senderColorMap = senderColorMap;
    qml->defaultSender = defaultSender;
    QListIterator <QMessage *> iter(*this);
    int i=0;
    iter.toFront();
    while(iter.hasNext()) {

        if (i%100 == 0) { // every 100 iterations allow gui to process events
            if (cancel) {
                qDebug() << "!!!!!!!!!CALL CANCEL" << __FILE__ << __LINE__;
                if (qml) {
                    qDeleteAll(qml->begin(),qml->end());
                    delete qml;
                    qml = 0;
                    return qml;
                }
            }
            qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,5);

        }

        i++;
        message =  iter.next();
        qml->append(new QMessage(*message));
    }
    return qml;
}
