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

#include "fixtoolbar.h"
#include "mainwindow.h"
#include "globals.h"
#include "worksheet.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QtWidgets>

void MainWindow::readSettings()
{
  QSettings settings("fix8","logviewer");
  QDesktopWidget *desktop = QApplication::desktop();
  QRect rect = desktop->screenGeometry(desktop->primaryScreen());
  int h = rect.height()*.75;
  int w = rect.width()*.70;
  int x = (rect.width() -w)/2;
  int y = (rect.height() - h)/2;
  QRect defaultSize(x,y,w,h);


  bool bstatus =  restoreGeometry(settings.value("Geometry").toByteArray());
  if (!bstatus) {
   qDebug() << "NO DEFAULT SIZE" << __FILE__ << __LINE__;
    setGeometry(defaultSize);
  }
   restoreState(settings.value("MainWindowState").toByteArray());
   restoreDockWidget(consoleDock);
    lastSelectedDir = settings.value("LastSelectedDir").toString();
   fileDirState= settings.value("FileDirState").toByteArray();
   fileFilter = settings.value("FileFilter").toString();
   Qt::ToolButtonStyle  tbs = (Qt::ToolButtonStyle) settings.value("ToolButtonStyle",Qt::ToolButtonIconOnly).toInt();
   setToolButtonStyle(tbs);
   switch (tbs) {
     case Qt::ToolButtonIconOnly:
       iconsOnlyA->setChecked(true);
       break;
   case Qt::ToolButtonTextOnly:
       iconsTextOnlyA->setChecked(true);
       break;
   default:
        iconsWithTextA->setChecked(true);
   }
   QSize toolbarIconSize = settings.value("ToolButtonSize",GUI::Globals::regIconSize).toSize();
   mainToolBar->setIconSize(toolbarIconSize);
   searchToolBar->setIconSize(toolbarIconSize);
   filterToolBar->setIconSize(toolbarIconSize);

   if (toolbarIconSize == GUI::Globals::smallIconSize)
       iconSizeSmallA->setChecked(true);
   else if (toolbarIconSize == GUI::Globals::largeIconSize)
       iconSizeLargeA->setChecked(true);
   else
       iconSizeRegA->setChecked(true);

    QVariant var;
    QPalette pal = mainMenuBar->palette();
    var = settings.value("MenuBarColor");
    menuBarStyleSheet = var.toString();
    mainMenuBar->setStyleSheet(menuBarStyleSheet);
    fileMenu->setStyleSheet(menuStyle);
    windowMenu->setStyleSheet(menuStyle);
    optionMenu->setStyleSheet(menuStyle);
    schemaMenu->setStyleSheet(menuStyle);
    helpMenu->setStyleSheet(menuStyle);
    messageSplitterSettings = settings.value("MessageSplitter").toByteArray();
}
void MainWindow::writeSettings()
{
  QSettings settings("fix8","logviewer");
  settings.setValue("Geometry",saveGeometry());
  settings.setValue("MainWindowState",saveState());
  if (tabW->count() > 0) {
      WorkSheet *ws =  qobject_cast <WorkSheet *> (tabW->widget(0));
      if (ws) {
          QByteArray ba = ws->splitter->saveState();
          settings.setValue("MessageSplitter",ba);
      }
  }
}
