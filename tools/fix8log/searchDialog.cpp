#include "searchDialog.h"
#include "lineedit.h"
#include "database.h"
#include <QAbstractItemDelegate>

SearchDialog::SearchDialog(Database *db,QWidget *parent) :
    QDialog(parent),database(db)
{

    // setWindowIcon(QIcon(":/images/vicon22.png"));
    // QFile file(":/SearchDialog.qss");
    //  file.open(QFile::ReadOnly);
    // QString styleSheet = QLatin1String(file.readAll());
    // setStyleSheet(styleSheet);


    setWindowTitle("Fix8 Logviewer");
    QDesktopWidget *desktop = qApp->desktop();
    QRect geom = desktop->availableGeometry(this);
    qint32 screenHeightMax = (geom.height() *.50);
    qint32 screenWidthMax = (geom.width() *.50);

    setMaximumSize(screenWidthMax,screenHeightMax);
    QVBoxLayout *vbox = new QVBoxLayout();
    setLayout(vbox);


    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);

    closeB = buttonBox->addButton(QDialogButtonBox::Close);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //connect(closeB,SIGNAL(clicked()),this,SIGNAL(accepted()));

    closeB->setToolTip(tr("Close this dialog"));
    closeB->setStatusTip(tr("Close this dialog"));
    closeB->setWhatsThis(tr("Will close this dialog, but will not explicityly apply any changes"));


    QWidget *titleArea = new QWidget(this);
    QHBoxLayout *titleBox = new QHBoxLayout(titleArea);
    titleArea->setLayout(titleBox);
    iconL = new QLabel(titleArea);
    iconL->setPixmap(QPixmap(":/images/svg/magniflying_glass.svg").scaledToHeight(32,Qt::SmoothTransformation));
    titleL = new QLabel("Search Filters",this);
    titleBox->addWidget(iconL,0,Qt::AlignLeft);
    titleBox->addWidget(titleL,0,Qt::AlignLeft);
    titleBox->addSpacing(1);

    QFont font;
    font.setPointSize(font.pointSize() + 1);
    font.setBold(true);
    titleL->setFont(font);
    titleL->setAlignment(Qt::AlignLeft);

    editArea = new QGroupBox("Edit");
    QGridLayout *egrid = new QGridLayout();
    editArea->setLayout(egrid);
    aliasEdit = new QLineEdit();
    aliasEdit->setMaxLength(24);
    functionEdit = new LineEdit();
    aliasL = new QLabel("Alias",this);
    functionL = new QLabel("Function",this);


    QVBoxLayout *ebuttonBox = new QVBoxLayout();
    ebuttonBox->setMargin(0);
    saveB = new QPushButton("Save",this);
    connect(saveB,SIGNAL(clicked()),this,SLOT(saveSlot()));
    cancelB = new QPushButton("Cancel",this);
    connect(cancelB,SIGNAL(clicked()),this,SLOT(cancelSlot()));

    ebuttonBox->addWidget(saveB,0);
    ebuttonBox->addWidget(cancelB,0);
    egrid->setMargin(0);
    egrid->addWidget(aliasL,0,0);
    egrid->addWidget(functionL,0,1);
    egrid->addWidget(aliasEdit,1,0,Qt::AlignTop);
    egrid->addWidget(functionEdit,1,1,Qt::AlignTop);
    egrid->addLayout(ebuttonBox,0,2,2,1,Qt::AlignVCenter);
    egrid->setRowStretch(0,0);
    egrid->setRowStretch(1,0);
    egrid->setColumnStretch(0,0);
    egrid->setColumnStretch(1,1);
    egrid->setColumnStretch(2,0);

    workArea = new QWidget(this);
    QHBoxLayout *wbox = new QHBoxLayout(workArea);
    workArea->setLayout(wbox);

    tableView = new QTableView(this);
    model = new QStandardItemModel(this);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSortingEnabled(true);
    tableView->setAlternatingRowColors(true);
    connect(tableView,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    QStringList headers;
    headers << "Alias" << "Function";
    model->setHorizontalHeaderLabels(headers);
    delegate = new QStyledItemDelegate(tableView);
    QItemEditorFactory *editFactory = new QItemEditorFactory();
    searchCompleter = new QCompleter(this);
    searchCompleter->setCompletionMode(QCompleter::InlineCompletion);
    searchCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    searchCompleter->setWrapAround(false);
    QStringList strList;
    strList << "David" << "Maria" << "Johnny";
    QStringListModel *strModel = new QStringListModel(strList,this);

    searchCompleter->setModel(strModel);
    le = new LineEdit();
    le->setCompleter(searchCompleter);
    le = (LineEdit *) editFactory->createEditor(QVariant::String,le);
    \
    delegate->setItemEditorFactory(editFactory);
    tableView->setItemDelegateForColumn(1,delegate);
    tableView->setModel(model);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bbox = new QVBoxLayout();
    newB = new QPushButton("New",this);
    connect(newB,SIGNAL(clicked()),this,SLOT(newSearchSlot()));
    undoB = new QPushButton("Undo",this);
    deleteB = new QPushButton("Delete",this);

    exportB = new QPushButton("Export...",this);
    importB = new QPushButton("Import...",this);

    bbox->addWidget(newB,0,Qt::AlignTop);
    bbox->addWidget(undoB,0,Qt::AlignTop);
    bbox->addWidget(deleteB,0,Qt::AlignTop);
    bbox->addStretch(1);
    bbox->addWidget(exportB,0,Qt::AlignBottom);
    bbox->addWidget(importB,0,Qt::AlignBottom);
    wbox->addWidget(tableView,1);
    wbox->addLayout(bbox,0);


    editArea->hide();
    vbox->setMargin(5);
    vbox->setSpacing(0);
    vbox->addWidget(titleArea,0,Qt::AlignLeft);
    vbox->addWidget(workArea,1);
    vbox->addWidget(editArea,0);
    vbox->addSpacing(12);
    vbox->addWidget(buttonBox,0);
    resize(sizeHint());
    mode = ViewMode;
}

