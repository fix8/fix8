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
    QDialog(parent),tableSchemaList(0),defaultTableSchema(0),
    currentSchemaItem(0),defaultSchemaItem(0),database(db),globalSchemaOn(GlobalSchemaOn)
{
    setWindowIcon(QIcon(":/images/svg/editSchema.svg"));
    setWindowTitle(tr("Fix8 LogViewer"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
    setLayout(vbox);
    QWidget *titleArea = new QWidget(this);
    QHBoxLayout *titleBox = new QHBoxLayout(titleArea);
    titleArea->setLayout(titleBox);

    iconL = new QLabel(titleArea);
    iconL->setPixmap(QPixmap(":/images/svg/editSchema.svg").scaled(42,42));

    QString str = tr("Table Schema Editor");
    titleL = new QLabel(str,titleArea);
    QFont fnt = titleL->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize() + 2);
    fnt.setItalic(true);
    titleL->setFont(fnt);
    titleBox->addWidget(iconL,0);
    titleBox->addWidget(titleL,1,Qt::AlignCenter);
    titleL->setAlignment(Qt::AlignCenter);
    QFrame *hrLine = new QFrame(this);
    hrLine->setFrameStyle(QFrame::HLine);

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
    fnt = targetArea->font();
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
    schemaArea = buildSchemaArea();

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
    messageListView = new QListView();
    availableListView = new QListView();
    selectedListView = new QListView();

    wgrid->addWidget(messageListL,0,0,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(availableListL,0,1,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(selectedListL,0,2,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(messageListView,1,0);
    wgrid->addWidget(availableListView,1,1);
    wgrid->addWidget(selectedListView,1,2);
    wgrid->setMargin(3);
    wgrid->setSpacing(4);
    wgrid->setRowStretch(0,0);
    wgrid->setRowStretch(1,1);
    wgrid->setColumnStretch(0,0);
    wgrid->setColumnStretch(1,1);
    wgrid->setColumnStretch(2,1);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    closeB = buttonBox->addButton(QDialogButtonBox::Close);
    applyB = buttonBox->addButton(QDialogButtonBox::Apply);
    connect(buttonBox,SIGNAL(clicked(QAbstractButton*)),
            this,SLOT(actionButtonSlot(QAbstractButton*)));
    messageL = new QLabel(this);
    messageL->setAlignment(Qt::AlignCenter);
    fnt = messageL->font();
    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setBold(true);
    messageL->setFont(fnt);
    QPalette pal = messageL->palette();
    regMesssgeColor = pal.color(QPalette::WindowText);
    errorMessageColor = Qt::red;

    vbox->addWidget(titleArea,0);
    vbox->addWidget(hrLine,0);
    vbox->addLayout(topBox,0);
    vbox->addSpacing(22);
    vbox->addWidget(splitter,1);
    vbox->addSpacing(12);
    vbox->addWidget(messageL,0);
    vbox->addSpacing(12);
    vbox->addWidget(buttonBox,0);

    populateMessageList();
    newSchemaStackArea->setCurrentIndex(RegMode);
    buttonStackArea->setCurrentIndex(RegMode);
    viewMode = RegMode;
}
QWidget *SchemaEditorDialog::buildSchemaArea()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *sbox = new QVBoxLayout(widget);
    widget->setLayout(sbox);
    newSchemaArea = new QWidget(widget);
    QVBoxLayout  *newBox = new QVBoxLayout(newSchemaArea);
    newSchemaArea->setLayout(newBox);
    newBox->setMargin(0);
    newBox->setSpacing(5);
    ///QWidget *schemaInner = QWidget();
    availableSchemasListView = new QListView(newSchemaArea);
    connect(availableSchemasListView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(availableSchemasClickedSlot(QModelIndex)));
    schemaModel = new QStandardItemModel(availableSchemasListView);
    availableSchemasListView->setModel(schemaModel);
    newSchemaL = new QLabel("Name",newSchemaArea);
    newSchemaLine = new QLineEdit(newSchemaArea);
    newSchemaLine->setMaxLength(24);
    newSchemaLine->setToolTip("Name of Schema");
    QRegExp regExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]+$");
    QValidator *val = new QRegExpValidator(regExp,this);
    newSchemaLine->setValidator(val);
    connect(newSchemaLine,SIGNAL(textEdited(const QString &)),
            this,SLOT(nameEditedSlot(const QString &)));
    newBox->addWidget(newSchemaL,0,Qt::AlignBottom);
    newBox->addWidget(newSchemaLine,0,Qt::AlignTop);
    newSchemaStackArea = new QStackedWidget(widget);
    newSchemaStackArea->insertWidget(RegMode,availableSchemasListView);
    newSchemaStackArea->insertWidget(NewMode,newSchemaArea);
    descriptionBox = new QGroupBox("Description",widget);
    descriptionBox->setFlat(true);
    QVBoxLayout *dbox = new QVBoxLayout();
    dbox->setMargin(4);
    descriptionBox->setLayout(dbox);
    descriptionE = new QTextEdit(descriptionBox);
    descriptionE->setToolTip("Optional Field");
    QPalette pal = descriptionE->palette();
    regularColor = pal.color(QPalette::Base);
    editColor = QColor(174,184,203,180);
    pal = newSchemaLine->palette();
    pal.setColor(QPalette::Base,editColor);
    newSchemaLine->setPalette(pal);
    QFontMetrics fm(descriptionE->font());
    descriptionE->setMaximumHeight(fm.height()*3);
    availableSchemasL = new QLabel("Available Schemas");
    descriptionE->setReadOnly(true);
    dbox->addWidget(descriptionE);
    buttonStackArea = new QStackedWidget(this);
    QWidget *schemaButtonEditArea = new QWidget(this);
    QHBoxLayout *schemaButtonEditBox = new QHBoxLayout();
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
    QWidget *schemaButtonArea = new QWidget(widget);
    QGridLayout *schemaGrid = new QGridLayout();
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
    sbox->setMargin(0);
    sbox->setSpacing(0);
    sbox->addWidget(availableSchemasL,0);
    sbox->addWidget(newSchemaStackArea,1);
    sbox->addWidget(descriptionBox,0);
    buttonStackArea->insertWidget(RegMode,schemaButtonArea);
    buttonStackArea->insertWidget(NewMode,schemaButtonEditArea);
    sbox->addWidget(buttonStackArea,0,Qt::AlignBottom);
    return (widget);
}
void SchemaEditorDialog::showEvent(QShowEvent *se)
{
    validate();
    QDialog::showEvent(se);
}

void SchemaEditorDialog::populateMessageList()
{
    QStandardItem *item;
    messageModel = new QStandardItemModel();
    messageListView->setModel(messageModel);
    int count = Globals::messagePairs->count();
    messageModel->setRowCount(count);
    for(int i= 0;i< count;i++) {
        //qDebug() << "Create item I = " << i << "total = " << count;
        item = new QStandardItem(Globals::messagePairs->at(i).second);
        messageModel->setItem(i,item);
    }
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
    if (viewMode == NewMode || viewMode == EditMode) {
        if (newSchemaLine->text().length() > 1)
            isValid = true;
        newSchemaPB->setEnabled(false);
        saveEditPB->setEnabled(isValid);
        editSchemaPB->setEnabled(false);
        deleteSchemaPB->setEnabled(false);
        copySchemaPB->setEnabled(false);
    }
    else {
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
    return isValid;
}
void SchemaEditorDialog::saveSettings()
{
    QSettings settings("fix8","logviewer");
    settings.setValue("ScehmaEditorGeometry",saveGeometry());
    settings.setValue("ScehmaEditorSpliiter",splitter->saveState());
}

void SchemaEditorDialog::restoreSettings()
{
    QSettings settings("fix8","logviewer");
    restoreGeometry(settings.value("ScehmaEditorGeometry").toByteArray());
    splitter->restoreState(settings.value("ScehmaEditorSpliiter").toByteArray());
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
