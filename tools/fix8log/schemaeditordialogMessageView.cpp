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
#include "schemaeditordialog.h"
#include "schemadelegate.h"
#include "schemaitem.h"
#include "database.h"
#include "globals.h"
#include <QCloseEvent>

using namespace GUI;
void SchemaEditorDialog::buildMessageView()
{
    messageView = new QWidget(centralStack);
    QVBoxLayout *vbox = new QVBoxLayout(messageView);
    messageView->setLayout(vbox);
    // target window at top of main widget
    targetArea = new QWidget(centralStack);
    QHBoxLayout *tarBox = new QHBoxLayout(targetArea);
    targetArea->setLayout(tarBox);
    QFont fnt = targetArea->font();
    fnt.setBold(true);
    targetArea->setFont(fnt);

    windowL = new QLabel("Window:",targetArea);
    windowL->setFont(fnt);
    windowV = new QLabel("",targetArea);
    windowV->setAlignment(Qt::AlignLeft);
    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setItalic(true);
    windowV->setFont(fnt);
    QFontMetrics fm1(fnt);
    windowV->setMinimumWidth(fm1.maxWidth()*8);
    tarBox->addWidget(windowL,0,Qt::AlignLeft);
    tarBox->addWidget(windowV,1);

    QHBoxLayout *topBox = new QHBoxLayout();
    topBox->addWidget(targetArea,2);
    statusArea = new QWidget(targetArea);
    QHBoxLayout *sbox = new QHBoxLayout(statusArea);
    statusArea->setLayout(sbox);
    sbox->setMargin(0);
    statusI = new QLabel(targetArea);
    statusL = new QLabel(targetArea);
    statusL->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    statusL->setFont(fnt);
    statusL->setText("");
    sbox->addWidget(statusI);
    sbox->addWidget(statusL);
    topBox->addWidget(statusArea,1,Qt::AlignRight);

    splitter = new QSplitter(Qt::Horizontal,this);
    buildSchemaArea();
    QWidget *workWidget = new QWidget(splitter);
    QGridLayout *wgrid = new QGridLayout(workWidget);
    workWidget->setLayout(wgrid);

    splitter->insertWidget(0,schemaArea);
    splitter->insertWidget(1,workWidget);
    splitter->setChildrenCollapsible(true);

    messageListL  = new QLabel("Messages");
    messageListL->setToolTip("All possible FIX messages");
    availableListL = new QLabel("Available Fields");
    availableListL->setToolTip("All fields associated with selected message");
    selectedListL = new QLabel("Selected Fields");
    selectedListL->setToolTip("Fields that define table(s) headers");
    selectedListL->setAlignment(Qt::AlignHCenter);
    messageListL->setAlignment(Qt::AlignHCenter);
    availableListL->setAlignment(Qt::AlignHCenter);
    selectedListL->setAlignment(Qt::AlignHCenter);
    QWidget *messageArea = new QWidget(this);
    QVBoxLayout *mbox = new QVBoxLayout(messageArea);
    messageArea->setLayout(mbox);
    mbox->setMargin(0);
    messageListTreeView = new QTreeView(this);
    messageSpacerItem = new QSpacerItem(22,32);
    mbox->addWidget(messageListTreeView,1);
    mbox->addSpacerItem(messageSpacerItem);

    messageModel = new QStandardItemModel(messageListTreeView);
    messageListTreeView->setModel(messageModel);
    messageDelegate = new MessageItemDelegate(messageListTreeView);
    messageListTreeView->setItemDelegateForColumn(0,messageDelegate);
    QStringList messageListHeaders;
    messageListHeaders << "Name";
    messageModel->setHorizontalHeaderLabels(messageListHeaders);
    messageListTreeView->setSortingEnabled(true);
    messageListTreeView->setUniformRowHeights(true);
    connect(messageListTreeView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(messageListClickedSlot(QModelIndex)));
    QWidget *availableArea = new QWidget(this);
    QVBoxLayout *avbox = new QVBoxLayout(availableArea);
    avbox->setMargin(0);
    availableArea->setLayout(avbox);

    availableFieldsTreeView = new QTreeView(this);
    availableFieldsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QWidget *availableButtonArea = new QWidget(availableArea);
    QHBoxLayout *abox = new QHBoxLayout(availableButtonArea);
    availableButtonArea->setLayout(abox);
    abox->setMargin(0);
    expandBG = new QButtonGroup(this);
    QIcon expandIcon;
    expandIcon.addPixmap(QPixmap(":/images/svg/buttonOn.svg"),
                         QIcon::Normal,
                         QIcon::On);
    expandIcon.addPixmap(QPixmap(":/images/svg/empty.svg"),
                         QIcon::Normal,
                         QIcon::Off);
    expandBG->setExclusive(false);
    expandPB = new QPushButton("Expand",availableButtonArea);
    QRect rect;
    rect.setX(0);
    rect.setY(0);
    rect.setWidth(10);
    rect.setHeight(expandPB->height());
    messageSpacerItem->setGeometry(rect);
    collapsePB = new QPushButton("Collapse",availableButtonArea);
    expandPB->setIcon(expandIcon);
    collapsePB->setIcon(expandIcon);
    expandPB->setCheckable(true);
    collapsePB->setCheckable(true);
    expandPB->setToolTip("Expand All Tree Items");
    collapsePB->setToolTip("Collapse All Tree Items");
    expandBG->addButton(expandPB);
    expandBG->addButton(collapsePB);
    connect(expandPB,SIGNAL(toggled(bool)),this,SLOT(expandAllSlot(bool)));
    connect(collapsePB,SIGNAL(toggled(bool)),this,SLOT(collapseAllSlot(bool)));

    abox->addWidget(expandPB,0);
    abox->addWidget(collapsePB,0);
    avbox->addWidget(availableFieldsTreeView,1);
    avbox->addWidget(availableButtonArea,0,Qt::AlignLeft);
    //connect(availableFieldsTreeView,SIGNAL(clicked(QModelIndex)),
    //        this,SLOT(availableFieldsTreeViewClickedSlot(QModelIndex)));

    availableFieldModel = new QStandardItemModel(availableFieldsTreeView);
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    availableFieldModel->setColumnCount(1);
    availableFieldHeaderItem = new QStandardItem("Fields");
    availableFieldModel->setHorizontalHeaderItem(0,availableFieldHeaderItem);
    availableFieldsTreeView->setSortingEnabled(true);
    availableFieldsTreeView->setModel(availableFieldModel);

    QWidget *selectArea = new QWidget(this);
    QVBoxLayout *selectBox = new QVBoxLayout(selectArea);
    selectBox->setMargin(0);
    selectArea->setLayout(selectBox);

    selectedFieldsTreeView = new SelectedFieldsTreeView(selectArea);
    connect(selectedFieldsTreeView,SIGNAL(clicked(QModelIndex)),this,SLOT(selectedListClickedSlot(QModelIndex)));
    selectedFieldsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    selectedFieldModel = new QStandardItemModel(selectedFieldsTreeView);
    selectedFieldModel->setColumnCount(1);
    selectedHeaderItem = new QStandardItem("");
    selectedFieldModel->setHorizontalHeaderItem(0,selectedHeaderItem);
    selectedFieldsTreeView->setSortingEnabled(true);
    selectedFieldsTreeView->setModel(selectedFieldModel);

    clearPB = new QPushButton("Clear",this);
    connect(clearPB,SIGNAL(clicked()),this,SLOT(clearSelectedSlot()));
    clearAllPB  = new QPushButton("Clear All",this);
    connect(clearAllPB,SIGNAL(clicked()),this,SLOT(clearAllSlot()));
    defaultPB = new QPushButton("Default",this);
    connect(defaultPB,SIGNAL(clicked()),this,SLOT(defaultSlot()));
    defaultPB->setToolTip("Add Default Fields");
    QWidget *selectButonArea = new QWidget(this);
    QHBoxLayout *selectBBox = new QHBoxLayout(selectButonArea);
    selectBBox->setMargin(0);
    selectButonArea->setLayout(selectBBox);
    selectBBox->addWidget(clearPB,0,Qt::AlignLeft);
    selectBBox->addWidget(clearAllPB,0,Qt::AlignLeft);
    selectBBox->addStretch(1);
    selectBBox->addWidget(defaultPB,0);
    selectBox->addWidget(selectedFieldsTreeView,1);
    selectBox->addWidget(selectButonArea,0);

    wgrid->addWidget(messageListL,0,0,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(availableListL,0,1,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(selectedListL,0,2,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(messageArea,1,0);
    wgrid->addWidget(availableArea,1,1);
    wgrid->addWidget(selectArea,1,2);
    wgrid->setMargin(3);
    wgrid->setSpacing(4);
    wgrid->setRowStretch(0,0);
    wgrid->setRowStretch(1,1);
    wgrid->setColumnStretch(0,0);
    wgrid->setColumnStretch(1,1);
    wgrid->setColumnStretch(2,1);

    messageL = new QLabel(this);
    messageL->setAlignment(Qt::AlignCenter);
    fnt = messageL->font();
    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setBold(true);
    messageL->setFont(fnt);
    QPalette pal = messageL->palette();
    regMesssgeColor = pal.color(QPalette::WindowText);
    errorMessageColor = Qt::red;
    vbox->addLayout(topBox,0);
    vbox->addSpacing(10);
    vbox->addWidget(splitter,1);
    vbox->addSpacing(12);
    vbox->addWidget(messageL,0);
}
