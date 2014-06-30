#include "searchDialog.h"
#include "database.h"
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
    qint32 screenHeightMax = (geom.height() *.40);
    qint32 screenWidthMax = (geom.width() *.33);

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


    workArea = new QWidget(this);
    QHBoxLayout *wbox = new QHBoxLayout(this);
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
    tableView->setModel(model);
    bbox = new QVBoxLayout(this);
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

    vbox->addWidget(titleArea,0,Qt::AlignLeft);
    vbox->addWidget(workArea,1);
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
    if (mode != ViewMode)
        return;
    QStandardItem *aliasItem = new QStandardItem();
    QStandardItem *funcItem = new QStandardItem();
    QList <QStandardItem *> items;
    items.append(aliasItem);
    items.append(funcItem);
    model->appendRow(items);
    QModelIndex mi = funcItem->index();
    tableView->edit(mi);

    mode = EditMode;
    validate();
}

void SearchDialog::editSearchSlot()
{
    validate();
}
void SearchDialog::rowSelectedSlot(QModelIndex)
{

}
void SearchDialog::validate()
{

}
