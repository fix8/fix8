#include "globals.h"
#include <QDebug>
#include <QPalette>
#include <QToolBar>
using namespace GUI;
float   Globals::version = 0.0;
QString Globals::versionStr;
QSize Globals::smallIconSize;
QSize Globals::regIconSize;
QSize Globals::largeIconSize;
QColor Globals::menubarDefaultColor;
Globals* Globals::m_pInstance = NULL;
Globals* Globals::Instance()
{
    if (!m_pInstance)   {// Only allow one instance of class to be generated.
        m_pInstance = new Globals;
     QToolBar *toolbar = new QToolBar();
     QPalette pal = toolbar->palette();
     menubarDefaultColor = pal.color(QPalette::Background);
     regIconSize = toolbar->iconSize();
     smallIconSize = regIconSize.scaled(regIconSize.width()*.66,regIconSize.height()*.66,Qt::KeepAspectRatio);
     largeIconSize = regIconSize.scaled(regIconSize.width()*1.333,regIconSize.height()*1.333,Qt::KeepAspectRatio);
     delete toolbar;
    }
    return m_pInstance;

}
Message::Message(QString str, MessageType mt):msg(str),messageType(mt)
{

}

Message::Message(const Message &m): msg(m.msg), messageType(m.messageType)
{

}
Message::Message()
{

}
