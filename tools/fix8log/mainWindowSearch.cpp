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
#include "comboboxlineedit.h"
#include "fixmimedata.h"
#include "fixtoolbar.h"
#include "lineedit.h"
#include "mainwindow.h"
#include "nodatalabel.h"
#include "worksheet.h"
#include "worksheetmodel.h"
#include "globals.h"
#include "searchfunction.h"
#include "searchlineedit.h"
#include "tableschema.h"
#include <QQuickView>
#include <QtWidgets>
#include <QStandardItemModel>
#include <QtScript>
#include <QScriptSyntaxCheckResult>

void MainWindow::searchTextChangedSlot()
{
    haveSearchFunction = false;
    validateSearchText();
}
void MainWindow::validateSearchText()
{
    QString searchText = searchLineEdit->toPlainText();
    if (searchFunctionMap.contains(searchText)) {
        int index = searchFunctionMap.value(searchText);
        QVariant var = searchSelectCB->itemData(index);
        SearchFunction *sf = (SearchFunction *) var.value<void *>();
        searchSelectCB->setCurrentIndex(index);
        qDebug() << "Search Text found"<< __FILE__ << __LINE__;

    }
    else {
        // qDebug() << "Search Text not found"<< __FILE__ << __LINE__;
        searchSelectCB->setCurrentIndex(0);
    }
    validateSearchButtons();
}
/*
void MainWindow::setSearchColumnNames(QStringList columnNames)
{
    searchColumnNames = columnNames;

}
*/
SearchFunction  MainWindow::createSearchRoutine(bool &bstatus)
{
    QString str;
    QString strValue;
    QScriptValueList args;
    SearchFunction sf;
    searchArgList.clear();
    QString func = "function(";
    if (!tableSchema) {
        bstatus = false;
        qWarning() << "Error, table schame is null" << __FILE__ << __LINE__;
        return sf;
    }
    QString f = searchLineEdit->toPlainText();
    QString ff = f.simplified();
    if (!fieldUsePairList) {
        qWarning() << "FIELDS NOT SET FOR SEARCH" << __FILE__ << __LINE__;
        bstatus = false;
        return sf;
    }
    int mapCount = fieldUsePairList->count();
    for(int k=0;k<mapCount;k++) {
        QString fieldName = fieldUsePairList->at(k).first;
        if (ff.contains(fieldName)) {
            if (!searchArgList.contains(fieldName))
                searchArgList.append(fieldName);
        }
    }
    int rowCount = searchArgList.count();
    for(int i=0;i<rowCount;i++) {
        str  = searchArgList.at(i);
        func.append(str);
        if (i < rowCount -1) {
            func.append(",");
        }
    }
    func.append(")");
    func.append(" { return ");
    func.append(searchLineEdit->toPlainText());
    func.append(";}");
    sf.javascript = func;
    sf.function = searchLineEdit->toPlainText();
    QScriptSyntaxCheckResult::State syntaxState;
    QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(func);
    syntaxState =syntaxResult.state();
    if ( syntaxState == QScriptSyntaxCheckResult::Error) {
        qDebug() << "Syntax error";
        bstatus = false;
    }
    else {
        qDebug() << "OK";
        bstatus = true;
    }
    return sf;
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
    if (!haveSearchFunction) {
        searchFunction =  createSearchRoutine(bstatus);
        //ws->setSearchString(searchLineEdit->toPlainText());
        QScriptSyntaxCheckResult syntaxResult =
                engine.checkSyntax(searchFunction.javascript);
        syntaxState =syntaxResult.state();
        if ( syntaxState == QScriptSyntaxCheckResult::Error) {
            errorMessage = syntaxResult.errorMessage();
            GUI::ConsoleMessage msg(errorMessage,GUI::ConsoleMessage::ErrorMsg);
            displayConsoleMessage(msg);
            validateSearchButtons();
            return;
        }
        haveSearchFunction = true;
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
    SearchFunction newSearchFun = createSearchRoutine(bstatus);

    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Search Failed, work sheet is null" << __FILE__ << __LINE__;
        validateSearchButtons();
        update();
        return;
    }
    if (newSearchFun.function == searchFunction.function)
        return;
    ws->setSearchFunction(newSearchFun);
    searchFunction = newSearchFun;


    if (searchFunction.function.length() <  3) {
        ws->doSearch(WorkSheet::SearchOff);
        haveSearchFunction = false;
        validateSearchButtons();
        return;
    }
    else {
        haveSearchFunction = true;
    }
    bstatus = runSearchScript();
    if (bstatus) {
        if (tabW->count()  < 1) {
            qWarning() << "Search Failed, no work sheets" << __FILE__ << __LINE__;
            validateSearchButtons();
            update();
            return;
        }
        //Debug() << "Determine where to search from forward or backward based upon selected row
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
    QMessage *qmsg;
    QVariant var;
    QString arg;
    searchFunctionVal = engine.evaluate(searchFunction.javascript);
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
    qDebug() << "********************   FIX NEW SEARCH *******************" << __FILE__ << __LINE__;

    QVector <qint32> filterLogicalIndexes;
    for(int i=0;i<wsm->rowCount();i++) {
        args.clear();
        item = wsm->item(i,0);
        var  = item->data();
        QStringListIterator iter(searchArgList);
        qmsg = (QMessage *) var.value<void *>();
        while(iter.hasNext()) {
            arg = iter.next();
            if (qmsg->map.contains(arg)) {
                QMultiMap<QString,QVariant>::iterator miter = qmsg->map.find(arg);
                if (miter != qmsg->map.end())
                    qDebug() << i << ":" << item->text() << ", arg = " << arg << " value = " << miter.value();
            }
        }
        qmsg = (QMessage *) var.value<void *>();
        /*
        answer = searchFunction.call(QScriptValue(), args);
        if (answer.toBool()) {
            filterLogicalIndexes.append(i);
        }
        */
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
    if (tabW->count()  < 1)
        enableSearch = false;
    else {
        ws = qobject_cast <WorkSheet *> (tabW->currentWidget());
        if (!ws) {
            qWarning() << "Search Failed, work sheet is null" << __FILE__ << __LINE__;
            enableSearch = false;
        }
        else {
            wsm = ws->getModel();
            if (!wsm || (wsm->rowCount() < 1))
                enableSearch = false;
        }
    }
    if (enableSearch) {
        if (searchFunction.javascript.length() < 3)
            enableSearch = false;
        else {
            QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(searchFunction.javascript);
            syntaxState =syntaxResult.state();
            if ( syntaxState == QScriptSyntaxCheckResult::Error) {
                qDebug() << "Have Search Error" << __FILE__ << __LINE__;
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
void MainWindow::setSearchFunction(const SearchFunction &searchFunc)
{
    searchFunction  = searchFunc;
    searchLineEdit->setText(searchFunction.function);
    validateSearchButtons();
}
void MainWindow::setSearchFunctions(SearchFunctionList *sfl)
{
    populateSearchList(sfl);
}

void MainWindow::updateSearchFunctions(SearchFunctionList *sfl)
{

    bool updateFunction = false;
    QVariant var;
    SearchFunction *currentSearchFunction = 0;
    SearchFunction *newSearchFunction = 0;
    int currentIndex = searchSelectCB->currentIndex();
    if (currentIndex == 0) {
        validateSearchButtons();
        return;
    }
    var = searchSelectCB->itemData(currentIndex);
    if (var.isValid()) {
        currentSearchFunction = (SearchFunction *) var.value<void *>();
    }

    //searchSelectCB->clear();
    if (sfl && (sfl->count() > 0)  && currentSearchFunction) {
        newSearchFunction = sfl->findByID(currentSearchFunction->id);
        if (newSearchFunction) {
            if (newSearchFunction->id == currentSearchFunction->id){
                if (newSearchFunction->function != currentSearchFunction->function) {
                    updateFunction = true;
                    *currentSearchFunction = *newSearchFunction;
                }
            }
        }
        else {
            searchLineEdit->setText("");
            WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
            if (ws){
                ws->doSearch(WorkSheet::SearchOff);
            }
        }
    }
    validateSearchButtons();
    populateSearchList(sfl);
    if (updateFunction) {
        searchLineEdit->setText(newSearchFunction->function);
        searchReturnSlot();
    }
    else if (currentSearchFunction) {
        disconnect(searchSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(searchFunctionSelectedSlot(int)));
        searchLineEdit->setText(currentSearchFunction->function);
        int index = searchFunctionMap.value(currentSearchFunction->function);
        searchSelectCB->setCurrentIndex(index);
        connect(searchSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(searchFunctionSelectedSlot(int)));
    }

}
void MainWindow::populateSearchList(SearchFunctionList *sfl)
{
    int index = 1;

    SearchFunction *sf;
    disconnect(searchSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(searchFunctionSelectedSlot(int)));

    searchFunctionMap.clear();
    searchSelectCB->clear();
    if (!sfl || sfl->count() < 1) {
        connect(searchSelectCB,SIGNAL(currentIndexChanged(int)),SLOT(searchFunctionSelectedSlot(int)));
        return;
    }

    if (searchFunctionList.count() > 0) {
        qDeleteAll(searchFunctionList.begin(),searchFunctionList.end());
    }
    searchFunctionList = *sfl;
    QListIterator <SearchFunction *> iter(searchFunctionList);
    searchSelectCB->addItem("Select Function");
    while(iter.hasNext()) {
        sf = iter.next();
        QVariant var;
        var.setValue((void *) sf);

        searchSelectCB->addItem(sf->alias,var);
        searchFunctionMap.insert(sf->function,index);
        index++;
    }
    connect(searchSelectCB,SIGNAL(currentIndexChanged(int)),SLOT(searchFunctionSelectedSlot(int)));
}
void MainWindow::saveSearchStringSlot()
{
    emit showSearchDialogAddMode(searchLineEdit->toPlainText());
}
void MainWindow::searchFunctionSelectedSlot(int index)
{
    if (index == 0)
        searchLineEdit->setText("");
    else {
        QVariant var = searchSelectCB->itemData(index);
        if (var.isValid()) {
            SearchFunction *sf = (SearchFunction *) var.value<void *>();
            searchLineEdit->setText(sf->function);
            searchReturnSlot();
        }
    }
}