void SearchDialog::showEvent(QShowEvent *se)
{
    //validate();
    QDialog::showEvent(se);
}

QSize SearchDialog::sizeHint() const
{

    QSize s = QDialog::sizeHint();
    return s;
}
void SearchDialog::newSearchSlot()
{
    editArea->show();
    if (mode != ViewMode)
        return;
    /*
    QStandardItem *aliasItem = new QStandardItem();
    QStandardItem *funcItem = new QStandardItem();
    QList <QStandardItem *> items;
    items.append(aliasItem);
    items.append(funcItem);
    model->appendRow(items);
    QModelIndex mi = funcItem->index();
    tableView->edit(mi);
    */
    mode = EditMode;
    validate();
}

void SearchDialog::editSearchSlot()
{
    validate();
}
void SearchDialog::cancelSlot()
{
    mode = ViewMode;
    aliasEdit->clear();
    functionEdit->setText("");
    validate();
}
void SearchDialog::saveSlot()
{
    mode = ViewMode;
    QStandardItem *aliasItem = new QStandardItem(aliasEdit->text());
    QStandardItem *funcItem = new QStandardItem(functionEdit->toPlainText());
    QList <QStandardItem *> items;
    items.append(aliasItem);
    items.append(funcItem);
    model->appendRow(items);
    aliasEdit->clear();
    functionEdit->setText("");
    validate();
}
void SearchDialog::rowSelectedSlot(QModelIndex)
{

}
void SearchDialog::validate()
{
    if (mode == EditMode) {
        editArea->show();
        saveB->setEnabled(true);
        cancelB->setEnabled(true);
        newB->setEnabled(false);
        deleteB->setEnabled(false);
        importB->setEnabled(false);
        exportB->setEnabled(false);
    }
    else if (mode == ViewMode) {
        editArea->hide();
        saveB->setEnabled(false);
        cancelB->setEnabled(false);
        newB->setEnabled(true);
        deleteB->setEnabled(true);
        importB->setEnabled(true);
        exportB->setEnabled(true);
    }
}
