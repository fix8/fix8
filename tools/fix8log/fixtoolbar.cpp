#include "fixtoolbar.h"
#include <QDebug>
#include <QEvent>

FixToolBar::FixToolBar(QString name, QWidget *parent) :
    QToolBar(name,parent)
{
}
bool  FixToolBar::event(QEvent *event)
{
    return QToolBar::event(event);
}
