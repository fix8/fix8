#ifndef MessageItem_H
#define MessageItem_H

#include <QStandardItem>
#include <QDateTime>

class MessageItem : public QStandardItem
{
public:
    explicit MessageItem();
    explicit MessageItem(qint32);
    explicit MessageItem(float);
    explicit MessageItem(QString);
    explicit MessageItem(QDateTime &);
    inline void setValue(qint32 i)
    {
        mtype = QVariant::Int;
        ivalue = i;
        setText(QString::number(i));
    }
    void setValue(double);
    inline void setValue(QString &str)
    {
        mtype = QVariant::String;
        svalue = str;
        setText(str);
    }

    void setValue(QDateTime &);

    bool operator< ( const QStandardItem & other ) const;
private:
    qint32 ivalue;
    double  dvalue;
    QString svalue;
    QDateTime tvalue;

    QVariant::Type mtype;
};

#endif // MessageItem_H
