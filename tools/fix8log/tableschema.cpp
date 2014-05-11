#include "tableschema.h"
#include <QDebug>
TableSchema::TableSchema():id(-1),locked(false)
{
}
TableSchema::TableSchema(QString Name, QString Description,bool Locked):id(-1),
    name(Name),description(Description),locked(Locked)
{
}
TableSchema::TableSchema(const TableSchema &ts)
{
    name = ts.name;
    description = ts.description;
    locked = ts.locked;
}
TableSchema & TableSchema::operator=( const TableSchema &rhs)
{
    if (this == &rhs)
        return *this;
    name        = rhs.name;
    description = rhs.description;
    locked      = rhs.locked;
    return *this;
}
TableSchemaList::TableSchemaList():QList <TableSchema *>()
{

}

TableSchemaList::~TableSchemaList()
{
    qDebug() << "Delete Table Schema List" << __FILE__ << __LINE__;
}

TableSchema *TableSchemaList::findByID(qint32 id)
{
    TableSchema *ts = 0;
    QListIterator <TableSchema *> iter(*this);
    while(iter.hasNext()) {
        ts = iter.next();
        if (ts->id == id)
            return ts;
    }
    return 0;
}

TableSchema *TableSchemaList::findByName(const QString &name)
{
    TableSchema *ts = 0;
    QListIterator <TableSchema *> iter(*this);
    while(iter.hasNext()) {
        ts = iter.next();
        if (ts->name == name)
            return ts;
    }
    return 0;
}
