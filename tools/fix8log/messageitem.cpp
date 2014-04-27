#include "messageitem.h"
#include <QDebug>
MessageItem::MessageItem():QStandardItem()
{

}
MessageItem::MessageItem(qint32 i):
    QStandardItem(),ivalue(i),mtype(QVariant::Int)
{
    setText(QString::number(i));
}
MessageItem::MessageItem(float f):
    QStandardItem(),dvalue(f),mtype(QVariant::Double)
{
    setText(QString::number(f));
}
MessageItem::MessageItem(QString s):
    QStandardItem(),svalue(s),mtype(QVariant::String)
{
    setText(s);
}
MessageItem::MessageItem(QDateTime &dt):
    QStandardItem(),tvalue(dt),mtype(QVariant::DateTime)
{
    setText(dt.toString());
}
/*void MessageItem::setValue(qint32 i)
{
    mtype = QVariant::Int;
    ivalue = i;
    setText(QString::number(i));
}
*/
void MessageItem::setValue(double f)
{
    mtype = QVariant::Double;
    dvalue = f;
    setText(QString::number(f));
}
/*
void MessageItem::setValue(QString str)
{
    mtype = QVariant::String;
    svalue = str;
    setText(str);
}
*/
void MessageItem::setValue(QDateTime &dt)
{
    mtype = QVariant::DateTime;
    tvalue = dt;
    setText(dt.toString());
}
bool MessageItem::operator< ( const QStandardItem & other ) const
{
    const MessageItem *ot = (MessageItem *) &other;
    switch(mtype) {
    case QVariant::Int:
        return (this->ivalue <  ot->ivalue );
        break;
    case QVariant::Double:
        return (this->dvalue <  ot->dvalue );
        break;

    case QVariant::String:
        return (this->svalue <  ot->svalue );
        break;
    case QVariant::DateTime:
        return (this->tvalue <  ot->tvalue );
        break;
    default:
        qWarning() << "Unkown data type for < operator" << __FILE__;
        return false;
    }
}
