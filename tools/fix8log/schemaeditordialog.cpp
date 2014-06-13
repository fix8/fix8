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
SchemaEditorDialog::SchemaEditorDialog(Database *db,QWidget *parent) :
    QMainWindow(parent),tableSchemaList(0),defaultTableSchema(0),
    currentSchemaItem(0),defaultSchemaItem(0),database(db),
    expandMode(Anything),defaultHeaderItems(0),currentTableSchema(0),inUseTableSchema(0),tempTableSchema(0),
    tableSchemaStatus(NoMods),undoBuild(false),currentMainWindow(0)
{
    setWindowIcon(QIcon(":/images/svg/editSchema.svg"));
    setWindowTitle(tr("Table Schema Editor"));
    setAnimated(true);
    mainMenuBar = menuBar();
    fileMenu = mainMenuBar->addMenu(tr("&File"));
    mainToolBar = new QToolBar("Main Toolbar",this);
    mainToolBar->setObjectName("SchemaToolBar");
    mainToolBar->setMovable(true);
    addToolBar(Qt::TopToolBarArea,mainToolBar);
    closeA = new QAction(tr("&Close Window"),this);
    closeA->setIcon(QIcon(":/images/32x32/application-exit.png"));
    closeA->setToolTip(tr("Close This Window"));
    connect(closeA,SIGNAL(triggered()),this,SLOT(closeSlot()));

    applyA = new QAction(tr("&Apply"),this);
    applyA->setIcon(QIcon(":/images/svg/apply.svg"));
    applyA->setToolTip(tr("Apply Schema"));
    connect(applyA,SIGNAL(triggered()),this,SLOT(applySlot()));
    saveA = new QAction(tr("&Save"),this);
    saveA->setIcon(QIcon(":/images/svg/document-save.svg"));
    saveA->setToolTip(tr("Save"));
    connect(saveA,SIGNAL(triggered()),this,SLOT(saveSchemaSlot()));

    undoA = new QAction(tr("&Undo"),this);
    connect(undoA,SIGNAL(triggered()),this,SLOT(undoSchemaSlot()));
    undoA->setIcon(QIcon(":/images/svg/edit-undo.svg"));
    undoA->setToolTip(tr("Undo and rest to last saved value"));
    connect(undoA,SIGNAL(triggered()),this,SLOT(undoSchemaSlot()));

    newSchemaA = new QAction(QIcon(":/images/svg/newspreadsheet.svg"),"New",this);
    newSchemaA->setToolTip("Create a new schema");
    connect(newSchemaA,SIGNAL(triggered()),this,SLOT(newSchemaSlot()));

    copySchemaA = new QAction(QIcon(":/images/svg/spreadsheetCopy.svg"),"Copy",this);
    copySchemaA->setToolTip("Create a copy of the selected schema");

    editSchemaA = new QAction(QIcon(":/images/svg/editSchema.svg"),"Edit",this);
    editSchemaA->setToolTip("Modify given name and or description of selected schema");
    connect(editSchemaA,SIGNAL(triggered()),this,SLOT(editSchemaSlot()));

    deleteSchemaA = new QAction(QIcon(":/images/svg/editdelete.svg"),"Delete",this);
    deleteSchemaA->setToolTip("Delete selected schema");
    connect(deleteSchemaA,SIGNAL(triggered()),this,SLOT(deleteSchemaSlot()));

    fileMenu->addAction(applyA);
    fileMenu->addAction(saveA);
    fileMenu->addAction(undoA);
    fileMenu->addAction(closeA);
    mainToolBar->addAction(closeA);
    mainToolBar->addAction(applyA);
    mainToolBar->addAction(saveA);
    mainToolBar->addAction(undoA);
    mainToolBar->addSeparator();
    mainToolBar->addAction(newSchemaA);
    mainToolBar->addAction(copySchemaA);
    mainToolBar->addAction(editSchemaA);
    mainToolBar->addAction(deleteSchemaA);

    // central widget if schema mainwindow
    QWidget *cWidget = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(cWidget);
    cWidget->setLayout(vbox);
    setCentralWidget(cWidget);

    // target window at top of main widget
    targetArea = new QWidget(this);
    QHBoxLayout *tarBox = new QHBoxLayout(targetArea);
    targetArea->setLayout(tarBox);
    QFont fnt = targetArea->font();
    fnt.setBold(true);
    targetArea->setFont(fnt);

    windowL = new QLabel("Window:",this);
    windowL->setFont(fnt);
    windowV = new QLabel("",this);
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
    viewMode = RegMode;
}
void SchemaEditorDialog::buildSchemaArea()
{
    schemaArea = new QStackedWidget(this);
    availableArea = new QWidget(schemaArea);
    QVBoxLayout *abox = new QVBoxLayout(availableArea);
    abox->setMargin(0);
    availableArea->setLayout(abox);
    availableSchemasL = new QLabel("Schemas",availableArea);
    availableSchemasListView = new QListView(availableArea);
    SchemaDelegate *schemaDelegate = new SchemaDelegate(availableSchemasListView);
    availableSchemasListView->setItemDelegate(schemaDelegate);
    connect(availableSchemasListView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(availableSchemasClickedSlot(QModelIndex)));
    availableSchemaModel = new QStandardItemModel(availableSchemasListView);
    availableSchemasListView->setModel(availableSchemaModel);
    descriptionBox = new QGroupBox("Description",availableArea);
    descriptionBox->setFlat(true);
    QVBoxLayout *dbox = new QVBoxLayout(descriptionBox);
    dbox->setMargin(4);
    descriptionBox->setLayout(dbox);
    descriptionE = new QTextEdit(descriptionBox);
    QFontMetrics fm(descriptionE->font());
    descriptionE->setToolTip("Optional Field");
    descriptionE->setMaximumHeight(fm.height()*4);
    descriptionE->setReadOnly(true);
    dbox->addWidget(descriptionE);

    abox->addWidget(availableSchemasL,0);
    abox->addWidget(availableSchemasListView,1);
    abox->addWidget(descriptionBox,0);
    //abox->addWidget(schemaButtonArea);
    newSchemaArea = new QWidget(schemaArea);
    QVBoxLayout  *newBox = new QVBoxLayout(newSchemaArea);
    newSchemaArea->setLayout(newBox);
    newBox->setMargin(0);
    newBox->setSpacing(5);
    newAvailableSchemasL = new QLabel("Create New Schema",newSchemaArea);
    newSchemaL = new QLabel("Name",newSchemaArea);
    newSchemaLine = new QLineEdit(newSchemaArea);
    newSchemaLine->setMaxLength(24);
    newSchemaLine->setToolTip("Name of Schema");
    newSchemaLine->setToolTip("Type name of new schema\n(Can also use \'Enter\' key to save)");
    QRegExp regExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]+$");
    QValidator *val = new QRegExpValidator(regExp,this);
    newSchemaLine->setValidator(val);
    connect(newSchemaLine,SIGNAL(textEdited(const QString &)),
            this,SLOT(nameEditedSlot(const QString &)));
    connect(newSchemaLine,SIGNAL(returnPressed()),
            this,SLOT(saveNewEditSlot()));
    newDescriptionBox = new QGroupBox("Description",newSchemaArea);
    newDescriptionBox->setFlat(true);
    QVBoxLayout *ndbox = new QVBoxLayout(newDescriptionBox);
    ndbox->setMargin(4);
    newDescriptionBox->setLayout(ndbox);
    newDescriptionE = new QTextEdit(newDescriptionBox);
    newDescriptionE->setToolTip("Optional Field");
    ndbox->addWidget(newDescriptionE);

    QWidget *schemaButtonEditArea = new QWidget(newSchemaArea);
    QHBoxLayout *schemaButtonEditBox = new QHBoxLayout(schemaButtonEditArea);
    schemaButtonEditArea->setLayout(schemaButtonEditBox);
    saveEditPB = new QPushButton("Save",schemaButtonEditArea);
    saveEditPB->setToolTip("Save schema");
    saveEditPB->setIcon(QIcon(":/images/svg/checkmark.svg"));
    cancelEditPB = new QPushButton("Cancel",schemaButtonEditArea);
    cancelEditPB->setToolTip("Cancel the creation or editing of a schema");
    cancelEditPB->setIcon(QIcon(":/images/svg/cancel.svg"));
    connect(saveEditPB,SIGNAL(clicked()),this,SLOT(saveNewEditSlot()));
    connect(cancelEditPB,SIGNAL(clicked()),this,SLOT(cancelNewSlot()));
    schemaButtonEditBox->addWidget(cancelEditPB,0);
    schemaButtonEditBox->addWidget(saveEditPB,0);
    newBox->addWidget(newAvailableSchemasL,0);
    newBox->addSpacing(32);
    newBox->addWidget(newSchemaL,0,Qt::AlignBottom);
    newBox->addWidget(newSchemaLine,0,Qt::AlignTop);
    newBox->addWidget(newDescriptionBox,0,Qt::AlignTop);
    newBox->addWidget(schemaButtonEditArea,0,Qt::AlignTop);
    newBox->addStretch(1);
    QPalette pal = descriptionE->palette();
    regularColor = pal.color(QPalette::Base);
    editColor = QColor(174,184,203,180);
    pal = newSchemaLine->palette();
    pal.setColor(QPalette::Base,editColor);
    newSchemaLine->setPalette(pal);
    newDescriptionE->setPalette(pal);
    schemaArea->insertWidget(RegMode,availableArea);
    schemaArea->insertWidget(EditMode,newSchemaArea);
    schemaArea->setCurrentIndex(RegMode);
}
void SchemaEditorDialog::buildSelectedListFromCurrentSchema()
{
    SchemaItem *schemaItem;

    QItemSelectionModel *selectionModel = availableSchemasListView->selectionModel();

    selectionModel->clear();
    if (!currentTableSchema) {
        qWarning() << "No current table schema selected" << __FILE__ << __LINE__;
        return;
    }
    if (!currentTableSchema->fieldList) {
        qWarning() << "No Fields in current table" << __FILE__ << __FILE__;
        return;
    }
    int numOfSchemas = availableSchemaModel->rowCount();
    for(int i = 0;i< numOfSchemas;i++) {
        schemaItem = (SchemaItem *) availableSchemaModel->item(i);
        if (schemaItem->tableSchema == currentTableSchema) {
            currentSchemaItem = schemaItem;
            selectionModel->select(schemaItem->index(),QItemSelectionModel::Select);
            break;
        }
    }
    setUpdatesEnabled(false);
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    QStringList messageNameList;
    QString tooltipStr;
    FieldUse *fieldUse;
    MessageField *messageField;
    QBaseEntry *qbe;
    QVariant var;
    QStandardItem *selectItem;
    QStandardItem *availItem;
    QMap<QString,QStandardItem *>::iterator  iter;
    QList <QBaseEntry *> *bel = currentTableSchema->getFields();
    if (!bel) {
        qWarning() << "NO Field list found for current schema " << __FILE__ << __LINE__;
        return;
    }
    QListIterator <QBaseEntry *> iter2(*bel);
    if (selectedFieldModel->rowCount() > 0) {
        selectedFieldModel->removeRows(0,selectedFieldModel->rowCount());
    }
    while(iter2.hasNext()) {
        qbe = iter2.next();
        iter  = selectedMap.find(qbe->name);
        if (iter == selectedMap.end()) { // not found
            // see if in available and if so mark it checked
            QMultiMap<QString,QStandardItem *>::iterator availIter = availableMap.find(qbe->name);
            if (availIter != availableMap.end()) {
                availItem = availIter.value();
                availItem->setCheckState(Qt::Checked);
            }
            selectItem = new QStandardItem(qbe->name);
            if (fieldUseList) {
                fieldUse = fieldUseList->findByName(qbe->name);
                if (fieldUse) {
                    QListIterator <MessageField *> iter(fieldUse->messageFieldList);
                    while(iter.hasNext()) {
                        messageField = iter.next();
                        messageNameList << messageField->name;
                    }
                }
                if (messageNameList.count() == 0)
                    tooltipStr = "Not used in any messages";
                if (messageNameList.count() == 1)
                    tooltipStr = "Used in " + messageNameList.at(0) + " message" ;
                else if(messageNameList.count() < 5)
                    tooltipStr = "Used in: " + messageNameList.join("\n\t");
                else
                    tooltipStr = "Used in " + QString::number(messageNameList.count()) + " messages";

                selectItem->setToolTip(tooltipStr);
            }
            var.setValue((void *) qbe);
            selectItem->setData(var);
            selectedFieldModel->appendRow(selectItem);
            selectedMap.insert(qbe->name,selectItem);
            selectedBaseEntryList.append(qbe);
        }
    }
    selectionModel = messageListTreeView->selectionModel();
    selectionModel->select(messageModel->index(0,0),QItemSelectionModel::Select);
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    setUpdatesEnabled(true);
    //qDebug() << "11 Call Validate " << __FILE__ << __LINE__;

    // validate();
}
void SchemaEditorDialog::closeEvent(QCloseEvent *ce)
{
    if (statusValue == SaveNeeded) {
        QMessageBox::information(this,"FIX8LogViewer","Save or Undo Changes Before Closing");
        ce->ignore();
    }
    else
        QMainWindow::closeEvent(ce);

}

