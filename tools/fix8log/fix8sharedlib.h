#ifndef FIX8SHAREDLIB_H
#define FIX8SHAREDLIB_H
#include <QList>
#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include <fix8/f8types.hpp>
#include "fix8/traits.hpp"
#include "messagefield.h"

class Fix8SharedLib
{
public:
    Fix8SharedLib();
    typedef enum {SystemLib,UserLib} LibType;
    qint16 count;
    QString name;
    QString fileName;
    LibType libType;
    bool    isOK;
    QString errorMessage;
    std::function<const F8MetaCntx&()> ctxFunc;
    MessageFieldList *messageFieldList;
    FieldTraitVector fieldTraitV;
    QList<QPair<QString ,FieldUse *>> fieldUsePairList;
    QMap<QString, QBaseEntry *> baseMap;
    QMultiMap <QString,FieldTrait *> fieldsInUseMap;
    FieldUseList fieldUseList;
    QStringList defaultHeaderStrs;
    QBaseEntryList defaultHeaderItems;
};

class Fix8SharedLibList : public QList <Fix8SharedLib *>
{
public:
    Fix8SharedLibList();
    Fix8SharedLib *findByName(QString &name);
    bool removeByName(QString &name);
};
#endif // FIX8SHAREDLIB_H
