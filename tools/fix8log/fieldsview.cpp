#include "fieldsview.h"

FieldsView::FieldsView(QWidget *parent) :
    QQuickWidget(parent)
{
    setSource(QUrl("qrc:qml/fieldView.qml"));
}
