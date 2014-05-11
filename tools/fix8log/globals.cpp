#include "globals.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QPalette>
#include <QToolBar>
#include <QVector>
using namespace GUI;
float   Globals::version = 0.2;
int Globals::databaseVersion;
QString Globals::versionStr;
QSize Globals::smallIconSize;
QSize Globals::regIconSize;
QSize Globals::largeIconSize;
QColor Globals::menubarDefaultColor;
Globals* Globals::m_pInstance = NULL;

QString Globals::timeFormats[] {
    "Day-Mon-YY hh:min:sec", "Day Mon hh:min:sec", "hh:min:sec","hh:min"};
Globals::TimeFormat Globals::timeFormat = Globals::HHMM;
QVector <Globals::MessagePair> * Globals::messagePairs=0;
Globals* Globals::Instance()
{
    if (!m_pInstance)   {// Only allow one instance of class to be generated.
        m_pInstance = new Globals;
     databaseVersion  = 2;
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
QDataStream &operator<<(QDataStream &ds, const fix8logdata &data)
{
    ds << data.windowID;
    ds << data.worksheetID;
    return ds;
}

QDataStream &operator>>(QDataStream &ds, fix8logdata &data)
{
   ds >> data.windowID;
   ds >> data.worksheetID;
   return ds;
}

