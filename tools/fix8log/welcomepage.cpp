#include "welcomepage.h"

WelcomePage::WelcomePage(QWidget *parent) :
    QQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSource(QUrl("qrc:qml/welcome.qml"));
}
