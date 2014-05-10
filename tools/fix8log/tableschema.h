#ifndef TABLESCHEMA_H
#define TABLESCHEMA_H
#include <QString>
#include <QList>
class TableSchema
{
public:
    TableSchema();
    TableSchema(QString name, QString description,bool isLocked);
    TableSchema(const TableSchema &);
    TableSchema & operator=( const TableSchema &rhs);
    qint32 id;
    QString name;
    QString description;
    bool locked;
};

class TableSchemaList : public QList <TableSchema *>
{
public:
 TableSchemaList();
 ~TableSchemaList();
 TableSchema *findByID(qint32 id);
 TableSchema *findByName(const QString &name);
};
#endif // TABLESCHEMA_H