void SchemaEditorDialog::showEvent(QShowEvent *se)
{
    validate();
    QMainWindow::showEvent(se);
}
void SchemaEditorDialog::setDefaultHeaderItems( QBaseEntryList &DefaultHeaderItems)
{
    defaultHeaderItems = &DefaultHeaderItems;
}

void SchemaEditorDialog::setBaseMaps(QMap<QString, QBaseEntry *>  &BaseMap)
{
    baseMap = &BaseMap;
}
void SchemaEditorDialog::setFieldUseList(FieldUseList &ful)
{
    fieldUseList = &ful;
}
void SchemaEditorDialog::populateMessageList(MessageFieldList *mfl)
{
    messageFieldList = mfl;
    MessageField *mf;
    QStandardItem *item;
    if (!messageFieldList) {
        qWarning() << "Error - messageFieldList is null" << __FILE__ << __LINE__;
        messageModel->setRowCount(0);
        return;
    }
    QListIterator <MessageField *> iter(*messageFieldList);
    messageModel->setRowCount(messageFieldList->count());
    int i = 0;
    while(iter.hasNext()) {
        mf = iter.next();
        QVariant var;
        var.setValue((void *) mf);
        item = new QStandardItem(mf->name);
        item->setData(var);
        item->setData(false,MessageItemDelegate::InUseRole);
        messageModel->setItem(i,item);
        i++;
    }
    messageModel->sort(0);
}
void SchemaEditorDialog::setTableSchemas(TableSchemaList *tsl, TableSchema *dts)
{
    tableSchemaList = tsl;
    if (availableSchemaModel->rowCount() > 0) {
        availableSchemaModel->removeRows(0,availableSchemaModel->rowCount() -1);
    }
    defaultTableSchema = dts;
    if (defaultTableSchema) {
        currentTableSchema = defaultTableSchema;
        defaultSchemaItem = new SchemaItem(*defaultTableSchema);
        currentSchemaItem = defaultSchemaItem;
        availableSchemaModel->appendRow(defaultSchemaItem);
    }
    else {
        defaultSchemaItem = 0;
        currentTableSchema = 0;
        currentSchemaItem = 0;
        qWarning() << "NO Default Table Schema  For Scema Edtior" << __FILE__ << __LINE__;
    }
    if (!tableSchemaList) {
        qWarning() << "NO Default Table Schema List  For Scema Edtior" << __FILE__ << __LINE__;
        return;
    }
    TableSchema *tableSchema;
    SchemaItem *schemaItem;
    QListIterator <TableSchema *> iter(*tableSchemaList);
    while(iter.hasNext()) {
        tableSchema = iter.next();
        if (tableSchema != defaultTableSchema) {
            schemaItem = new SchemaItem(*tableSchema);
            availableSchemaModel->appendRow(schemaItem);
        }
    }
/*
    if (currentTableSchema) {
        tempTableSchema = currentTableSchema->clone();
    }
    buildSelectedListFromCurrentSchema();
*/
}
void SchemaEditorDialog::setTableSchemaInUse(TableSchema *ts)
{
    if (tempTableSchema)  {
        delete tempTableSchema;
         tempTableSchema = 0;
    }
    inUseTableSchema = ts;
    currentTableSchema = inUseTableSchema;
    if (currentTableSchema) {
        tempTableSchema = currentTableSchema->clone();
        buildSelectedListFromCurrentSchema();
    }
    else {
        tempTableSchema = 0;
    }
}
bool SchemaEditorDialog::setCurrentTarget( MainWindow *mainWindow,bool editRequest)
{
    QString str;
    if (!mainWindow) {
        qWarning() << "Error, mainwindow is null" << __FILE__ <<__LINE__;
        return false;
    }
    if (editRequest) {
        if (mainWindow == currentMainWindow) {
            qDebug() << "Windows are the same, return" << __FILE__ << __LINE__;
            return false;
        }
        else if (currentMainWindow && (tableSchemaStatus == HaveMods)) {
            str = "Schema Editor Locked By Window " + currentMainWindow->getName();
            str.append("\nSave or Cancel Changes To Unlock");
            mainWindow->displayMessageDialog(str);
            return false;
        }
    }
    if (currentMainWindow) {
        if (currentMainWindow != mainWindow) {
            if (tableSchemaStatus == HaveMods) {
                    str = "Schema Editor Locked By Window " + currentMainWindow->getName();
                    str.append("\nSave or Cancel Changes To Unlock");
                    mainWindow->displayMessageDialog(str);
                    return false;

            }
            currentMainWindow = mainWindow;
            str = mainWindow->getName();
            if (str.length() < 1) {
                str = qApp->applicationDisplayName();
            }
            showWindowArea(mainWindow->getName());
        }
        else { // currentWindow == mainWindow
            if (tableSchemaStatus == HaveMods) {
                    str = "Save Needed For Current Schema " + currentTableSchema->name;
                    str.append("\nFirst Save or Cancel Changes In Schema Editor");
                    mainWindow->displayMessageDialog(str);
                    return false;

            }
            str = mainWindow->getName();
            if (str.length() < 1) {
                str = qApp->applicationDisplayName();
            }
            showWindowArea(str);
        }
    }
    else {
        currentMainWindow = mainWindow;
        str = mainWindow->getName();
        if (str.length() < 1) {
            str = qApp->applicationDisplayName();
        }
        showWindowArea(str);
    }
    return true;
}
void SchemaEditorDialog::showWindowArea( QString windowName)
{
        windowV->setText(windowName);


}
void SchemaEditorDialog::updateStatusOfMessageList()
{
    QStandardItem *item;
    QModelIndex index;
    MessageField *mf;
    QVariant var;
    qDebug() << "UPDATE MESSAGE LIST STATUS: " << __FILE__ << __LINE__;
    if (!messageModel) {
        qWarning() << "NULL MESSAGE MODEL" << __FILE__ << __LINE__;
    }
    if (messageModel->rowCount() < 1)
        return;
    for(int i=0;i<messageModel->rowCount();i++) {
        index = messageModel->index(i,0);
        item = messageModel->itemFromIndex(index);
        var = item->data();
        mf = (MessageField *) var.value<void *>();
        //mf->fieldsV;
    }

}

