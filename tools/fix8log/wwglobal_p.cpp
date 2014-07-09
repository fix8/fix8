#include "wwglobal_p.h"
#include <QColor>
#include <QString>
#include <QMimeData>
#include <QPixmap>
#include <QWidget>
#include <QStyle>

QwwPrivatable::QwwPrivatable(QwwPrivate *p) {
    d_ww_ptr = p;
}
QwwPrivatable::~QwwPrivatable() {
    delete d_ww_ptr;
}

QString wwFindStandardColorName(const QColor &c){
    foreach(QString name, QColor::colorNames()){
        QColor tmp;
        tmp.setNamedColor(name);
        if(tmp==c)
            return name;
    }
    return QString::null;
}

ColorDrag::ColorDrag(QWidget *source, const QColor &color, const QString &name) : QDrag(source) {
        int siz = source->style()->pixelMetric(QStyle::PM_ButtonIconSize, 0, source);
        QPixmap px = QPixmap(QSize(siz,siz));
        px.fill(color);
        setPixmap(px);
        QMimeData *mimeData = new QMimeData;
        mimeData->setColorData(color);
        mimeData->setText(name);
        mimeData->setImageData(px.toImage());
        setMimeData(mimeData);
}
namespace wwWidgets {

QIcon icon(const QString &name, const QIcon &fallback) {
#if QT_VERSION >= 0x040600
    return QIcon::fromTheme(name, fallback);
#else
    return fallback;
#endif
}
}
