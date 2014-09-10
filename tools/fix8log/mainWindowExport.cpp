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
#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxcellrange.h"
#include "xlsxworksheet.h"

#include "comboboxlineedit.h"
#include "editHighLighter.h"
#include "fix8sharedlib.h"
#include "fixmimedata.h"
#include "fixtoolbar.h"
#include "mainwindow.h"
#include "nodatalabel.h"
#include "worksheet.h"
#include "worksheetmodel.h"
#include "globals.h"
#include "lineedit.h"
#include "proxyFilter.h"
#include "tableschema.h"
#include "worksheetmodel.h"
#include <QQuickView>
#include <QtWidgets>
#include <QStandardItemModel>
void MainWindow::exportAsCSV(QString fileName,const WorkSheet *ws)
{
    QString str;
    bool bstatus;
    int i,j;
    int linecount = 0;
    QStandardItem *item;
    QModelIndex index;
    const ProxyFilter *proxyFilter = 0;
    const WorkSheetModel *model = 0;
    if (!ws || !ws->fixTable) {
        qWarning() << "ws is null or HAS NO FIX TABLE" << __FILE__ << __LINE__;
        GUI::ConsoleMessage msg("Export failed, work sheet is null or has no table",
                                GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        return;
    }
    QFileInfo fi(fileName);
    if (fi.suffix().toLower() != "csv")
        fileName.append(".csv");

    QFile file(fileName);
    QTextStream ts(&file);
    if (file.exists()) {
        str = fileName + " already exists.";
        QMessageBox::warning(this,"Export Failed",str);
        GUI::ConsoleMessage msg ("Export failed" + str,
                                 GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        return;

    }
    bstatus = file.open(QIODevice::WriteOnly);
    if (!bstatus) {
        str = "Unable to open file: " + fileName;
        QMessageBox::warning(this,"Export Failed",str);
        GUI::ConsoleMessage msg("Export failed" + str,
                                GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        return;
    }
    model = ws->fixTable->getWorkSheetModel();
    bool proxyInUse = ws->fixTable->proxyFilterInUse();
    if (proxyInUse) {
        proxyFilter = ws->fixTable->getProxyFilter();
        for (i=0;i<proxyFilter->rowCount(); i++ ) {
            QStringList itemList;
            for(j=0;j< proxyFilter->columnCount();j++) {
                index = proxyFilter->index(i,j);
                index = proxyFilter->mapToSource(index);
                item = model->itemFromIndex(index);
                if (item) {
                    itemList << item->text();
                }
                else {
                    itemList << " ";
                }

            }
            ts << itemList.join(',') << "\n";
        }
    }
    else {
        for (i=0;i<model->rowCount(); i++ ) {
            QStringList itemList;
            for(j=0;j< model->columnCount();j++) {
                index = model->index(i,j);
                item = model->itemFromIndex(index);
                if (item) {
                    itemList << item->text();
                }
                else {
                    itemList << " ";
                }

            }
            ts << itemList.join(',') << "\n";
        }
    }
    GUI::ConsoleMessage msg(QString("Exported to " +  fileName + ", " + QString::number(i) + " lines."));
    displayConsoleMessage(msg);
    file.close();
}
QTXLSX_USE_NAMESPACE

void MainWindow::exportAsXLSXA(QString fileName,WorkSheet *ws)
{
    QString str;
    QStringList headerArray;
    //headerArray << "A"<<"B"<<"C"<<"D"<<"E"<<"F"<<"G"<<"H"<<"I"<<"J"<<"K"<<"L"<<"M"<<"N"<<"O"<<"P"<<"Q"<<"R"<<"S"<<"T"<<"U"<<"V"<<"W"<<"X"<<"Y"<<"Z"<<"AA"<<"AB"<<"AC"<<"AD"<<"AE"<<"AF"<<"AG"<<"AH"<<"AI"<<"AJ"<<"AK"<<"AL"<<"AM"<<"AN"<<"AO"<<"AP"<<"AQ"<<"AR"<<"AS"<<"AT"<<"AU"<<"AV"<<"AW"<<"AX"<<"AY"<<"AZ";
    bool bstatus;
    int i,j;
    int linecount = 0;
    QStandardItem *item;
    QModelIndex index;
    const ProxyFilter *proxyFilter = 0;
    const WorkSheetModel *model = 0;
    if (!ws || !ws->fixTable) {
        qWarning() << "ws is null or HAS NO FIX TABLE" << __FILE__ << __LINE__;
        GUI::ConsoleMessage msg("Export failed, work sheet is null or has no table",
                                GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        return;
    }
    model = ws->fixTable->getWorkSheetModel();


    QFileInfo fi(fileName);
    if (fi.suffix().toLower() != "xlsx")
        fileName.append(".xlsx");
    Document xlsx;
    QString xmlFileName = ws->getFileName();
    xlsx.addSheet("From:" + xmlFileName);
    Format headerStyle;
    headerStyle.setFontBold(true);
    headerStyle.setHorizontalAlignment(Format::AlignHCenter);
    headerStyle.setVerticalAlignment(Format::AlignVCenter);
    int w;
    QFontMetrics fm(ws->fixTable->font());
    xlsx.setColumnWidth(1,model->columnCount() + 1,20);
    bool proxyInUse = ws->fixTable->proxyFilterInUse();
    for (i=0;i<model->columnCount();i++) {
        item = model->horizontalHeaderItem(i);
        xlsx.setColumnFormat(1,i,headerStyle);
        xlsx.write(1, i+1, item->text());
    }
    if (proxyInUse) {
        proxyFilter = ws->fixTable->getProxyFilter();
        for (i=0;i<proxyFilter->rowCount(); i++ ) {
            QStringList itemList;
            for(j=0;j< proxyFilter->columnCount();j++) {
                index = proxyFilter->index(i,j);
                index = proxyFilter->mapToSource(index);
                item = model->itemFromIndex(index);
                if (item) {
                  xlsx.write(i+2,j+1,item->text());
                }
            }
        }
    }
    else {
        for(i=0;i<model->rowCount();i++) {
            for(j=0;j< model->columnCount();j++) {
                index = model->index(i,j);
                item = model->itemFromIndex(index);
                if (item) {
                    xlsx.write(i+2,j+1,item->text());
                }

            }
        }
    }
    xlsx.saveAs(fileName);
    GUI::ConsoleMessage msg(QString("Exported to " +  fileName + ", " + QString::number(i) + " lines."));
    displayConsoleMessage(msg);
}
