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

QBaseEntry::QBaseEntry(const BaseEntry &be):ft(0)
{
    name = QString::fromLatin1(be._name);
    baseEntryList = 0;
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
        append(be);
    }
}
bool   QBaseEntryList::operator==( const QBaseEntryList &qbel)
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

QBaseEntryList *QBaseEntryList::clone()
{
    QBaseEntry *qbe;
    QBaseEntry *nqbe;
    QBaseEntryList *qbel = new QBaseEntryList();
    QListIterator <QBaseEntry *> iter(*this);
    while(iter.hasNext()) {
        qbe = iter.next();
        nqbe = new QBaseEntry(*qbe);
        qbel->append(nqbe);
    }
    return qbel;
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
