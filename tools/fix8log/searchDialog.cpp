#include "searchDialog.h"
#include "lineedit.h"
#include "database.h"
#include "tableschema.h"
#include <QAbstractItemDelegate>
#include <QtScript>

SearchDialog::SearchDialog(Database *db,TableSchema *ts,QWidget *parent) :
    QDialog(parent),database(db),tableSchema(ts),haveError(false),strModel(0),
    aliasItem(0),functionItem(0),mainWindow(0)
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
    connect(closeB,SIGNAL(clicked()),this,SIGNAL(accepted()));

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
    //QRegExp nameRegExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]*$");
    QRegExp nameRegExp("^\\S+(\\s\\S+)+$");
    QRegExpValidator *nameValidator = new QRegExpValidator(nameRegExp,this);
    aliasEdit->setValidator(nameValidator);
    connect(aliasEdit,SIGNAL(textChanged(QString)),this,SLOT(aliasChangedSlot(QString)));
    aliasEdit->setMaxLength(24);
    functionEdit = new LineEdit();
    connect(functionEdit,SIGNAL(textChanged()),this,SLOT(functionChangedSlot()));
    connect(functionEdit,SIGNAL(returnPressed()),this,SLOT(functionReturnSlot()));
    aliasL = new QLabel("Alias",this);
    aliasL->setToolTip("Name to show in search pull-down menu");
    functionL = new QLabel("Function",this);
    functionL->setToolTip("Enter Valid Java Script Here (case sensitive)");


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

    functionEdit->setCompleter(searchCompleter);
    editHighlighter = new EditHighLighter(functionEdit->document());
    tableView->setItemDelegateForColumn(1,delegate);
    connect(tableView,SIGNAL(clicked(QModelIndex)),this,SLOT(rowSelectedSlot(QModelIndex)));
    tableView->setModel(model);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bbox = new QVBoxLayout();
    newB = new QPushButton("New",this);
    connect(newB,SIGNAL(clicked()),this,SLOT(newSearchSlot()));
    editB = new QPushButton("Edit",this);
    connect(editB,SIGNAL(clicked()),this,SLOT(editSearchSlot()));
    deleteB = new QPushButton("Delete",this);
    connect(deleteB,SIGNAL(clicked()),this,SLOT(deleteSlot()));
    exportB = new QPushButton("Export...",this);
    importB = new QPushButton("Import...",this);

    bbox->addWidget(newB,0,Qt::AlignTop);
    bbox->addWidget(editB,0,Qt::AlignTop);
    bbox->addWidget(deleteB,0,Qt::AlignTop);
    bbox->addStretch(1);
    bbox->addWidget(exportB,0,Qt::AlignBottom);
    bbox->addWidget(importB,0,Qt::AlignBottom);
    wbox->addWidget(tableView,1);
    wbox->addLayout(bbox,0);
    messageL = new QLabel(this);
    regPal = messageL->palette();
    errorPal = regPal;
    errorPal.setColor(QPalette::WindowText,Qt::red);
    editArea->hide();
    vbox->setMargin(5);
    vbox->setSpacing(0);
    vbox->addWidget(titleArea,0,Qt::AlignLeft);
    vbox->addWidget(workArea,1);
    vbox->addWidget(messageL);
    vbox->addWidget(editArea,0);
    vbox->addSpacing(12);
    vbox->addWidget(buttonBox,0);
    resize(sizeHint());
    mode = ViewMode;
}
void SearchDialog::setTableSchema(TableSchema *ts)
{
    tableSchema = ts;
    QString colName;
    QStringList colNameList;
    int rowCount = tableSchema->fieldNames.count();
    for(int i=0;i<rowCount;i++) {
        colName  = tableSchema->fieldNames[i];
        colNameList.append(colName);

    }
    colNameList.sort();
    editHighlighter->setColumHeaders(colNameList);
    if (strModel)
        delete strModel;
    strModel = new QStringListModel(colNameList,this);

    searchCompleter->setModel(strModel);
    functionEdit->setCompleter(searchCompleter);
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
    QString str;
    if (mode != ViewMode)
        return;
    if (!tableSchema) {
        str = "Error - Table Schema Not Set";
        QMessageBox::warning(this,"Fix8Log Viewer",str);
        return;
    }
    editArea->show();

    QString colName;
    QStringList colNameList;
    int rowCount = tableSchema->fieldNames.count();
    for(int i=0;i<rowCount;i++) {
        colName  = tableSchema->fieldNames[i];
        colNameList.append(colName);

    }
    colNameList.sort();
    QStringListModel *strModel = new QStringListModel(colNameList,this);
    searchCompleter->setModel(strModel);
    mode = NewMode;
    validate();
}

