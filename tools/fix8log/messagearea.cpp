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

#include "messagearea.h"
#include "messagefield.h"

#include <QtWidgets>

MessageArea::MessageArea(QWidget *parent) :
    QWidget(parent)
{
    stackLayout = new QStackedLayout();
    setLayout(stackLayout);
    model = new QStandardItemModel(this);
    QStringList headerLabels;
    headerLabels << "Field" <<  "Name" << "Value";
    model->setHorizontalHeaderLabels(headerLabels);
    QWidget *workArea = new QWidget(this);
    QVBoxLayout *wBox = new QVBoxLayout();
    workArea->setLayout(wBox);
    tableView = new QTableView(workArea);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setModel(model);
    tableView->verticalHeader()->setVisible(false);
    QHeaderView *horHeader = tableView->horizontalHeader();
    horHeader->setSectionResizeMode(QHeaderView::Interactive);
    horHeader->setStretchLastSection(true);
    horHeader->setSectionsMovable(true);
    horHeader->setSortIndicatorShown(true);

    infoArea = new QWidget(workArea);
    QFont fnt = infoArea->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize()+2);
    infoArea->setFont(fnt);
    QPalette pal = infoArea->palette();
    QColor fgColor = pal.color(QPalette::WindowText);
    QColor bgColor = pal.color(QPalette::Window);
    pal.setColor(QPalette::WindowText,bgColor);
    pal.setColor(QPalette::Window,fgColor);
    infoArea->setAutoFillBackground(true);
    infoArea->setPalette(pal);
    QFormLayout *infoForm = new QFormLayout();
    infoArea->setLayout(infoForm);
    seqNumV = new QLabel(infoArea);
    seqNumV->setAlignment(Qt::AlignCenter);
    messageTypeV = new QLabel(infoArea);
    messageTypeV->setAlignment(Qt::AlignCenter);
    infoForm->addRow("Seq Num",seqNumV);
    infoForm->addRow("Mesg Type",messageTypeV);
    wBox->addWidget(tableView,1);
    wBox->addWidget(infoArea,0,Qt::AlignBottom);
    stackLayout->insertWidget(0,workArea);
}
void MessageArea::setMessageFieldList(MessageFieldList *mfl,int seqNum, QString &msgType)
{
    seqNumV->setText(QString::number(seqNum));
    messageTypeV->setText(msgType);
    model->removeRows(0,model->rowCount());
    if (!mfl || (mfl->count() < 1))
        return;
    QListIterator <MessageField> iter(*mfl);
    while(iter.hasNext()) {
        QList<QStandardItem *> itemList;
        MessageField mf = iter.next();
        QString str1 = QString::number(mf.id);
        // Do we need to delete these items after clear to avoid memory leak ?
        QStandardItem *item1 = new QStandardItem(str1);
        QStandardItem *item2 = new QStandardItem(mf.name);
        QStandardItem *item3 = new QStandardItem();
        if (mf.variant.isValid()) {
            switch (mf.variant.type()) {
            case QVariant::Double:
                item3->setText(QString::number(mf.variant.toDouble()));
                        break;
            case QVariant::String:
                item3->setText(mf.variant.toString());
                break;
            case QVariant::Int:
                item3->setText(QString::number(mf.variant.toInt()));
                        break;
            default:
                qWarning() << "Unknown data type" << __FILE__ << __LINE__;
                item3->setText(mf.variant.toString());
            }
        }
        itemList.append(item1);
        itemList.append(item2);
        itemList.append(item3);
        model->appendRow(itemList);
    }
}
