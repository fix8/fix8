
#include "mainwindow.h"
#include "globals.h"
#include "worksheet.h"
#include <QDebug>
#include <QDir>
#include <QtWidgets>

void MainWindow::readSettings()
{
  QSettings settings("fix8","logviewer");
   restoreGeometry(settings.value("Geometry").toByteArray());
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
   if (toolbarIconSize == GUI::Globals::smallIconSize)
       iconSizeSmallA->setChecked(true);
   else if (toolbarIconSize == GUI::Globals::largeIconSize)
       iconSizeLargeA->setChecked(true);
   else
       iconSizeRegA->setChecked(true);

    QVariant var;
    QPalette pal = mainMenuBar->palette();
    QColor defaultColor = pal.color(QPalette::Background);
    var = settings.value("MenuBarColor",defaultColor);
    QColor color = var.value<QColor>();
    pal.setColor(QPalette::Background,color);
    mainMenuBar->setPalette(pal);
    menubarColor = color;
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
