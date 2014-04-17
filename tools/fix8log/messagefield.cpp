#include "messagefield.h"

MessageField::MessageField(qint32 k, QVariant v) :
    QPair<qint32,QVariant>()
{
    first = k;
    second = v;
}
MessageFieldList::MessageFieldList() : QList<MessageField>()
{

}
