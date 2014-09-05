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
#include <stdio.h>

void MainWindow::filterTextChangedSlot()
{
    haveFilterFunction = false;
    validateFilterText();
}
void MainWindow::validateFilterText()
{
    QString filterText = filterLineEdit->toPlainText();
    if (filterFunctionMap.contains(filterText)) {
        int index = filterFunctionMap.value(filterText);
        QVariant var = filterSelectCB->itemData(index);
         SearchFunction *sf = (SearchFunction *) var.value<void *>();
        filterSelectCB->setCurrentIndex(index);
    }
    else {
        filterSelectCB->setCurrentIndex(0);
    }
}
void MainWindow::filterReturnSlot()
{
    // Return Key Pressed
    bool bstatus;
    QString errorMessage;
    QVector <qint32> filterLogicalIndexes;
    QScriptSyntaxCheckResult::State syntaxState;
    SearchFunction newfilterFunction = createRoutine(bstatus,false);
    if (newfilterFunction == filterFunction) {
        return;
    }
    else {
        filterFunction = newfilterFunction;
        if (bstatus == false) {
            return;
        }
    }
    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Filter Failed, work sheet is null" << __FILE__ << __LINE__;
        update();
        return;
    }
    WorkSheetData::FilterMode filterMode = (WorkSheetData::FilterMode) filterButtonGroup->checkedId();
    if (filterFunction.function.length() <  3) {
        haveFilterFunction = false;
        return;
    }
    else {
        haveFilterFunction = true;
    }
   runFilterScript();
}
void MainWindow::setFilterFunctions(SearchFunctionList *sfl)
{
    populateFilterList(sfl);
}
void MainWindow::updateFilterFunctions(SearchFunctionList *sfl)
{
    bool updateFunction = false;
    bool resetToZero = false;
    QVariant var;
    SearchFunction *currentFilterFunction = 0;
    SearchFunction *newSearchFunction = 0;
    int currentIndex = filterSelectCB->currentIndex();
    if (currentIndex == 0) {
        resetToZero = true;
    }
    var = filterSelectCB->itemData(currentIndex);
    if (var.isValid()) {
        currentFilterFunction = (SearchFunction *) var.value<void *>();
    }
    if (sfl && (sfl->count() > 0)  && currentFilterFunction) {
        newSearchFunction = sfl->findByID(currentFilterFunction->id);
        if (newSearchFunction) {
            if (newSearchFunction->id == currentFilterFunction->id){
                if (newSearchFunction->function != currentFilterFunction->function) {
                    updateFunction = true;
                    *currentFilterFunction = *newSearchFunction;
                }
            }
        }
        else {
            filterLineEdit->setText("");
            WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
            if (ws){
                qDebug() << "TODO FILTER TO WORKS SHEET" << __FILE__ << __LINE__;
            }
        }
    }
    populateFilterList(sfl);
    if (updateFunction) {
        filterLineEdit->setText(newSearchFunction->function);
        filterReturnSlot();
    }
    else if (currentFilterFunction) {
        disconnect(filterSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(filterFunctionSelectedSlot(int)));
        filterLineEdit->setText(currentFilterFunction->function);
        int index = filterFunctionMap.value(currentFilterFunction->function);
        filterSelectCB->setCurrentIndex(index);
        connect(filterSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(filterFunctionSelectedSlot(int)));
    }
    else if (resetToZero) {
        filterSelectCB->setCurrentIndex(0);
    }
}
void MainWindow::populateFilterList(SearchFunctionList *sfl)
{
    int index = 1;
    SearchFunction *sf;
    disconnect(filterSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(filterFunctionSelectedSlot(int)));
    filterFunctionMap.clear();
    filterSelectCB->clear();
    if (!sfl || sfl->count() < 1) {
        connect(filterSelectCB,SIGNAL(currentIndexChanged(int)),SLOT(filterFunctionSelectedSlot(int)));
        return;
    }
    if (filterFunctionList.count() > 0) {
        qDeleteAll(filterFunctionList.begin(),filterFunctionList.end());
    }
    filterFunctionList = *sfl;
    QListIterator <SearchFunction *> iter(filterFunctionList);
    filterSelectCB->addItem("Select Function");
    while(iter.hasNext()) {
        sf = iter.next();
        QVariant var;
        var.setValue((void *) sf);
        filterSelectCB->addItem(sf->alias,var);
        filterFunctionMap.insert(sf->function,index);
        index++;
    }
    connect(filterSelectCB,SIGNAL(currentIndexChanged(int)),SLOT(filterFunctionSelectedSlot(int)));
}
void MainWindow::saveFilterStringSlot()
{
    emit showFilterDialogAddMode(filterLineEdit->toPlainText());
}
void MainWindow::filterFunctionSelectedSlot(int index)
{
    bool bstatus;
     WorkSheetData::FilterMode filterMode;
    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Filter Failed, work sheet is null" << __FILE__ << __LINE__;
        update();
        return;
    }
    if (index == 0) {
        filterLineEdit->setText("");
         ws->wipeFilter();
    }
    else {
        QVariant var = filterSelectCB->itemData(index);
        if (var.isValid()) {
            SearchFunction *sf = (SearchFunction *) var.value<void *>();
            if (!sf) {
                qWarning() << "No Filter  Function " << __FILE__ << __LINE__;
                return;
            }
            filterLineEdit->setText(sf->function);
            if (*sf == filterFunction) {
                return;

            }
            filterFunction = *sf;
            filterMode  = (WorkSheetData::FilterMode) filterButtonGroup->checkedId();
            filterFunction = createRoutine(bstatus,false);
            if (filterFunction.function.length() <  3) {
                haveFilterFunction = false;
                return;
            }
            else {
                haveFilterFunction = true;
            }
        }
    }
    if (index != 0)
         runFilterScript();
}
void MainWindow::filterToolbarVisibleSlot(bool visible)
{
    WorkSheet *workSheet;
    for(int i=0;i < tabW->count();i++) {
        workSheet = qobject_cast <WorkSheet *> (tabW->widget(i));
        if (!visible)
            workSheet->setFilterMode(WorkSheetData::Off);
        else {
            WorkSheetData::FilterMode filterMode = (WorkSheetData::FilterMode) filterButtonGroup->checkedId();
            workSheet->setFilterMode(filterMode);
        }
    }
}
void MainWindow::filterModeChangedSlot(int fm)
{
    WorkSheetData::FilterMode filterMode = (WorkSheetData::FilterMode) fm;
    bool bstatus;
    QString errorMessage;
    QScriptSyntaxCheckResult::State syntaxState;
    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Filter Failed, work sheet is null" << __FILE__ << __LINE__;
        update();
        return;
    } 
    SearchFunction newfilterFunction = createRoutine(bstatus,false);
    if ( filterFunction == newfilterFunction) {
        qDebug() << "FILTERS ARE THE SAME" << __FILE__ <<__LINE__;
        ws->setFilterMode(filterMode);
    }
    else {
        qDebug() << "DIFFFRENT FILTERS RUN AGAIN...";
        runFilterScript();
    }
    update();
}
bool MainWindow::runFilterScript()
{
    bool  skip = false;
    QScriptValueList args;
    QScriptValue answer;
    QStandardItem *item;
    QMessage *qmsg;
    QVariant var,var1;
    QString arg;
    QVector <qint32> filterLogicalIndexes;
    filterFunctionVal = engine.evaluate(filterFunction.javascript);
    if (tabW->count()  < 1) {
        qWarning() << "Filter Failed, no work sheets" << __FILE__ << __LINE__;
        update();
        return false;
    }
    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        qWarning() << "Search Failed, work sheet is null" << __FILE__ << __LINE__;
        update();
        return false;
    }
    if (filterArgList.count() < 1) {
        qWarning() << "No filter arguments provided " << __FILE__ << __LINE__;
        ws->setSearchIndexes(filterLogicalIndexes); // no indexes
        return false;
    }
    WorkSheetModel *wsm = ws->getModel();
    if (!wsm || (wsm->rowCount() < 1)) {
        qWarning() << "Filter Failed, work sheet model is null, or has no rows" << __FILE__ << __LINE__;
        update();
        return false;
    }
    int row=0;
    int numOfFilterArguments = filterArgList.count();
    QStringListIterator iter(filterArgList);
    for(int i=0;i<wsm->rowCount();i++) {
        skip = false;
        args.clear();
        item = wsm->item(i,0);
        var  = item->data();
        qmsg = (QMessage *) var.value<void *>();
        QVariantList **variantLists;
        iter.toFront();
        variantLists = new QVariantList *[numOfFilterArguments];
        // gets list of all values of messages that apply to search arguments
        int skipPoint = 0;
        for(int j=0;j<numOfFilterArguments;j++) {
            variantLists[j] = new QVariantList();
            arg = filterArgList.at(j);
            if (qmsg->map.contains(arg)) {
                QMultiMap<QString,QVariant>::iterator miter = qmsg->map.find(arg);
                while (miter != qmsg->map.end() &&  miter.key() == arg) {
                    var1 = miter.value();
                    variantLists[j]->append(var1);
                    miter++;
                }
            }
            else {
                skipPoint = j;
                skip = true;
                break; // nbsps
            }
        }
        if (skip) {
            for (int j=0;j< skipPoint;j++) {
                delete  variantLists[j];
            }
            delete [] variantLists;
        }
        else  {
            QVariant **vector=0;
            int totalSize = 1;
            int repeatLength=1;
            //iteratate over rows, then columns
            for(int ii=0;ii<numOfFilterArguments;ii++)
                totalSize = totalSize* variantLists[ii]->count();
            vector = new QVariant*[totalSize]; // double array
            for(int ii=0;ii< numOfFilterArguments;ii++)
                vector[ii] = new QVariant[numOfFilterArguments];
            repeatLength= 1;
            for(int ii=0;ii<numOfFilterArguments;ii++) {
                if (variantLists[ii]->count() ==0 ) {
                    qWarning() << "ERROR NO VALUES IN VECTOR" << __FILE__ << __LINE__;
                }
                else {
                    repeatLength = totalSize/(repeatLength * (variantLists[ii]->count()));
                    int k = 0;
                    int m = 0;
                    for(int j=0;j<totalSize;j++) {
                        vector[j][ii]  = variantLists[ii]->at(k);
                        m++; // should tis be incremented at end of loop
                        if (m >= repeatLength) {
                            m = 0;
                            k++;
                            if (k > variantLists[ii]->count())
                                k = 0;
                        }
                    }
                }
            }
            QVariantList vargs;
            QVariant mvar;
            for (int ii=0;ii<totalSize;ii++) {
                args.clear();
                vargs.clear();
                for (int j=0;j< filterArgList.count();j++) {
                    mvar = vector[ii][j];
                    switch (mvar.type()) {
                    case QVariant::Int:
                        args <<  mvar.toInt();
                        vargs << mvar;
                        break;
                    case QVariant::Double:
                        args <<  mvar.toDouble();
                        vargs << mvar;
                        break;
                    case QVariant::String:
                        args <<  mvar.toString();
                        vargs << mvar;
                        break;
                    default:
                        qWarning() << "Unknown variarnt type in message" << __FILE__ << __LINE__;
                    }
                }
                answer = filterFunctionVal.call(QScriptValue(), args);
                if (answer.toBool()) {
                    filterLogicalIndexes.append(row);
                    break;
                }
            }
            for (int ii=0;ii<numOfFilterArguments;ii++) {
                delete []vector[ii];
            }
            delete []vector;
            vector = 0;
        }
        row++;
    }
    WorkSheetData::FilterMode filterMode = (WorkSheetData::FilterMode) filterButtonGroup->checkedId();
    ws->setFilterIndexes(filterLogicalIndexes,filterMode); // add mode to this
    update();
    return true;
}
