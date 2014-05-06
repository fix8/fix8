#include "messagefield.h"

MessageField::MessageField(qint32 ID, QString &Name,QVariant Var) :
    id(ID),name(Name),variant(Var)
{

}
MessageFieldList::MessageFieldList() : QList<MessageField>()
{

}
