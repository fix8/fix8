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

#include "fixmimedata.h"
#include "fixtoolbar.h"
#include "lineedit.h"
#include "mainwindow.h"
#include "nodatalabel.h"
#include "worksheet.h"
#include "worksheetmodel.h"
#include "globals.h"
#include "searchlineedit.h"
#include "tableschema.h"
#include <QQuickView>
#include <QtWidgets>
#include <QStandardItemModel>
#include <QtScript>
#include <QScriptSyntaxCheckResult>

void MainWindow::searchTextChangedSlot()
{
    haveSearchString = false;

}
void MainWindow::setSearchColumnNames(QStringList columnNames)
{
    searchColumnNames = columnNames;

}
QString MainWindow::createSearchRoutine(bool &bstatus)
{
    QString str;
    QString strValue;
    QString func = "(function(";
    if (!tableSchema) {
        bstatus = false;
        qWarning() << "Error, table schame is null" << __FILE__ << __LINE__;
        return str;
    }
    int rowCount = tableSchema->fieldNames.count();

    for(int i=0;i<rowCount;i++) {
        str  = tableSchema->fieldNames[i];
        func.append(str);
        if (i < rowCount -1) {
            func.append(",");
        }
    }
    func.append(") {return ");
    func.append(searchLineEdit->toPlainText());
    func.append(";})");
    bstatus = true;
    return func;
}
void MainWindow::searchActionSlot(QAction *action)
{
    WorkSheet *ws = 0;
    QString errorMessage;
    QScriptSyntaxCheckResult::State syntaxState;
    bool bstatus;

    if (tabW->count()  < 1) {
        qWarning() << "Search Failed, no work sheets" << __FILE__ << __LINE__;
        return;
    }
    ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Search Feailed, work sheet is null" << __FILE__ << __LINE__;
        return;
    }
    if (!haveSearchString) {
        searchString =  createSearchRoutine(bstatus);
        ws->setSearchString(searchLineEdit->toPlainText());
        //qDebug() << "Search String = " << searchString << __FILE__ << __LINE__;
        QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(searchString);
        syntaxState =syntaxResult.state();
        if ( syntaxState == QScriptSyntaxCheckResult::Error) {
            errorMessage = syntaxResult.errorMessage();
            GUI::ConsoleMessage msg(errorMessage,GUI::ConsoleMessage::ErrorMsg);
            displayConsoleMessage(msg);
            validateSearchButtons();
            return;
        }
        haveSearchString = true;
    }
    runSearchScript();
    WorkSheet::SearchType searchType;
    if (action == searchNextA)
        searchType = WorkSheet::SearchNext;
    else if (action == searchBackA)
        searchType = WorkSheet::SearchBack;
    else if (action == searchEndA)
        searchType = WorkSheet::SearchLast;
    else
        searchType = WorkSheet::SearchFirst;

    quint32  searchStatus  = ws->doSearch(searchType);
    validateSearchButtons(searchStatus,ws);
}
void MainWindow::searchReturnSlot()
{
    // Return Key Pressed
    bool bstatus;
    QString errorMessage;
    QScriptSyntaxCheckResult::State syntaxState;
    QString newSearchStr = createSearchRoutine(bstatus);
    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Search Failed, work sheet is null" << __FILE__ << __LINE__;
        validateSearchButtons();
        update();
        return;
    }
    if (newSearchStr == searchString)
        return;
    ws->setSearchString(searchLineEdit->toPlainText());
    searchString = newSearchStr;
    QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(searchString);
    syntaxState =syntaxResult.state();
    if ( syntaxState == QScriptSyntaxCheckResult::Error) {
        errorMessage = syntaxResult.errorMessage();
        GUI::ConsoleMessage msg(errorMessage,GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        haveSearchString = false;
        validateSearchButtons();
        return;
    }
    else {
        haveSearchString = true;
    }
    bstatus = runSearchScript();
    if (bstatus) {
        if (tabW->count()  < 1) {
            qWarning() << "Search Failed, no work sheets" << __FILE__ << __LINE__;
            validateSearchButtons();
            update();
            return;
        }
        // qDebug() << "Determine where to search from forward or backward based upon selected row
        WorkSheet::SearchType searchType;
        searchType = WorkSheet::SearchNext;
        quint32  searchStatus  = ws->doSearch(searchType);
        validateSearchButtons(searchStatus,ws);
    }
}
bool MainWindow::runSearchScript()
{
    QScriptValueList args;
    QScriptValue answer;
    QStandardItem *item;
    searchFunction = engine.evaluate(searchString);
    if (tabW->count()  < 1) {
        qWarning() << "Search Failed, no work sheets" << __FILE__ << __LINE__;
        validateSearchButtons();
        update();
        return false;
    }
    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Search Failed, work sheet is null" << __FILE__ << __LINE__;
        validateSearchButtons();
        update();
        return false;
    }
    WorkSheetModel *wsm = ws->getModel();
    if (!wsm || (wsm->rowCount() < 1)) {
        qWarning() << "Search Failed, work sheet model is null, or has no rows" << __FILE__ << __LINE__;
        validateSearchButtons();
        update();
        return false;
    }
    QVector <qint32> filterLogicalIndexes;
    for(int i=0;i<wsm->rowCount();i++) {
        args.clear();
        for(int j=0;j<wsm->columnCount();j++) {
            item = wsm->item(i,j);
            args <<item->text();
        }
        answer = searchFunction.call(QScriptValue(), args);
        if (answer.toBool()) {
            filterLogicalIndexes.append(i);
        }
    }
    ws->setSearchIndexes(filterLogicalIndexes);
    validateSearchButtons();
    update();
    return true;
}
void MainWindow::validateSearchButtons()
{
    QScriptSyntaxCheckResult::State syntaxState;
    WorkSheet *ws;
    WorkSheetModel *wsm;
    bool enableSearch = true;
    qDebug() << "VALIDATE SEARCH() " << __FILE__ << __LINE__;
    if (tabW->count()  < 1)
        enableSearch = false;
    else {
        ws = qobject_cast <WorkSheet *> (tabW->currentWidget());
        if (!ws) {
            qWarning() << "Search Feailed, work sheet is null" << __FILE__ << __LINE__;
            enableSearch = false;
        }
        else {
            wsm = ws->getModel();
            if (!wsm || (wsm->rowCount() < 1))
                enableSearch = false;
        }
    }
    if (enableSearch) {
        if (searchString.length() < 3)
            enableSearch = false;
        else {
            QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(searchString);
            syntaxState =syntaxResult.state();
            if ( syntaxState == QScriptSyntaxCheckResult::Error) {
             qDebug() << "HAve Serch Error" << __FILE__ << __LINE__;
                enableSearch = false;
            }
        }
    }
    searchBackA->setEnabled(enableSearch);
    searchBeginA->setEnabled(enableSearch);
    searchNextA->setEnabled(enableSearch);
    searchEndA->setEnabled(enableSearch);
}
void MainWindow::validateSearchButtons(quint32 searchStatus, WorkSheet *ws)
{
    bool enableSearch = false;
    if (searchStatus & WorkSheet::SearchHasPrevious)
        enableSearch = true;
    searchBackA->setEnabled(enableSearch);
    searchBeginA->setEnabled(enableSearch);

    enableSearch = false;
    if (searchStatus & WorkSheet::SearchHasNext) {
        enableSearch = true;
    }
    searchNextA->setEnabled(enableSearch);
    searchEndA->setEnabled(enableSearch);
}
void MainWindow::rowSelectedSlot(int row)
{
    qDebug() << "Main WIndow row selected " << row << __FILE__<< __LINE__;
    validateSearchButtons();
}
void MainWindow::searchToolbarVisibleSlot(bool visible)
{
    WorkSheet *workSheet;
    for(int i=0;i < tabW->count();i++) {
        workSheet = qobject_cast <WorkSheet *> (tabW->widget(i));
        if (!visible)
            workSheet->doSearch(WorkSheet::SearchOff);
        else
            workSheet->doSearch(WorkSheet::ResumeSearch);
    }
}
void MainWindow::setSearchString(const QString &searchStr)
{
    searchLineEdit->setText(searchStr);
    validateSearchButtons();
}
