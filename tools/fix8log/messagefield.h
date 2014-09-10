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

#ifndef MESSAGEFIELD_H
#define MESSAGEFIELD_H
#include <QColor>
#include <QMap>
#include <QPair>
#include <QVariant>
#include <QList>
#include <QVector>
#include <fix8/f8includes.hpp>
#include <fix8/f8types.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"

using namespace FIX8;

class QFieldTrait {
public:
    QFieldTrait() {};
    QString name;
    FieldTrait::FieldType ft;
    QString desciption;
    QVariant::Type type;
};

class FieldTraitVector : public QVector <FieldTrait>
{
public:
    FieldTraitVector() : QVector <FieldTrait>()
    {

    }
};
class BaseEntryList : public  QList <BaseEntry *>
{
public:
    explicit BaseEntryList();
    BaseEntry *findByName(QString &);
};
class QBaseEntry
{
public:
    QBaseEntry();
    QBaseEntry(const BaseEntry &);
    QBaseEntry(const QBaseEntry *qbe);
    QBaseEntry(const QBaseEntry &);
    void print(QString &str);
    QString name;
    FieldTrait *ft;
    QList<QBaseEntry *> *baseEntryList;
};
class QBaseEntryList : public  QList <QBaseEntry *>
{
public:
    explicit QBaseEntryList();
    QBaseEntryList(const QBaseEntryList &);
    void print();
    bool   operator==( const QBaseEntryList &);
    QBaseEntryList & operator=( const QBaseEntryList &rhs);
    QBaseEntry *findByName(QString &);
    void removeByName(QString &name);
    QBaseEntryList *clone();
    QStringList getFieldNames();
};

class MessageField
{
public:
    explicit MessageField(QString  &key,QString &name,QBaseEntryList *);
    explicit MessageField(QString  &key,QString &name);
    QString key;
    QString name;
    QBaseEntryList *qbel;
    QVector<int> fieldsV;


};
class MessageFieldList : public QList<MessageField *>
{
public:
    explicit MessageFieldList();
};

class FieldUse {
public:
    FieldUse();
    QString name;
    MessageFieldList messageFieldList;
    FieldTrait *field;
    bool isDefault;
};

class FieldUseList : public QList <FieldUse *>
{
 public:
    FieldUseList();
    FieldUse * findByName(QString &);
};
class QMessage
{
  public:
    QMessage(Message *m,QLatin1String senderID,std::function<const F8MetaCntx&()> ctxFunc);
    QMessage(Message *m,QLatin1String senderID, int seqID,std::function<const F8MetaCntx&()> ctxFunc);
    QMessage(const QMessage &);
    Message *mesg;
    QString senderID;
    QMultiMap <QString, QVariant > map;
    int seqID;
private:
    void generateItems(GroupBase *gb);
    std::function<const F8MetaCntx&()> ctxFunc;

};
class QMessageList : public QList <QMessage *>
{
public:
    QMessageList();
    QMessageList(const QMessageList &list);
    QMessageList *clone(const bool &cancel); // cancel set from outside in case want to exit loading
    QMap <QString,QColor> senderColorMap;
    QString defaultSender;
    static QColor senderColors[6];
};
#endif // MESSAGEFIELD_H
