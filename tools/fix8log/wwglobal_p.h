#ifndef __WW_GLOBAL_P_H
#define __WW_GLOBAL_P_H

#include "wwglobal.h"


#define WW_DECLARE_PUBLIC(Class) \
    inline Class* q_func() { return static_cast<Class *>(q_ww_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ww_ptr); } \
    friend class Class;

/**
 * \internal
 * @class QwwPrivate
 * @class QwwPrivatable
 */
class QwwPrivate {
protected:
    QwwPrivate(QwwPrivatable *p) {
        q_ww_ptr = p;
    }
    QwwPrivatable *q_ww_ptr;
};

#include <QDrag>

class ColorDrag : public QDrag {
public:
    ColorDrag(QWidget *source, const QColor &color, const QString &name);
};

#include <QIcon>

namespace wwWidgets {
    QIcon icon(const QString &name, const QIcon &fallback = QIcon());
};

#endif
