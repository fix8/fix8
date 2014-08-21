#ifndef FIX8SHAREDLIB_H
#define FIX8SHAREDLIB_H
#include <QList>
#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include <fix8/f8types.hpp>
#include "fix8/traits.hpp"
#include "tableschema.h"
#include "messagefield.h"
class QLibrary;

class Fix8SharedLib
{
public:
    Fix8SharedLib();

    static Fix8SharedLib *create(QString fileName);
    typedef enum {SystemLib,UserLib} LibType;
    void setDefaultTableSchema(TableSchema *defaultTS);
    TableSchema *createDefaultTableSchema();
    TableSchema *getDefaultTableSchema();
    bool generateSchema(TableSchema *ts);
    TableSchemaList *getTableSchemas();
    TableSchema *getTableSchema(qint32 tableSchemaID);
    void setTableSchemas(TableSchemaList *tsl);
    qint16 count;
    QString name;
    QString fileName;
    LibType libType;
    bool    isOK;
    QString errorMessage;
    QLibrary *fixLib;
    std::function<const F8MetaCntx&()> ctxFunc;
    QFunctionPointer _handle;
    MessageFieldList *messageFieldList;
    FieldTraitVector fieldTraitV;
    QList<QPair<QString ,FieldUse *>> fieldUsePairList;
    QMap<QString, QBaseEntry *> baseMap;
    QMultiMap <QString,FieldTrait *> fieldsInUseMap;
    FieldUseList fieldUseList;
    QBaseEntryList defaultHeaderItems;
    TableSchema *defaultTableSchema;
    TableSchemaList *tableSchemas;
    static QStringList defaultHeaderStrs;
    static void init();

    bool loadFix8so();
private:
    void generate_traits(const TraitHelper &tr, QMap <QString, QBaseEntry *> &baseMap,FieldUseList &ful,
                      MessageField *mf,QList <QBaseEntry *> *qbaseEntryList, int *level);
    void generate_traits(const TraitHelper &tr,QMap <QString, QBaseEntry *> &baseMap,FieldUseList &ful,
                      MessageField *mf,QBaseEntryList *qbaseEntryList,int *level);
};

class Fix8SharedLibList : public QList <Fix8SharedLib *>
{
public:
    Fix8SharedLibList();
    Fix8SharedLib *findByName(QString &name);
    Fix8SharedLib *findByFileName(QString  &filename);
    bool removeByName(QString &name);
};
#endif // FIX8SHAREDLIB_H
