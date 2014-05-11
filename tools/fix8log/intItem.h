
#ifndef INT_ITEM_H
#define INT_ITEM_H
#include <QDateTime>
#include <QStandardItem>

class IntItem : public QStandardItem
{
 public:
  IntItem(qint32 value);
  IntItem(const IntItem &);
   bool operator< ( const QStandardItem & other ) const;
  qint32 value;
};
#endif
