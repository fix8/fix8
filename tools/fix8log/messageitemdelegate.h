#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H

#include <QStyledItemDelegate>

class MessageItemDelegate : public QStyledItemDelegate
{
public:
    explicit MessageItemDelegate(QObject *parent = 0);
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
    void paint(QPainter *painter,
           const QStyleOptionViewItem &option,
           const QModelIndex &index) const;
      QPixmap inUseOldPixmap;
      QPixmap inUseNewPixmap;
      static int InUseRole;
};

#endif // MESSAGEITEMDELEGATE_H
