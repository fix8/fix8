#ifndef MESSAGEFIELD_H
#define MESSAGEFIELD_H

#include <QPair>
#include <QVariant>
#include <QList>

class MessageField : public QPair<qint32,QVariant>
{
public:
    explicit MessageField(qint32,QVariant);
};
class MessageFieldList : public QList<MessageField>
{
public:
explicit MessageFieldList();
};
#endif // MESSAGEFIELD_H
