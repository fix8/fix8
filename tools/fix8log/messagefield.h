#ifndef MESSAGEFIELD_H
#define MESSAGEFIELD_H

#include <QPair>
#include <QVariant>
#include <QList>

class MessageField
{
public:
    explicit MessageField(qint32,QString &,QVariant);
qint32 id;
QString name;
QVariant variant;

};
class MessageFieldList : public QList<MessageField>
{
public:
explicit MessageFieldList();
};
#endif // MESSAGEFIELD_H
