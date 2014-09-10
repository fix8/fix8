#include "messageitemdelegate.h"
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QDebug>
int MessageItemDelegate::InUseRole = Qt::UserRole +2;
MessageItemDelegate::MessageItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    inUseOldPixmap = QPixmap(":/images/svg/checkmark.svg").scaledToHeight(16);//,Qt::SmoothTransformation);
}
QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    /*
    QSize s;
    s.setWidth(option.fontMetrics.width(option.text) + inUseOldPixmap.width() + (inUseOldPixmap.width()*.50)); // margin 50% of pixmap
    s.setHeight(option.fontMetrics.height() + (option.fontMetrics.height()*0.20)); // 20% margin
    */
    QSize s = QStyledItemDelegate::sizeHint(option,index);
    return s;

}
void MessageItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);
    bool inUse;
    QRect rect = option.rect;
    if (index.data(InUseRole).isValid()) {
        inUse = index.data(InUseRole).toBool();
        if  (inUse) {
            int x     = rect.x() +  rect.width()- 5 - inUseOldPixmap.width();
            int y = rect.y() + (rect.height() - inUseOldPixmap.height())/2;
            painter->drawPixmap(x,y,inUseOldPixmap);
        }
    }
}
