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

#include "mainwindow.h"
#include "messagearea.h"
#include "fixmimedata.h"
#include "fixtoolbar.h"
#include "globals.h"
#include "nodatalabel.h"
#include "fix8sharedlib.h"
#include "tableschema.h"
#include "worksheet.h"
#include <QDebug>
#include "globals.h"
#include <QtWidgets>
//#include <QStandardItemModel>

void MainWindow::quitSlot()
{
    writeSettings();
    emit exitApp();
    //qApp->quit();
}
void MainWindow::editSchemaSlot()
{
    if (tableSchema) {
        qDebug() << "Current Table Schema = " << tableSchema->name << __FILE__ << __LINE__;
    }
    else
        qDebug() << "We have null table schema";
    emit editSchema(this);
}

void MainWindow::hideColumnActionSlot(QAction *action)
{
    WorkSheet *workSheet;
    if (!action)
        return;
    QVariant var =  action->data();
    qint32 colNum = var.toInt();

    for(int i =0;i< tabW->count();i++) {
        workSheet = qobject_cast <WorkSheet *> (tabW->widget(i));
        if (workSheet)
            workSheet->hideColumn(colNum,action->isChecked());
    }
}
void MainWindow::iconStyleSlot(QAction *action)
{
    QSettings settings("fix8","logviewer");
    Qt::ToolButtonStyle  tbs = Qt::ToolButtonTextOnly;
    tbs = Qt::ToolButtonTextOnly;
    if (action == iconsOnlyA)
        tbs = Qt::ToolButtonIconOnly;
    else if (action == iconsWithTextA)
        tbs = Qt::ToolButtonTextUnderIcon;
    //setToolButtonStyle(tbs);
    emit toolButtonStyleModified(tbs);
    settings.setValue("ToolButtonStyle",tbs);
}
void MainWindow::iconSizeSlot(QAction *action)
{
    QSettings settings("fix8","logviewer");
    QSize is = GUI::Globals::regIconSize;
    if (action == iconSizeSmallA)
        is = GUI::Globals::smallIconSize;
    else if (action == iconSizeLargeA)
        is  = GUI::Globals::largeIconSize;
    mainToolBar->setIconSize(is);
    searchToolBar->setIconSize(is);
    filterToolBar->setIconSize(is);
    settings.setValue("ToolButtonSize",is);
}
void MainWindow::configFGSlot()
{
    colorSelectionFG = true;
    int status;
    QSettings settings("fix8","logviewer");

    menuBarStyleSheet =  mainMenuBar->styleSheet();
    QColorDialog *colorDialog = new QColorDialog(this);
    colorDialog->setWindowTitle(tr("Select Menu Bar Text Color"));
    colorDialog->setCustomColor(0,GUI::Globals::menubarDefaultColor);
    connect(colorDialog,SIGNAL(colorSelected(QColor)),this,SLOT(setColorSlot(QColor)));
    connect(colorDialog,SIGNAL(currentColorChanged(QColor)),this,SLOT(currentColorChangedSlot(QColor)));
    colorDialog->raise();
    status = colorDialog->exec();
    if (status != QDialog::Accepted) {
        menuBar()->setStyleSheet(menuBarStyleSheet);
    }
    else {
        menuBarStyleSheet =  mainMenuBar->styleSheet();
        settings.setValue("MenuBarColor",menuBarStyleSheet);
    }
    colorDialog->deleteLater();
}
void MainWindow::configSlot()
{
    int status;
    QSettings settings("fix8","logviewer");

    colorSelectionFG = false;
    menuBarStyleSheet =  mainMenuBar->styleSheet();
    QColorDialog *colorDialog = new QColorDialog();
    colorDialog->setWindowTitle(tr("Select Menu Bar Color"));
    colorDialog->setCustomColor(0,GUI::Globals::menubarDefaultColor);
    connect(colorDialog,SIGNAL(colorSelected(QColor)),this,SLOT(setColorSlot(QColor)));
    connect(colorDialog,SIGNAL(currentColorChanged(QColor)),this,SLOT(currentColorChangedSlot(QColor)));
    colorDialog->raise();
    status = colorDialog->exec();
    if (status != QDialog::Accepted) {
        menuBar()->setStyleSheet(menuBarStyleSheet);
        fileMenu->setStyleSheet("");
    }
    else  {
        menuBarStyleSheet =  mainMenuBar->styleSheet();
        settings.setValue("MenuBarColor",menuBarStyleSheet);
    }
    colorDialog->deleteLater();
}
void MainWindow::setColorSlot(QColor  color)
{    
    QString colorStr;
    QString colorValue;
    QString backgroundValue;
    QStringList attributes = menuBarStyleSheet.split(';');
    QStringListIterator iter(attributes);
    QString attribute;
    while(iter.hasNext()) {
        attribute = iter.next();
        if (attribute.contains("background-color")) {
            backgroundValue = attribute;
            if (backgroundValue.length() > 2)
                backgroundValue = backgroundValue.split(':').last();
            else {
                backgroundValue = menuBar()->palette().color(QPalette::Window).name();
                qDebug() << "Use palette" << backgroundValue << __FILE__ << __LINE__;
            }
        }
        if (attribute.contains("color")) {
            colorValue = attribute;
            if (colorValue.length() > 2)
                colorValue  = colorValue.split(':').last();
            else
                colorValue = menuBar()->palette().color(QPalette::WindowText).name();
        }
    }
    if (backgroundValue.length() < 2)
        backgroundValue = menuBar()->palette().color(QPalette::Window).name();

    if (colorValue.length() <2)
        colorValue = menuBar()->palette().color(QPalette::WindowText).name();
    if (colorSelectionFG)
        colorValue = color.name();
    else
        backgroundValue = color.name();
        qDebug() << "Attributes: " << attributes;

   colorStr = "QMenuBar::item{ background:" +  backgroundValue +  "; color:" + colorValue + "} QMenuBar{background:" + backgroundValue + ";}";
   menuBar()->setStyleSheet(colorStr);

}
void MainWindow::currentColorChangedSlot(QColor color)
{
    QString colorStr;
    QString colorValue;
    QString backgroundValue;
    QStringList attributes = menuBarStyleSheet.split(';');
    QStringListIterator iter(attributes);
    QString attribute;
    while(iter.hasNext()) {
        attribute = iter.next();
        if (attribute.contains("background-color")) {
            backgroundValue = attribute;
            if (backgroundValue.length() > 2)
                backgroundValue = backgroundValue.split(':').last();
            else {
                backgroundValue = menuBar()->palette().color(QPalette::Window).name();
                qDebug() << "Use palette" << backgroundValue << __FILE__ << __LINE__;
            }
        }
        if (attribute.contains("color")) {
            colorValue = attribute;
            if (colorValue.length() > 2)
                colorValue  = colorValue.split(':').last();
            else
                colorValue = menuBar()->palette().color(QPalette::WindowText).name();
        }
    }
    if (backgroundValue.length() < 2)
        backgroundValue = menuBar()->palette().color(QPalette::Window).name();

    if (colorValue.length() <2)
        colorValue = menuBar()->palette().color(QPalette::WindowText).name();
    if (colorSelectionFG)
        colorValue = color.name();
    else
        backgroundValue = color.name();
        qDebug() << "Attributes: " << attributes;

   colorStr = "QMenuBar::item{ background:" +  backgroundValue +  "; color:" + colorValue + "} QMenuBar{background:" + backgroundValue + ";}";
   menuBar()->setStyleSheet(colorStr);
    //fileMenu->setStyleSheet(menuStyle);
   // optionMenu->setStyleSheet(menuStyle);
   // schemaMenu->setStyleSheet(menuStyle);
   // helpMenu->setStyleSheet(menuStyle);
}
void MainWindow::editTabNameSlot(bool isOn)
{
    QString tabName;
    WorkSheet *workSheet;
    int index = tabW->currentIndex();
    if (isOn) {
        tabName =  tabW->tabText(index);
        cancelEditTabNamePB->show();
        tabNameLineEdit->show();
        tabNameLineEdit->setFocus();
        editTabNamePB->setToolTip("Save");
        tabNameLineEdit->setText(tabName);
        tabNameLineEdit->selectAll();
    }
    else  {
        cancelEditTabNamePB->hide();
        tabNameLineEdit->hide();
        editTabNamePB->setToolTip("Edit current tab name");
        tabName = tabNameLineEdit->text();
        tabW->setTabText(index,tabName);
        workSheet = qobject_cast <WorkSheet *> (tabW->widget(index));
        if (workSheet)
            workSheet->setAlias(tabName);
    }
}
void MainWindow::cancelTabNameSlot()
{
    cancelEditTabNamePB->hide();
    tabNameLineEdit->hide();
    editTabNamePB->setToolTip("Edit current tab name");
    editTabNamePB->setChecked(false);
    editTabNamePB->setEnabled(true);
}
void MainWindow::tabNameModifiedSlot(QString str)
{
    bool isValid = false;
    if (str.length() >0)
        isValid = true;
    editTabNamePB->setEnabled(isValid);
    if(isValid)
        tabNameLineEdit->setToolTip(tr("Enter return to change name and exit edit mode"));
    else
        tabNameLineEdit->setToolTip(tr("Enter tab name"));
}
void MainWindow::tabNameReturnKeySlot()
{
    QString tabName;
    WorkSheet *worksheet;
    int index = tabW->currentIndex();
    tabName = tabNameLineEdit->text();
    if (tabName.length() < 1)
        return;
    cancelEditTabNamePB->hide();
    tabNameLineEdit->hide();
    editTabNamePB->setChecked(false);
    editTabNamePB->setToolTip("Edit current tab name");
    tabW->setTabText(index,tabName);
    worksheet = qobject_cast <WorkSheet *> (tabW->widget(index));
    worksheet->setAlias(tabName);
}
void MainWindow::tabCurentChangedSlot(int index)
{
    QMenu *senderMenu;
    WorkSheet *worksheet = qobject_cast <WorkSheet *> (tabW->widget(index));
    if (worksheet) {
        SearchFunction sf = worksheet->getSearchFunction();
        setSearchFunction(sf);
        senderMenu = worksheet->getSenderMenu();
        if (senderMenu)
            filterSenderMenuA->setMenu(senderMenu);
        else
            filterSenderMenuA->setMenu(0);
    }
    if (!editTabNamePB->isChecked())
        return;
    cancelTabNameSlot();
}
void MainWindow::showMessageArea(bool bstatus)
{
    WorkSheet *workSheet;
    for(int i=0;i < tabW->count();i++) {
        workSheet = qobject_cast <WorkSheet *> (tabW->widget(i));
        if(bstatus)
            workSheet->messageArea->hide();
        else
            workSheet->messageArea->show();
    }
}
void MainWindow::autoSaveOnSlot(bool isOn)
{
    emit autoSaveOn(isOn);
}
void MainWindow::cancelSessionRestoreSlot()
{
    loadingActive = false;
    if (tabW->count() > 0) {
        stackW->setCurrentIndex(ShowTab);
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
    }
    else {
        stackW->setCurrentIndex(ShowNoDataLabel);
        copyTabA->setEnabled(false);
        showMessageA->setEnabled(false);
    }
    emit cancelSessionRestore();
}
void MainWindow::setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat tf)
{
    emit notifyTimeFormatChanged(tf);
}
void MainWindow::setTimeFormatSlot(GUI::Globals::TimeFormat tf)
{
    WorkSheet *workSheet;
    for(int i=0;i < tabW->count();i++) {
        workSheet = qobject_cast <WorkSheet *> (tabW->widget(i));
        if (workSheet)
            workSheet->setTimeFormat(tf);
    }
    repaint();
}
void MainWindow::toolbarOrientationChangedSlot(Qt::Orientation orient)
{
    if (orient == Qt::Vertical) {
        searchL->setText("");
        searchLV->setText("Search");
    }
    else {
        searchL->setText("Search");
        searchLV->setText("");
    }
}
void MainWindow::modelDroppedSlot(FixMimeData *m)
{
    if (!m)
        return;
    emit modelDropped(m);
}
void MainWindow::schemaSelectedSlot(QAction *action)
{
    TableSchema *ts;
    WorkSheet *ws;

    QVariant var =  action->data();

    ts = (TableSchema *) var.value<void *>();
    if (!ts) {
        qWarning() << "Error - Table Schema Not Found" << __FILE__ << __LINE__;
        return;
    }
    schemaV->setText(ts->name);
    tableSchema = ts;
    QListIterator <WorkSheet *> iter2(workSheetList);
    while(iter2.hasNext()) {
        ws = iter2.next();
        ws->setTableSchema(tableSchema);
    }
    emit tableSchemaChanged(ts);
}
void MainWindow::setWindowNameSlot()
{
    bool ok;
    QString str =
            QInputDialog::getText(this,"Fix8LogView","Window Title",
                                  QLineEdit::Normal,windowTitle(),&ok);
    if (ok && str.length() > 0) {
        setWindowTitle(str);
        name = str;
    }
}
void MainWindow::displayMessageSlot(GUI::ConsoleMessage msg)
{
    displayConsoleMessage(msg);
}
void MainWindow::terminatedWorkSheetCopySlot(WorkSheet *ws)
{
    qDebug() << "REMOVE THIS SLOT ?" << __FILE__ << __LINE__;
}
void MainWindow::doPopupSlot(const QModelIndex &mi,const  QPoint &pt)
{
    popupMenu->popup(pt);
}
void MainWindow::popupMenuActionSlot(QAction*)
{

}
void MainWindow::linkSearchSlot(bool turnedOn)
{
    qDebug() << "Link Search Enabled " << turnedOn << __FILE__ << __LINE__;
    linkSearchOn = turnedOn;
}
void MainWindow::setFontSlot(QAction *fontAction)
{
  QFont fnt = font();
  int ptSize = fnt.pointSize();
  if (fontAction == fontIncreaseA)
      ptSize +=1;
  else if (fontAction == fontDecreaseA)
      ptSize -=1;
  else { // set to default
      ptSize = GUI::Globals::fontPtSize;
  }
  fnt.setPointSize(ptSize);
  setFont(fnt);
}
