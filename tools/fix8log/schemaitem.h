#ifndef SCHEMAITEM_H
#define SCHEMAITEM_H

#include <QStandardItem>
#include <QString>

class SchemaItem : public QStandardItem
{
public:
    SchemaItem(QString str);
    QString descritption;
    bool locked; // cant erase
};

#endif // SCHEMAITEM_H
