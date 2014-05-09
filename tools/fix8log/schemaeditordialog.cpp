#include "schemaeditordialog.h"
#include "globals.h"
using namespace GUI;
SchemaEditorDialog::SchemaEditorDialog(QWidget *parent) :
    QDialog(parent)
{
    qDebug() << "SCMEMA 1";
    setWindowIcon(QIcon(":/images/svg/editSchema.svg"));
    setWindowTitle(tr("Fix8 LogViewer"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
    setLayout(vbox);
    QWidget *titleArea = new QWidget(this);
    QHBoxLayout *titleBox = new QHBoxLayout(titleArea);
    titleArea->setLayout(titleBox);
    qDebug() << "SCMEMA 6";

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
    qDebug() << "SCMEMA 10";

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
    connect(applyBG,SIGNAL(buttonClicked(QAbstractButton*)),
            this,SLOT(applyButtonSlot(QAbstractButton*)));
    QHBoxLayout *topBox = new QHBoxLayout();
    QWidget *applyArea = new QWidget(this);
    qDebug() << "SCMEMA 15";

    QHBoxLayout *applyBox = new QHBoxLayout();
    applyArea->setLayout(applyBox);
    applyBox->addWidget(applyToAllRB,0,Qt::AlignLeft);
    applyBox->addWidget(applyToWindowRB,0,Qt::AlignLeft);
    applyBox->addWidget(applyOnlyToCurrentRB,0,Qt::AlignLeft);
    qDebug() << "SCMEMA 17";

    targetArea = new QWidget();
    QHBoxLayout *tarBox = new QHBoxLayout();
    targetArea->setLayout(tarBox);
    fnt = targetArea->font();
    fnt.setBold(true);
    targetArea->setFont(fnt);
    qDebug() << "SCMEMA 18";

    windowL = new QLabel("Window:");
    windowL->setFont(fnt);
    windowV = new QLabel("");
    qDebug() << "SCMEMA 20";

    tabL = new QLabel("Tab:");
    tabL->setFont(fnt);
    tabV = new QLabel("");
    qDebug() << "SCMEMA 21";

    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setItalic(true);
    windowV->setFont(fnt);
    tabV->setFont(fnt);
    qDebug() << "SCMEMA 22";

    tarBox->addWidget(windowL,0,Qt::AlignLeft);
    tarBox->addWidget(windowV,1,Qt::AlignLeft);
    tarBox->addSpacing(22);
    tarBox->addWidget(tabL,0);
    tarBox->addWidget(tabV,1,Qt::AlignLeft);
    qDebug() << "SCMEMA 23";

    topBox->addWidget(applyArea,0,Qt::AlignLeft);
    topBox->addWidget(targetArea,0,Qt::AlignLeft);
    qDebug() << "SCMEMA 24";

    topBox->addStretch(1);
    qDebug() << "SCMEMA 25";


    QWidget *workWidget = new QWidget();
    QGridLayout *wgrid = new QGridLayout();
    workWidget->setLayout(wgrid);

    availableSchemasL = new QLabel("Available Schemas");

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
    qDebug() << "SCMEMA 30";

    schemaArea = new QWidget(this);
    QVBoxLayout *sbox = new QVBoxLayout(schemaArea);
    schemaArea->setLayout(sbox);
    newSchemaStackArea = new QStackedWidget(this);

    availableSchemasListView = new QListView(schemaArea);
    schemaModel = new QStandardItemModel(availableSchemasListView);
    availableSchemasListView->setModel(schemaModel);

    newSchemaArea = new QWidget(schemaArea);
    QVBoxLayout  *newBox = new QVBoxLayout(newSchemaArea);
    newSchemaArea->setLayout(newBox);
    newBox->setMargin(0);
    newBox->setSpacing(5);
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
    newSchemaStackArea->insertWidget(RegMode,availableSchemasListView);
    newSchemaStackArea->insertWidget(NewMode,newSchemaArea);

    descriptionBox = new QGroupBox("Description",schemaArea);
    descriptionBox->setFlat(true);
    QVBoxLayout *dbox = new QVBoxLayout();
    dbox->setMargin(4);
    descriptionBox->setLayout(dbox);
    descriptionE = new QTextEdit(descriptionBox);
    descriptionE->setToolTip("Optional Field");


    QPalette pal = descriptionE->palette();
    regularColor = pal.color(QPalette::Base);
    editColor = QColor(174,184,203,180);
    QFontMetrics fm(descriptionE->font());
    descriptionE->setMaximumHeight(fm.height()*3);
    descriptionE->setReadOnly(true);
    dbox->addWidget(descriptionE);
    qDebug() << "SCMEMA 40";

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
    connect(cancelEditPB,SIGNAL(clicked()),this,SLOT(cancelNewSlot()));
    schemaButtonEditBox->addWidget(cancelEditPB,0);
    schemaButtonEditBox->addWidget(saveEditPB,0);

    QWidget *schemaButtonArea = new QWidget(this);
    QGridLayout *schemaGrid = new QGridLayout();
    schemaButtonArea->setLayout(schemaGrid);
    newSchemaPB = new QPushButton("New",schemaArea);
    newSchemaPB->setToolTip("Create a new schema");
    connect(newSchemaPB,SIGNAL(clicked()),this,SLOT(newSchemaSlot()));
    copySchemaPB = new QPushButton("Copy",schemaArea);
    copySchemaPB->setToolTip("Create a copy of the selected schema");
    editSchemaPB = new QPushButton("Edit",schemaArea);
    editSchemaPB->setToolTip("Modify given name and or description of selected schema");
    deleteSchemaPB = new QPushButton("Delete",schemaArea);
    deleteSchemaPB->setToolTip("Delete selected schema");
    schemaGrid->addWidget(newSchemaPB,0,0);
    schemaGrid->addWidget(copySchemaPB,0,1);
    schemaGrid->addWidget(editSchemaPB,1,0);
    schemaGrid->addWidget(deleteSchemaPB,1,1);
    sbox->setMargin(0);
    sbox->setSpacing(0);
    sbox->addWidget(newSchemaStackArea,1);
    sbox->addWidget(descriptionBox,0);

    buttonStackArea->insertWidget(RegMode,schemaButtonArea);
    buttonStackArea->insertWidget(NewMode,schemaButtonEditArea);


    sbox->addWidget(buttonStackArea,0,Qt::AlignBottom);
    qDebug() << "SCMEMA 50";

    messageListView = new QListView();
    availableListView = new QListView();
    selectedListView = new QListView();
    wgrid->addWidget(availableSchemasL,0,0,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(messageListL,0,1,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(availableListL,0,2,Qt::AlignHCenter|Qt::AlignBottom);
    wgrid->addWidget(selectedListL,0,3,Qt::AlignHCenter|Qt::AlignBottom);

    wgrid->addWidget(schemaArea,1,0);
    wgrid->addWidget(messageListView,1,1);
    wgrid->addWidget(availableListView,1,2);
    wgrid->addWidget(selectedListView,1,3);
    wgrid->setMargin(3);
    wgrid->setSpacing(4);

    wgrid->setRowStretch(0,0);
    wgrid->setRowStretch(1,1);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    closeB = buttonBox->addButton(QDialogButtonBox::Close);
    applyB = buttonBox->addButton(QDialogButtonBox::Apply);
    connect(buttonBox,SIGNAL(clicked(QAbstractButton*)),
                this,SLOT(actionButtonSlot(QAbstractButton*)));
    vbox->addWidget(titleArea,0);
    vbox->addWidget(hrLine,0);
    vbox->addLayout(topBox,0);
    vbox->addSpacing(22);
    vbox->addWidget(workWidget,1);
    vbox->addSpacing(24);
    vbox->addWidget(buttonBox,0);
    qDebug() << "SCMEMA 80";

    populateMessageList();
    newSchemaStackArea->setCurrentIndex(RegMode);
    buttonStackArea->setCurrentIndex(RegMode);
    viewMode = RegMode;
    qDebug() << "SCMEMA 81";

}
void SchemaEditorDialog::populateMessageList()
{
    QStandardItem *item;
    qDebug() << "SCMEMA 100";

    messageModel = new QStandardItemModel();
    messageListView->setModel(messageModel);
    int count = Globals::messagePairs->count();
    messageModel->setRowCount(count);
    qDebug() << "SCMEMA 130";

    for(int i= 0;i< count;i++) {
        qDebug() << "Create item I = " << i << "total = " << count;
       item = new QStandardItem(Globals::messagePairs->at(i).second);
       messageModel->setItem(i,item);
    }
    qDebug() << "SCMEMA 150";


}
void SchemaEditorDialog::applyButtonSlot(QAbstractButton *button)
{
    if (button == applyOnlyToCurrentRB) {
        windowL->show();
        windowV->show();
        tabL->show();
        tabV->show();
    }
    else if (button == applyToWindowRB) {
        windowL->show();
        windowV->show();
        tabL->hide();
        tabV->hide();
    }
    else { // must be apply to all
        windowL->hide();
        windowV->hide();
        tabL->hide();
        tabV->hide();
    }
}
void SchemaEditorDialog::setCurrentTarget(QString &windowName, QString &tabName)
{
    windowV->setText(windowName);
    tabV->setText(tabName);
}
void SchemaEditorDialog::actionButtonSlot(QAbstractButton *button )
{
 if (button == closeB)
     emit finished(QDialogButtonBox::Close);
}
void SchemaEditorDialog::newSchemaSlot()
{
    viewMode = NewMode;
    buttonStackArea->setCurrentIndex(NewMode);
    newSchemaStackArea->setCurrentIndex(NewMode);
    QPalette pal = descriptionE->palette();
    pal.setColor(QPalette::Base,editColor);
    qDebug() << "NEw ITEM 2";
    descriptionE->setPalette(pal);
    descriptionE->setReadOnly(false);
    availableSchemasL->setText("New Schema");
    validate();
    qDebug() << "NEW ITEM 5";
}
void SchemaEditorDialog::cancelNewSlot()
{
    viewMode = RegMode;
    newSchemaStackArea->setCurrentIndex(RegMode);
    buttonStackArea->setCurrentIndex(RegMode);
    QPalette pal = descriptionE->palette();
    pal.setColor(QPalette::Base,regularColor);
    descriptionE->setPalette(pal);
    descriptionE->setText("");
    descriptionE->setReadOnly(true);
    availableSchemasL->setText("Available Schemas");
    validate();
}
bool SchemaEditorDialog::validate()
{
    bool isValid = false;
    if (viewMode == NewMode) {
        if (newSchemaLine->text().length() > 1)
            isValid = true;
        saveEditPB->setEnabled(isValid);
    }
    return isValid;
}
void SchemaEditorDialog::nameEditedSlot(const QString &)
{
    validate();
}