void SearchDialog::editSearchSlot()
{
    QItemSelectionModel *selectionModel;
    selectionModel = tableView->selectionModel();
    if (!selectionModel->hasSelection()) {
        validate();
        return;
    }
    QModelIndexList selectedItems = selectionModel->selectedRows();
    QModelIndex mi = selectedItems.at(0);
    if (!mi.isValid()) {
        qWarning() << "Error Edit Search Item, index is invalid " << __FILE__ << __LINE__;
        return;
    }
    int row = mi.row();
    aliasItem = model->item(row,0);
    if (!aliasItem) {
        qWarning() << "Error Edit Search Item, alias item  is invalid " << __FILE__ << __LINE__;
        return;
    }
    functionItem = model->item(row,1);
    if (!functionItem) {
        qWarning() << "Error Edit Search Item, function item  is invalid " << __FILE__ << __LINE__;
        return;
    }
    aliasEdit->setText(aliasItem->text());
    functionEdit->setText(functionItem->text());
    mode = EditMode;
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
    bool bstatus;
    QScriptEngine engine;
    QString errorMessage;
    QScriptSyntaxCheckResult::State syntaxState;
    QString newSearchStr = createSearchRoutine(bstatus);

    if (!bstatus) {
        setMessage("Error - invalid function");
        haveError = true;
        validate();
        return;
    }
    QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(newSearchStr);
    syntaxState =syntaxResult.state();
    if ( syntaxState == QScriptSyntaxCheckResult::Error) {
        errorMessage = syntaxResult.errorMessage();
        setMessage(errorMessage);
        haveError = true;
        validate();
        return;
    }
    if (mode == NewMode) {
        QStandardItem *aliasItem = new QStandardItem(aliasEdit->text());
        QStandardItem *funcItem = new QStandardItem(functionEdit->toPlainText());
        QList <QStandardItem *> items;
        items.append(aliasItem);
        items.append(funcItem);
        model->appendRow(items);
    }
    else {// must be EditMode
        if (aliasItem && functionItem) {
            aliasItem->setText(aliasEdit->text());
            functionItem->setText(functionEdit->toPlainText());
        }
    }
    aliasEdit->clear();
    functionEdit->setText("");
    mode = ViewMode;
    validate();
}
void SearchDialog::rowSelectedSlot(QModelIndex)
{
    validate();
}
void SearchDialog::validate()
{
    QItemSelectionModel *selectionModel;
    QString aliasStr = aliasEdit->text();
    QString funStr   = functionEdit->toPlainText();
    if (mode == EditMode || mode == NewMode) {
        tableView->setEnabled(false);
        if ((aliasStr.length() > 0) && (funStr.length() > 2))
            saveB->setEnabled(true);
        else
            saveB->setEnabled(false);
        editArea->show();
        cancelB->setEnabled(true);
        newB->setEnabled(false);
        editB->setEnabled(false);
        deleteB->setEnabled(false);
        importB->setEnabled(false);
        exportB->setEnabled(false);
    }
    else if (mode == ViewMode) {
        tableView->setEnabled(true);
        selectionModel = tableView->selectionModel();
        if (selectionModel->hasSelection()) {
            editB->setEnabled(true);
            deleteB->setEnabled(true);
        }
        else {
            editB->setEnabled(false);
            deleteB->setEnabled(false);
        }
        editArea->hide();
        saveB->setEnabled(false);
        cancelB->setEnabled(false);
        newB->setEnabled(true);
        importB->setEnabled(true);
        exportB->setEnabled(true);
    }
}
void SearchDialog::aliasChangedSlot(QString)
{
    haveError = false;
    messageL->clear();
    validate();
}
void SearchDialog::functionChangedSlot()
{
    haveError = false;
    messageL->clear();
    validate();
}
QString SearchDialog::createSearchRoutine(bool &bstatus)
{
    QString str;
    QString strValue;
    QString func = "(function(";
    if (!tableSchema) {
        bstatus = false;
        qWarning() << "Error, table schame is null" << __FILE__ << __LINE__;
        return str;
    }
    int rowCount = tableSchema->fieldNames.count();

    for(int i=0;i<rowCount;i++) {
        str  = tableSchema->fieldNames[i];
        func.append(str);
        if (i < rowCount -1) {
            func.append(",");
        }
    }
    func.append(") {return ");
    func.append(functionEdit->toPlainText());
    func.append(";})");
    bstatus = true;
    return func;
}
void SearchDialog::setMessage(QString str, bool isError)
{
    if (isError)
        messageL->setPalette(errorPal);
    else
        messageL->setPalette(regPal);
    messageL->setText(str);

}
void SearchDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
        event->ignore();
    else
        QDialog::keyPressEvent(event);
}
void SearchDialog::functionReturnSlot()
{
    bool bstatus;
    QString errorMessage;
    QScriptEngine engine;

    QScriptSyntaxCheckResult::State syntaxState;
    QString newSearchStr = createSearchRoutine(bstatus);

    QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(newSearchStr);
    syntaxState =syntaxResult.state();
    if ( syntaxState == QScriptSyntaxCheckResult::Error) {
        errorMessage = syntaxResult.errorMessage();
        setMessage(errorMessage);
        haveError = true;
        mode = ViewMode;
        validate();
        return;
    }
    QString aliasStr = aliasEdit->text();
    if (aliasStr.length() < 1) {
        mode = ViewMode;
        validate();
        return;
    }
    QStandardItem *aliasItem = new QStandardItem(aliasStr);
    QStandardItem *funcItem = new QStandardItem(functionEdit->toPlainText());
    QList <QStandardItem *> items;
    items.append(aliasItem);
    items.append(funcItem);
    model->appendRow(items);
    aliasEdit->clear();
    functionEdit->setText("");
    mode = ViewMode;
    validate();
}
void SearchDialog::setMainWindow(MainWindow *mw)
{
    mainWindow = mw;
}
MainWindow *SearchDialog::getMainWindow()
{
    return mainWindow;
}
void SearchDialog::deleteSlot()
{
    QItemSelectionModel *selectionModel;
    selectionModel = tableView->selectionModel();
    if (!selectionModel->hasSelection()) {
        validate();
        return;
    }
    QModelIndexList selectedItems = selectionModel->selectedRows();
    QModelIndex mi = selectedItems.at(0);
    model->removeRow(mi.row());
    validate();
}
