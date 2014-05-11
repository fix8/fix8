#ifndef SCHEMAITEM_H
#define SCHEMAITEM_H

#include <QStandardItem>
#include <QString>
#include "tableschema.h"
class SchemaItem : public QStandardItem
{
public:
    SchemaItem(QString str);
    SchemaItem(TableSchema &ts);
    QString descritption;
    bool locked; // cant erase
    TableSchema *tableSchema;
};

#endif // SCHEMAITEM_H