bool SchemaEditorDialog::validate()
{
    bool isValid = false;
    bool schemaModified = false;
    tableSchemaStatus = HaveMods;
    // QItemSelectionModel *availSelModel =  availableFieldsTreeView->selectionModel();
    if (currentTableSchema) {
        if (tempTableSchema) {
            if (*currentTableSchema == *tempTableSchema) {
                tableSchemaStatus = NoMods;
            }
        }
    }
    else if (!tempTableSchema)
        tableSchemaStatus = NoMods;
    if (tableSchemaStatus == HaveMods) {
        schemaModified = true;
        setStatus(SaveNeeded);
    }
    else
        setStatus(Ok);
    saveA->setEnabled(schemaModified);
    undoA->setEnabled(schemaModified);
    if (currentSchemaItem) {
        currentSchemaItem->setModified(schemaModified);
        availableSchemasListView->viewport()->update();
    }
    if (viewMode == NewMode || viewMode == EditMode) {
        if (newSchemaLine->text().length() > 1)
            isValid = true;
        newSchemaA->setEnabled(false);
        saveEditPB->setEnabled(isValid);
        editSchemaA->setEnabled(false);
        deleteSchemaA->setEnabled(false);
        copySchemaA->setEnabled(false);
        applyA->setEnabled(false);
        expandPB->setEnabled(false);
        collapsePB->setEnabled(false);
    }
    else {
        if (currentTableSchema && inUseTableSchema && (*currentTableSchema == *inUseTableSchema)) {
            applyA->setEnabled(false);
        }
        else
            applyA->setEnabled(true);

        if (availableFieldModel->rowCount() > 0) {
            expandPB->setEnabled(true);
            collapsePB->setEnabled(true);
        }
        else {
            expandPB->setEnabled(false);
            collapsePB->setEnabled(false);
        }
        newSchemaA->setEnabled(true);
        saveEditPB->setEnabled(false);
        if (currentSchemaItem) {
            if (currentSchemaItem == defaultSchemaItem) {
                deleteSchemaA->setEnabled(false);
                editSchemaA->setEnabled(false);
            }
            else {
                deleteSchemaA->setEnabled(true);
                editSchemaA->setEnabled(true);
            }
            copySchemaA->setEnabled(true);
        }
        else {
            editSchemaA->setEnabled(false);
            deleteSchemaA->setEnabled(false);
            copySchemaA->setEnabled(false);
        }
    }
    bool enableClear = false;
    QItemSelectionModel *selectedSelModel =  selectedFieldsTreeView->selectionModel();
    if (selectedSelModel) {
        if (selectedSelModel->hasSelection())
            enableClear = true;
    }
    clearPB->setEnabled(enableClear);
    enableClear = false;
    if (selectedFieldModel->rowCount() > 0)
        enableClear = true;
    clearAllPB->setEnabled(enableClear);

    update();
    repaint();
    return isValid;
}
void SchemaEditorDialog::saveSettings()
{
    QSettings settings("fix8","logviewer");
    settings.setValue("SchemaEditorGeometry",saveGeometry());
    settings.setValue("SchemaEditorState",saveState());
    settings.setValue("SchemaEditorSplitter",splitter->saveState());
}

