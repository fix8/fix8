#include "schemaeditordialog.h"
#include "globals.h"
using namespace GUI;
SchemaEditorDialog::SchemaEditorDialog(QWidget *parent) :
    QDialog(parent)
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
    applyToWindowRB = new QRadioButton("Apply To Window",this);
    applyToAllRB = new QRadioButton("Apply To All",this);
    applyBG->addButton(applyOnlyToCurrentRB);
    applyBG->addButton(applyToWindowRB);
    applyBG->addButton(applyToAllRB);
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
    windowV = new QLabel("");

    tabL = new QLabel("Tab:");
    tabL->setFont(fnt);
    tabV = new QLabel("");
    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setItalic(true);
    windowV->setFont(fnt);
    tabV->setFont(fnt);
    tarBox->addWidget(windowL,0,Qt::AlignLeft);
    tarBox->addWidget(windowV,1,Qt::AlignLeft);
    tarBox->addSpacing(22);
    tarBox->addWidget(tabL,0);
    tarBox->addWidget(tabV,1,Qt::AlignLeft);

    topBox->addWidget(applyArea,0,Qt::AlignLeft);
    topBox->addWidget(targetArea,0,Qt::AlignLeft);
    topBox->addStretch(1);


    QWidget *workWidget = new QWidget();
    QGridLayout *wgrid = new QGridLayout();
    workWidget->setLayout(wgrid);

    availableSchemasL = new QLabel("Available Schemas");
    messageListL  = new QLabel("Messages");
    availableListL = new QLabel("Available Columns");
    selectedListL = new QLabel("Selected Columns");

    availableSchemasL->setAlignment(Qt::AlignHCenter);
    messageListL->setAlignment(Qt::AlignHCenter);
    availableListL->setAlignment(Qt::AlignHCenter);
    selectedListL->setAlignment(Qt::AlignHCenter);

    schemaArea = new QWidget(this);
    QVBoxLayout *sbox = new QVBoxLayout();
    schemaArea->setLayout(sbox);

    availableSchemasListView = new QListView();

    QWidget *schemaButtonArea = new QWidget(this);
    QGridLayout *schemaGrid = new QGridLayout();
    schemaButtonArea->setLayout(schemaGrid);

    newSchemaPB = new QPushButton("New",schemaArea);
    copySchemaPB = new QPushButton("Copy",schemaArea);
    editSchemaPB = new QPushButton("Edit",schemaArea);
    deleteSchemaPB = new QPushButton("Delete",schemaArea);
    schemaGrid->addWidget(newSchemaPB,0,0);
    schemaGrid->addWidget(copySchemaPB,0,1);
    schemaGrid->addWidget(editSchemaPB,1,0);
    schemaGrid->addWidget(deleteSchemaPB,1,1);
    sbox->setMargin(0);
    sbox->setSpacing(0);
    sbox->addWidget(availableSchemasListView,1);
    sbox->addWidget(schemaButtonArea,0,Qt::AlignBottom);

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

    vbox->addWidget(titleArea,0);
    vbox->addWidget(hrLine,0);
    vbox->addLayout(topBox,0);
    vbox->addSpacing(22);
    vbox->addWidget(workWidget,2);
    vbox->addStretch(1);
    vbox->addWidget(buttonBox,0);
    populateMessageList();
}
void SchemaEditorDialog::populateMessageList()
{
    QStandardItem *item;
    messageModel = new QStandardItemModel();
    messageListView->setModel(messageModel);
    int count = Globals::messagePairs->count();
    messageModel->setRowCount(count);
    for(int i= 0;i< count;i++) {
       item = new QStandardItem(Globals::messagePairs->at(i).second);
       messageModel->setItem(i,item);
    }

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
