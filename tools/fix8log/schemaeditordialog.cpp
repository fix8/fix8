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
#include "schemaitem.h"
#include "database.h"
#include "globals.h"
using namespace GUI;
SchemaEditorDialog::SchemaEditorDialog(Database *db,bool GlobalSchemaOn,QWidget *parent) :
    QMainWindow(parent),tableSchemaList(0),defaultTableSchema(0),
    currentSchemaItem(0),defaultSchemaItem(0),database(db),globalSchemaOn(GlobalSchemaOn),
    expandMode(Anything),defaultHeaderItems(0),currentTableSchema(0)
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
    connect(closeA,SIGNAL(triggered()),this,SLOT(closeSlot()));
    closeA->setIcon(QIcon(":/images/32x32/application-exit.png"));
    closeA->setToolTip(tr("Close This Window"));

    applyA = new QAction(tr("&Apply"),this);
    applyA->setIcon(QIcon(":/images/svg/apply.svg"));
    applyA->setToolTip(tr("Apply Schema"));

    saveA = new QAction(tr("&Save"),this);
    connect(saveA,SIGNAL(triggered()),this,SLOT(saveSchemaSlot()));
    saveA->setIcon(QIcon(":/images/svg/document-save.svg"));
    saveA->setToolTip(tr("Save"));
    fileMenu->addAction(applyA);
    fileMenu->addAction(saveA);

    fileMenu->addAction(closeA);
     mainToolBar->addAction(closeA);
     mainToolBar->addAction(applyA);
    mainToolBar->addAction(saveA);
    QWidget *cWidget = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(this);
    cWidget->setLayout(vbox);
    setCentralWidget(cWidget);


    applyBG = new QButtonGroup(this);
    applyBG->setExclusive(true);
    applyOnlyToCurrentRB = new QRadioButton("Apply To Tab",this);
    applyOnlyToCurrentRB->setToolTip("Only apply to tab at left");
    applyToWindowRB = new QRadioButton("Apply To Window",this);
    applyToWindowRB->setToolTip("Apply only to the tabs in the given window");

    applyToAllRB = new QRadioButton("Apply To All",this);
    applyToAllRB->setToolTip("Apply selected schema to all windows and all tabs");
    applyBG->addButton(applyOnlyToCurrentRB);
    applyBG->addButton(applyToWindowRB);
    applyBG->addButton(applyToAllRB);
    if (globalSchemaOn)
        applyToAllRB->setChecked(true);
    else
        applyToWindowRB->setChecked(true);
    connect(applyBG,SIGNAL(buttonClicked(QAbstractButton*)),
            this,SLOT(applyButtonSlot(QAbstractButton*)));
    QHBoxLayout *topBox = new QHBoxLayout();
    QWidget *applyArea = new QWidget(this);

    QHBoxLayout *applyBox = new QHBoxLayout();
    applyArea->setLayout(applyBox);
    applyBox->addWidget(applyToAllRB,0,Qt::AlignLeft);
    applyBox->addWidget(applyToWindowRB,0,Qt::AlignLeft);
    applyBox->addWidget(applyOnlyToCurrentRB,0,Qt::AlignLeft);

    targetArea = new QWidget();
    QHBoxLayout *tarBox = new QHBoxLayout();
    targetArea->setLayout(tarBox);
    QFont fnt = targetArea->font();
    fnt.setBold(true);
    targetArea->setFont(fnt);

    windowL = new QLabel("Window:");
    windowL->setFont(fnt);
    windowV = new QLineEdit("");
    windowV->setReadOnly(true);

    tabL = new QLabel("Tab:");
    tabL->setFont(fnt);
    tabV = new QLineEdit("");
    tabV->setReadOnly(true);

    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setItalic(true);
    windowV->setFont(fnt);
    tabV->setFont(fnt);
    QFontMetrics fm1(fnt);
    tabV->setMinimumWidth(fm1.maxWidth()*15);
    windowV->setMinimumWidth(fm1.maxWidth()*8);
    tarBox->addWidget(windowL,0,Qt::AlignLeft);
    tarBox->addWidget(windowV,1);
    tarBox->addSpacing(22);
    tarBox->addWidget(tabL,0);
    tarBox->addWidget(tabV,1);
    topBox->addWidget(applyArea,0,Qt::AlignLeft);
    topBox->addWidget(targetArea,1);
    topBox->addStretch(1);
    splitter = new QSplitter(Qt::Horizontal,this);
    buildSchemaArea();
    QWidget *workWidget = new QWidget();
    QGridLayout *wgrid = new QGridLayout();
    workWidget->setLayout(wgrid);

    splitter->insertWidget(0,schemaArea);
    splitter->insertWidget(1,workWidget);
    splitter->setChildrenCollapsible(true);

    messageListL  = new QLabel("Messages");
    messageListL->setToolTip("All possible FIX messages");
    availableListL = new QLabel("Available Columns");
    availableListL->setToolTip("All fields associated with selected message");
    selectedListL = new QLabel("Selected Columns");
    selectedListL->setToolTip("Table columns (schema)");
    selectedListL->setAlignment(Qt::AlignHCenter);
    messageListL->setAlignment(Qt::AlignHCenter);
    availableListL->setAlignment(Qt::AlignHCenter);
    selectedListL->setAlignment(Qt::AlignHCenter);
    QWidget *messageArea = new QWidget(this);
    QVBoxLayout *mbox = new QVBoxLayout(messageArea);
    messageArea->setLayout(mbox);
    mbox->setMargin(0);
    messageListView = new QListView();
    messageSpacerItem = new QSpacerItem(22,32);
    mbox->addWidget(messageListView,1);
    mbox->addSpacerItem(messageSpacerItem);

    messageModel = new QStandardItemModel();
    messageListView->setModel(messageModel);
    QStringList messageListHeaders;
    messageListHeaders << "Name";
    messageModel->setHorizontalHeaderLabels(messageListHeaders);

    connect(messageListView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(messageListClickedSlot(QModelIndex)));
    QWidget *availableArea = new QWidget(this);
    QVBoxLayout *avbox = new QVBoxLayout(availableArea);
    avbox->setMargin(0);
    availableArea->setLayout(avbox);

    availableTreeView = new QTreeView(this);
    availableTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
    avbox->addWidget(availableTreeView,1);
    avbox->addWidget(availableButtonArea,0,Qt::AlignLeft);
    //connect(availableTreeView,SIGNAL(clicked(QModelIndex)),
    //        this,SLOT(availableTreeViewClickedSlot(QModelIndex)));

    availableFieldModel = new QStandardItemModel(availableTreeView);
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));

    availableFieldModel->setColumnCount(1);
    availableFieldHeaderItem = new QStandardItem("Fields");
    availableFieldModel->setHorizontalHeaderItem(0,availableFieldHeaderItem);
    availableTreeView->setSortingEnabled(true);
    availableTreeView->setModel(availableFieldModel);

    QWidget *selectArea = new QWidget(this);
    QVBoxLayout *selectBox = new QVBoxLayout(selectArea);
    selectBox->setMargin(0);
    selectArea->setLayout(selectBox);

    selectedListView = new QTreeView(selectArea);
    connect(selectedListView,SIGNAL(clicked(QModelIndex)),this,SLOT(selectedListClickedSlot(QModelIndex)));
    selectedListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    selectedModel = new QStandardItemModel(selectedListView);
    selectedModel->setColumnCount(1);
    selectedHeaderItem = new QStandardItem("");
    selectedModel->setHorizontalHeaderItem(0,selectedHeaderItem);
    selectedListView->setSortingEnabled(true);
    selectedListView->setModel(selectedModel);

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
    selectBox->addWidget(selectedListView,1);
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
    vbox->addSpacing(22);
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
    connect(availableSchemasListView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(availableSchemasClickedSlot(QModelIndex)));
    schemaModel = new QStandardItemModel(availableSchemasListView);
    availableSchemasListView->setModel(schemaModel);
    descriptionBox = new QGroupBox("Description",availableArea);
    descriptionBox->setFlat(true);
    QVBoxLayout *dbox = new QVBoxLayout();
    dbox->setMargin(4);
    descriptionBox->setLayout(dbox);
    descriptionE = new QTextEdit(descriptionBox);
    QFontMetrics fm(descriptionE->font());
    descriptionE->setToolTip("Optional Field");
    descriptionE->setMaximumHeight(fm.height()*4);
    descriptionE->setReadOnly(true);
    dbox->addWidget(descriptionE);
    QWidget *schemaButtonArea = new QWidget(availableArea);
    QGridLayout *schemaGrid = new QGridLayout(schemaButtonArea);
    schemaButtonArea->setLayout(schemaGrid);
    newSchemaPB = new QPushButton("New",schemaButtonArea);
    newSchemaPB->setIcon(QIcon(":/images/svg/newspreadsheet.svg"));
    newSchemaPB->setToolTip("Create a new schema");
    connect(newSchemaPB,SIGNAL(clicked()),this,SLOT(newSchemaSlot()));
    copySchemaPB = new QPushButton("Copy",schemaButtonArea);
    copySchemaPB->setIcon(QIcon(":/images/svg/spreadsheetCopy.svg"));
    copySchemaPB->setToolTip("Create a copy of the selected schema");
    editSchemaPB = new QPushButton("Edit",schemaButtonArea);
    editSchemaPB->setIcon(QIcon(":/images/svg/editSchema.svg"));
    editSchemaPB->setToolTip("Modify given name and or description of selected schema");
    deleteSchemaPB = new QPushButton("Delete",schemaButtonArea);
    deleteSchemaPB->setIcon(QIcon(":/images/svg/editdelete.svg"));
    deleteSchemaPB->setToolTip("Delete selected schema");
    connect(deleteSchemaPB,SIGNAL(clicked()),this,SLOT(deleteSchemaSlot()));
    schemaGrid->addWidget(newSchemaPB,0,0);
    schemaGrid->addWidget(copySchemaPB,0,1);
    schemaGrid->addWidget(editSchemaPB,1,0);
    schemaGrid->addWidget(deleteSchemaPB,1,1);
    schemaGrid->setColumnStretch(0,0);
    schemaGrid->setColumnStretch(1,0);
    abox->addWidget(availableSchemasL,0);
    abox->addWidget(availableSchemasListView,1);
    abox->addWidget(descriptionBox,0);
    abox->addWidget(schemaButtonArea);
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
    QRegExp regExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]+$");
    QValidator *val = new QRegExpValidator(regExp,this);
    newSchemaLine->setValidator(val);
    connect(newSchemaLine,SIGNAL(textEdited(const QString &)),
            this,SLOT(nameEditedSlot(const QString &)));
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
      qDebug() << "\tTo do - clear current selection" << __FILE__ << __LINE__;
      return;
  }
  int numOfSchemas = schemaModel->rowCount();
  for(int i = 0;i< numOfSchemas;i++) {
      schemaItem = (SchemaItem *) schemaModel->item(i);
      if (schemaItem->tableSchema == currentTableSchema) {
        qDebug() << "Set current selection to schema item...." << __FILE__ << __LINE__;
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
          selectedModel->appendRow(selectItem);
          selectedMap.insert(qbe->name,selectItem);
          selectedBaseEntryList.append(qbe);
      }
      selectionModel = messageListView->selectionModel();
     selectionModel->select(messageModel->index(0,0),QItemSelectionModel::Select);
     messageListClickedSlot(messageModel->index(0,0));
  }
  connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
             this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
  setUpdatesEnabled(true);
  validate();


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
        messageModel->setItem(i,item);
        i++;
    }
    messageModel->sort(0);
}
void SchemaEditorDialog::setTableSchemas(TableSchemaList *tsl, TableSchema *dts)
{
    tableSchemaList = tsl;
    defaultTableSchema = dts;
    if (defaultTableSchema) {
        defaultSchemaItem = new SchemaItem(*defaultTableSchema);
        schemaModel->appendRow(defaultSchemaItem);
    }
    else {
        qWarning() << "NO Default Table Schema  For Scema Edtior" << __FILE__ << __LINE__;
    }
    if (!tsl) {
        qWarning() << "NO Default Table Schema List  For Scema Edtior" << __FILE__ << __LINE__;
        return;
    }
    tableSchemaList = tsl;
    TableSchema *tableSchema;
    SchemaItem *schemaItem;
    QListIterator <TableSchema *> iter(*tableSchemaList);
    while(iter.hasNext()) {
        tableSchema = iter.next();
        if (tableSchema != defaultTableSchema) {
            schemaItem = new SchemaItem(*tableSchema);
            schemaModel->appendRow(schemaItem);
        }
    }
    currentTableSchema = defaultTableSchema;
    buildSelectedListFromCurrentSchema();

}
void SchemaEditorDialog::setCurrentTableSchema(int scheamID)
{
}
void SchemaEditorDialog::setCurrentTarget(QString &windowName, QString &tabName)
{
    windowV->setText(windowName);
    tabV->setText(tabName);
    if (tabName.length() < 1) {
        applyToWindowRB->setChecked(true);
        applyOnlyToCurrentRB->setEnabled(false);
        tabV->setVisible(false);
    }
    else {
        applyOnlyToCurrentRB->setChecked(true);
        applyOnlyToCurrentRB->setEnabled(true);
        tabV->setVisible(true);
     }
}
bool SchemaEditorDialog::validate()
{
    bool isValid = false;
   // QItemSelectionModel *availSelModel =  availableTreeView->selectionModel();

    if (viewMode == NewMode || viewMode == EditMode) {
        if (newSchemaLine->text().length() > 1)
            isValid = true;
        newSchemaPB->setEnabled(false);
        saveEditPB->setEnabled(isValid);
        editSchemaPB->setEnabled(false);
        deleteSchemaPB->setEnabled(false);
        copySchemaPB->setEnabled(false);
        applyA->setEnabled(false);
        expandPB->setEnabled(false);
        collapsePB->setEnabled(false);
    }
    else {
        applyA->setEnabled(true);
        if (availableFieldModel->rowCount() > 0) {
            expandPB->setEnabled(true);
            collapsePB->setEnabled(true);
        }
        else {
            expandPB->setEnabled(false);
            collapsePB->setEnabled(false);
        }
        newSchemaPB->setEnabled(true);
        saveEditPB->setEnabled(false);
        if (currentSchemaItem) {
            if (currentSchemaItem == defaultSchemaItem) {
                deleteSchemaPB->setEnabled(false);
                editSchemaPB->setEnabled(false);
            }
            else {
                deleteSchemaPB->setEnabled(true);
                editSchemaPB->setEnabled(true);
            }
            copySchemaPB->setEnabled(true);
        }
        else {
            editSchemaPB->setEnabled(false);
            deleteSchemaPB->setEnabled(false);
            copySchemaPB->setEnabled(false);
        }
    }
    bool enableClear = false;
    QItemSelectionModel *selectedSelModel =  selectedListView->selectionModel();
    if (selectedSelModel) {
        if (selectedSelModel->hasSelection())
                enableClear = true;
    }
    clearPB->setEnabled(enableClear);
    enableClear = false;
    if (selectedModel->rowCount() > 0)
        enableClear = true;
    clearAllPB->setEnabled(enableClear);


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