void SchemaEditorDialog::restoreSettings()
{
    QSettings settings("fix8","logviewer");
    restoreGeometry(settings.value("SchemaEditorGeometry").toByteArray());
    restoreState(settings.value("SchemaEditorState").toByteArray());
    splitter->restoreState(settings.value("SchemaEditorSplitter").toByteArray());
}
void SchemaEditorDialog::setMessage(QString str, bool isError)
{
    QPalette pal = messageL->palette();
    if (isError)
        pal.setColor(QPalette::WindowText,errorMessageColor);
    else
        pal.setColor(QPalette::WindowText,regMesssgeColor);
    messageL->setPalette(pal);
    messageL->setText(str);
}
void SchemaEditorDialog::setStatus(StatusType st)
{
    statusValue = st;
    if (statusValue == SaveNeeded) {
        statusI->setPixmap(QPixmap(":/images/svg/greenPlus.svg"));
        statusL->setText("Save Needed");
        statusBar()->showMessage("Save Needed");
    }

    else if (statusValue == EmptyFields) {
        statusI->setPixmap(QPixmap(":/images/svg/xred.svg"));
        statusL->setText("No Fields");
        statusBar()->showMessage("Fields Needed");
        //statusArea->show();
    }

    else {
        statusI->setPixmap(QPixmap());
        statusL->setText("");
    }

}
void SchemaEditorDialog::windowDeleted(MainWindow *mw)
{
    if (mw == currentMainWindow)
        closeSlot();
}
