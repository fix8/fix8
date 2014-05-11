#include "schemaitem.h"

SchemaItem::SchemaItem(QString text):QStandardItem(text),locked(false),tableSchema(0)
{
}
SchemaItem::SchemaItem(TableSchema &ts):QStandardItem(ts.name)
{
    descritption = ts.description;
    locked = ts.locked;
    tableSchema = &ts;
}
