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

#include <memory>
#include "messagefield.h"
#include "worksheet.h"
#include "dateTimeDelegate.h"
#include "fixHeaderView.h"
#include "fixtableverticaheaderview.h"
#include "fixmimedata.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include "tableschema.h"
#include "worksheetmodel.h"
#include <QDebug>
#include <QMap>
#include <QQuickView>
#include <QtWidgets>
#include <iostream>
#include <string.h>
#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
using namespace FIX8;

void WorkSheet::setSearchIndexes(const QVector<qint32> &indexes)
{
    //qDebug() << "Work Sheet, set vertical headers to " << indexes << __FILE__ << __LINE__;
    searchLogicalIndexes = indexes;
    FixTableVerticaHeaderView *fvh = fixTable->getFixVerticalHeader();
    fvh->setHighlightList(searchLogicalIndexes);
    fvh->repaint();
    fvh->update();
    fixTable->viewport()->repaint();
    fixTable->viewport()->update();
}
QVector <qint32> WorkSheet::getSearchIndexes()
{
    return searchLogicalIndexes;
}
quint32 WorkSheet::doSearch(SearchType st)
{
    QMessage *message = 0;
    quint32 returnCode =  SearchEmpty;

    if (st == SearchOff) {
        fixTable->setSearchFilterOn(false);
        return SearchOk;
    }
    else if (st == ResumeSearch) {
        fixTable->setSearchFilterOn(true);
         return SearchOk;
    }
    if (!_model || (_model->rowCount() < 1))
        return SearchEmpty;

    if (searchLogicalIndexes.isEmpty())
        return SearchEmpty;


    if (!sm) {
        qWarning() << "Error  - selection model is null" << __FILE__ << __LINE__;
        return SearchEmpty;
    }
    QModelIndex index;
    int previousRow = -1;
    int nextRow     = -1;
    for(int i=0; i<searchLogicalIndexes.count(); i++){
        if (currentRow > searchLogicalIndexes[i])
            previousRow = searchLogicalIndexes[i];
    }
    for(int i=searchLogicalIndexes.count()-1;i>=0; i--){
        //qDebug() << "\tCurrent Row: " << currentRow;
        if (currentRow < searchLogicalIndexes[i]) {
            nextRow = searchLogicalIndexes[i];
            //qDebug() << "\tNext Row set to " << nextRow;
        }
    }
    QString h2;
    qint32 newCurrentRow = currentRow;
    switch(st) {
    case WorkSheet::SearchFirst:
        newCurrentRow = searchLogicalIndexes[0];
        if (searchLogicalIndexes.count() < 2)
            returnCode = SearchFinished;
        else
            returnCode = SearchHasNext;
        break;
    case WorkSheet::SearchBack:
        if (previousRow > -1) {
            newCurrentRow = previousRow;
            if (previousRow != searchLogicalIndexes[0])
                returnCode = SearchHasPrevious;
            if (searchLogicalIndexes.count() >1) {
                returnCode |= SearchHasNext;
            }
        }
        break;
    case WorkSheet::SearchNext:         
        if (nextRow > -1) {
            newCurrentRow = nextRow;
            if (nextRow != searchLogicalIndexes.last()) {
                returnCode |= SearchHasNext;
            }
            if (searchLogicalIndexes.count() >1 )
                returnCode |= SearchHasPrevious;
        }
        else if (searchLogicalIndexes.count() > 1) {
            returnCode |= SearchHasPrevious;
        }
        break;
    case WorkSheet::SearchLast:
        newCurrentRow = searchLogicalIndexes.last();
        if (searchLogicalIndexes.count() < 2)
            returnCode = SearchFinished;
        else
            returnCode = SearchHasPrevious;
        break;
    default:
        qWarning() << "Unknown search request" << __FILE__ << __LINE__;
        return SearchEmpty;
    }
    // if (newCurrentRow != currentRow) {
    // fixTable->selectRow( QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
    sm->setCurrentIndex(_model->index(newCurrentRow,1),QItemSelectionModel::Select);
    fixTable->selectRow(newCurrentRow);
    for (int i=0;i<  _model->columnCount();i++) {
        // loop over all to find valid variant, but should get it on first try
        QModelIndex otherIndex = _model->index(newCurrentRow,0);
        if (otherIndex.isValid()) {
            QVariant var = _model->data(otherIndex,Qt::UserRole + 1);
            if (var.isValid()) {
                message = (QMessage *) var.value<void *>();
                messageArea->setMessage(message);
                break;
            }
        }
    }

    //fixTable->setCurrentIndex(_model->index(newCurrentRow,0));

    currentRow = newCurrentRow;
    //}

    QString hex;
    hex = QString::number(returnCode,16);
    qDebug() << ">>>>>>>>>>>return code:" << hex;
    //qDebug() << "CHECK Selected Row  = " << fixTable-> << __FILE__ << __LINE__;
    return returnCode;
}
void WorkSheet::setSearchFunction(const SearchFunction &sf)
{
    searchFunction = sf;
}

SearchFunction &WorkSheet::getSearchFunction()
{
    return searchFunction;
}
