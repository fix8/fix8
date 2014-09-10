//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

#include "globals.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QPalette>
#include <QToolBar>
#include <QVector>
using namespace GUI;
float   Globals::version = 0.6;
int Globals::databaseVersion;
QString Globals::versionStr;
QSize Globals::smallIconSize;
QSize Globals::regIconSize;
QSize Globals::largeIconSize;
QColor Globals::menubarDefaultColor;
Globals* Globals::m_pInstance = NULL;
bool Globals::isFirstTime = false;
QString Globals::appName = "Fix8 LogViewer";
QString Globals::timeFormats[] {
    "Day-Mon-YY hh:min:sec", "Day Mon hh:min:sec", "hh:min:sec","hh:min"};
Globals::TimeFormat Globals::timeFormat = Globals::HHMM;
//QVector <Globals::MessagePair> * Globals::messagePairs=0;
qint32 Globals::fontPtSize = 16;
Globals* Globals::Instance()
{
    if (!m_pInstance)   {// Only allow one instance of class to be generated.
        m_pInstance = new Globals;
     databaseVersion  = 2;
     versionStr = QString::number(version);
     QToolBar *toolbar = new QToolBar();
     QPalette pal = toolbar->palette();
     menubarDefaultColor = pal.color(QPalette::Background);
     regIconSize = toolbar->iconSize();
     smallIconSize = regIconSize.scaled(regIconSize.width()*.66,regIconSize.height()*.66,Qt::KeepAspectRatio);
     largeIconSize = regIconSize.scaled(regIconSize.width()*1.333,regIconSize.height()*1.333,Qt::KeepAspectRatio);
     delete toolbar;
     QMainWindow *mw = new QMainWindow();
     fontPtSize = mw->font().pointSize();
     delete mw;
    }
    return m_pInstance;
}

ConsoleMessage::ConsoleMessage(QString str, ConsoleMessageType mt):msg(str),messageType(mt)
{

}

ConsoleMessage::ConsoleMessage(const ConsoleMessage &m): msg(m.msg), messageType(m.messageType)
{

}
ConsoleMessage::ConsoleMessage()
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

