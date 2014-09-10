#include "fieldsview.h"
#include <QDebug>
#include <QQuickItem>

FieldsView::FieldsView(QWidget *parent) :
    QQuickWidget(parent)
{
    setSource(QUrl("qrc:qml/fieldView.qml"));
    qmlObject = rootObject();
    if (!qmlObject) {
        qWarning() << "Root Object not found" << __FILE__ << __LINE__;
    }
}
void FieldsView::setBackgroundColor(QColor &color)
{
    QVariant returnedValue;
    qDebug() << "Set Color" << __FILE__ << __LINE__;
    QMetaObject::invokeMethod (qmlObject, "setBackground",
                               Q_RETURN_ARG(QVariant, returnedValue),
                               Q_ARG(QVariant,color));
}
