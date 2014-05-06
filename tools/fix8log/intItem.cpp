#include <QDebug>
#include "intItem.h"

IntItem::IntItem(qint32 Value):QStandardItem(QString::number(Value)),value(Value)
{
  \
}	
IntItem::IntItem(const IntItem &it):QStandardItem()
{
  setText(it.text());
  value = it.value;
}
bool IntItem::operator< ( const QStandardItem & other ) const
{  
      const IntItem *ot = (IntItem *) &other;
      return (this->value <  ot->value);
}
