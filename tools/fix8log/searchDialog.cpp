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

#include "searchDialog.h"
#include "lineedit.h"
#include "database.h"
#include "globals.h"
#include "tableschema.h"
#include <QAbstractItemDelegate>
#include <QXmlStreamWriter>
#include <QtScript>
class FileFilterProxyModel : public QSortFilterProxyModel
{
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

bool FileFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
    return fileModel->fileName(index0).indexOf(".backup.") < 0;
    // uncomment to call the default implementation
    //return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
SearchDialog::SearchDialog(Database *db,TableSchema *ts,DialogType dt,QWidget *parent) :
    QDialog(parent),database(db),tableSchema(ts),haveError(false),strModel(0),
    aliasItem(0),functionItem(0),mainWindow(0),searchFunctionList(0),dialogType(dt)
{
    QString str;
    QFont fnt = font();
    fnt.setBold(true);
    setFont(fnt);
    // setWindowIcon(QIcon(":/images/vicon22.png"));
    // QFile file(":/SearchDialog.qss");
    //  file.open(QFile::ReadOnly);
    // QString styleSheet = QLatin1String(file.readAll());
    // setStyleSheet(styleSheet);


    setWindowTitle(GUI::Globals::appName);
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
    connect(closeB,SIGNAL(clicked()),this,SLOT(closeSlot()));

    closeB->setToolTip(tr("Close this dialog"));
    closeB->setStatusTip(tr("Close this dialog"));
    closeB->setWhatsThis(tr("Will close this dialog, but will not explicityly apply any changes"));


    QWidget *titleArea = new QWidget(this);
    QHBoxLayout *titleBox = new QHBoxLayout(titleArea);
    titleArea->setLayout(titleBox);
    iconL = new QLabel(titleArea);

    if (dialogType == SearchDialogType) {
        str = "Search Methods";
        iconL->setPixmap(QPixmap(":/images/svg/searchEdit.svg").scaledToHeight(32,Qt::SmoothTransformation));
    }
    else {
        str = "Filter Methods";
        iconL->setPixmap(QPixmap(":/images/svg/filterEdit.svg").scaledToHeight(32,Qt::SmoothTransformation));
    }
    titleL = new QLabel(str,this);
    titleBox->addWidget(iconL,0,Qt::AlignLeft);
    titleBox->addWidget(titleL,0,Qt::AlignLeft);
    titleBox->addSpacing(1);

    fnt  = titleL->font();
    fnt.setPointSize(fnt.pointSize() + 1);
    fnt.setBold(true);
    titleL->setFont(fnt);
    titleL->setAlignment(Qt::AlignLeft);

    editArea = new QGroupBox("Edit");
    QGridLayout *egrid = new QGridLayout();
    editArea->setLayout(egrid);
    aliasEdit = new QLineEdit();
    fnt = aliasEdit->font();
    fnt.setPointSize(fnt.pointSize() + 2);
    fnt.setBold(true);
    aliasEdit->setFont(fnt);
    //QRegExp nameRegExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]*$");
    QRegExp nameRegExp("^\\S+(\\s\\S+)+$");
    QRegExpValidator *nameValidator = new QRegExpValidator(nameRegExp,this);
    aliasEdit->setValidator(nameValidator);
    connect(aliasEdit,SIGNAL(textChanged(QString)),this,SLOT(aliasChangedSlot(QString)));
    aliasEdit->setMaxLength(24);
    functionEdit = new LineEdit();
    functionEdit->setFont(fnt);

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
    connect(tableView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editSearchItemSlot(QModelIndex)));
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
    exportB->setToolTip(("Export functions to file"));
    connect(exportB,SIGNAL(clicked()),this,SLOT(exportSlot()));
    importB = new QPushButton("Import...",this);
    importB->setToolTip(("Import functions from file"));
    connect(importB,SIGNAL(clicked()),this,SLOT(importSlot()));
    bbox->addWidget(newB,0,Qt::AlignTop);
    bbox->addWidget(editB,0,Qt::AlignTop);
    bbox->addWidget(deleteB,0,Qt::AlignTop);
    bbox->addStretch(1);
    bbox->addWidget(exportB,0,Qt::AlignBottom);
    bbox->addWidget(importB,0,Qt::AlignBottom);
    wbox->addWidget(tableView,1);
    wbox->addLayout(bbox,0);
    messageL = new QLabel(this);
    messageL->setFont(fnt);
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
    QString str;
    tableSchema = ts;
    QString colName;
    QStringList colNameList;
    if (searchFunctionList) {
        qDeleteAll(searchFunctionList->begin(),searchFunctionList->end());
        delete searchFunctionList;
        searchFunctionList = 0;
    }
    model->removeRows(0,model->rowCount());
    if (!database) {
        qWarning() << "Error database is null";
        return;
    }
    int colCount = tableSchema->fieldNames.count();
    for(int i=0;i<colCount;i++) {
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
    if (dialogType == SearchDialogType)
        searchFunctionList = database->getSearchFunctions();
    else
        searchFunctionList = database->getFilterFunctions();
    if (!searchFunctionList || (searchFunctionList->count() < 1) ) {
        if (dialogType == SearchDialogType)
            str = "No Search Functions In Database";
        else
            str = "No Filter Functions In Database";
        setMessage(str,false);
    }
    else
        populateSearchFunctions();
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
void SearchDialog::editSearchItemSlot(QModelIndex mi)
{

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
    QString str;
    QScriptEngine engine;
    QString errorMessage;
    QScriptSyntaxCheckResult::State syntaxState;
    SearchFunction *sf;

    if (!database) {
        setMessage("Save failed as database is null");
        return;
    }
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
        sf = new SearchFunction();
        sf->alias = aliasEdit->text();
        sf->function = functionEdit->toPlainText();
        if (dialogType == SearchDialogType)
            bstatus = database->addSearchFunction(*sf);
        else
            bstatus = database->addFilterFunction(*sf);

        if (!bstatus) {
            if (dialogType == SearchDialogType)
                str = "Error saving search item to database";
            else
                str = "Error saving filter item to database";

            setMessage(str);
            mode = ViewMode;
            validate();
            return;
        }
        else {
            setMessage("Saved To Database",false);
        }
        if (!searchFunctionList)
            searchFunctionList = new SearchFunctionList();

        searchFunctionList->append(sf);
        QVariant var;
        var.setValue((void *) sf);
        QStandardItem *aliasItem = new QStandardItem(aliasEdit->text());
        QStandardItem *funcItem = new QStandardItem(functionEdit->toPlainText());
        aliasItem->setData(var);
        funcItem->setData(var);
        QList <QStandardItem *> items;
        items.append(aliasItem);
        items.append(funcItem);
        model->appendRow(items);
    }
    else {// must be EditMode
        if (aliasItem && functionItem) {
            QVariant var = aliasItem->data();
            sf = (SearchFunction *) var.value<void *>();
            sf->alias = aliasEdit->text();
            sf->function = functionEdit->toPlainText();
            if (dialogType == SearchDialogType)
                bstatus = database->updateSearchFunction(*sf);
            else
                bstatus = database->updateFilterFunction(*sf);

            if (!bstatus) {
                if (dialogType == SearchDialogType)
                    str = "Update of search method failed";
                else
                    str = "Update of filter method failed";


                setMessage(str);
                mode = ViewMode;
                validate();
                return;
            }
            aliasItem->setText(aliasEdit->text());
            functionItem->setText(functionEdit->toPlainText());
        }
    }
    aliasEdit->clear();
    functionEdit->setText("");
    mode = ViewMode;
    validate();
    qDebug() << "Emit Update Search Functions" << __FILE__ << __LINE__;
    emit updatedSearchFunctions(searchFunctionList);
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
        if (model->rowCount() < 1)
            exportB->setEnabled(false);
        else
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
    saveSlot();
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
    bool bstatus;
    QString str;
    QItemSelectionModel *selectionModel;
    selectionModel = tableView->selectionModel();
    if (!selectionModel->hasSelection()) {
        validate();
        return;
    }
    if(!database) {
        setMessage("Delete failed as database is null");
        validate();
        return;
    }
    QModelIndexList selectedItems = selectionModel->selectedRows();
    QModelIndex mi = selectedItems.at(0);
    QVariant var  = mi.data(Qt::UserRole + 1);
    if (!var.isValid()) {
        setMessage("Failed to delete item from database, have null value");
        validate();
        return;
    }
    SearchFunction *sf = (SearchFunction *) var.value<void *>();
    if (!sf) {
        setMessage("Failed to delete item from database, have null value2");
        validate();
        return;
    }
    if (searchFunctionList)
        searchFunctionList->removeOne(sf);
    if (dialogType == SearchDialogType)
        bstatus = database->removeSearchFunction(sf->id);
    else
        bstatus = database->removeFilterFunction(sf->id);
    if (!bstatus)
        setMessage("Failed to delete item from database");
    else
        setMessage("Deleted Function From Database",false);
    model->removeRow(mi.row());
    validate();
    emit updatedSearchFunctions(searchFunctionList);
}
void SearchDialog::populateSearchFunctions()
{
    SearchFunction *sf;
    if (!searchFunctionList)
        return;
    model->setRowCount(searchFunctionList->count());
    QListIterator<SearchFunction *> iter(*searchFunctionList);
    int row = 0;
    while(iter.hasNext()) {
        QVariant var;
        sf = iter.next();
        var.setValue((void *) sf);
        QStandardItem *aliasItem = new QStandardItem(sf->alias);
        aliasItem->setData(var);
        QStandardItem *funcItem = new QStandardItem(sf->function);
        funcItem->setData(var);
        model->setItem(row,0,aliasItem);
        model->setItem(row,1,funcItem);
        row++;
    }
}
void SearchDialog::setNewMode(QString searchStr)
{
    newSearchSlot();
    functionEdit->setText(searchStr);
    validate();
}
void SearchDialog::closeEvent(QCloseEvent *ce)
{
    aliasEdit->clear();
    functionEdit->setText("");
    mode = ViewMode;
    validate();
    QDialog::closeEvent(ce);
}
void SearchDialog::closeSlot()
{
    aliasEdit->clear();
    functionEdit->setText("");
    mode = ViewMode;
    validate();
    emit accepted();
}
void SearchDialog::exportSlot()
{
    QString str;
    QString str1;
    QSettings settings("fix8","logviewer");
    if (model->rowCount() < 1) {
        str = "Error -  no functions to export";
        QMessageBox::warning(this,GUI::Globals::appName,str,QMessageBox::Ok);
        return;
    }
    if (dialogType == SearchDialogType)
        str = "Export Search Functions";
    else
        str = "Export Filter Functions";

    QFileDialog *fileDialog = new QFileDialog(this,"Export Search Functions");
    if  (dialogType == SearchDialogType)
        str ="ExportImportSearchDir";
    else
        str ="ExportImportFilterDir";

    QString workingDir = settings.value(str,QDir::homePath()).toString();
    fileDialog->setDirectory(workingDir);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setProxyModel(new FileFilterProxyModel());
    fileDialog->setNameFilter("XML (*.xml)");
    int status = fileDialog->exec();
    if (status == QDialog::Accepted) {
        QString fileName = fileDialog->selectedFiles().at(0);
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if ((ext != "xml") && (ext != "XML")) {
            fileName.append(".xml");
            fi = QFileInfo(fileName);
            if (fi.exists()) {
                str1 = "Error - File " + fi.fileName() + " already exists";
                QMessageBox::warning(this,GUI::Globals::appName,str1,QMessageBox::Ok);
                return;
            }
            qDebug() << "Ask if want to overwrite" << __FILE__ << __LINE__;

        }
        settings.setValue(str,fi.absolutePath());
        writeXML(fileName);
    }
    fileDialog->deleteLater();
}
void SearchDialog::importSlot()
{
    QString str;
    if  (dialogType == SearchDialogType)
        str = "Export Search Functions";
    else
        str = "Export Filter Functions";
    QFileDialog *fileDialog = new QFileDialog(this,str);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setProxyModel(new FileFilterProxyModel());
    fileDialog->setNameFilter("XML (*.xml)");
    int status = fileDialog->exec();
    if (status == QDialog::Accepted) {
        qDebug() << "Accepted" << __FILE__ << __LINE__;
        QString fileName = fileDialog->selectedFiles().at(0);
        readXML(fileName);
    }
    fileDialog->deleteLater();
}
void SearchDialog::writeXML(QString &fileName)
{
    QString str;
    bool bstatus;
    QFile file(fileName);
    QStandardItem *aliasItem;
    QStandardItem *funcItem;
    bstatus = file.open(QIODevice::WriteOnly);
    if (!bstatus) {
        str = "Error - unable to open file, " + fileName;
        QMessageBox::warning(this,GUI::Globals::appName,str,QMessageBox::Open,QMessageBox::NoButton);
        return;
    }
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeDTD("<!DOCTYPE fix8search>");
    str = "Generated " + QDateTime::currentDateTime().toString() + " with " + GUI::Globals::appName + " " + GUI::Globals::versionStr;
    xmlWriter.writeComment(str);
    xmlWriter.writeStartElement("searchlist");
    int rowCount = model->rowCount();
    for (int i=0;i<rowCount;i++) {
        xmlWriter.writeStartElement("function");
        aliasItem = model->item(i,0);
        funcItem = model->item(i,1);
        xmlWriter.writeAttribute("alias",aliasItem->text());
        xmlWriter.writeCharacters(funcItem->text());
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
}
void SearchDialog::readXML(QString &fileName)
{
    QString str;
    bool bstatus;
    QFile file(fileName);
    QString funcStr;
    QString aliasStr;
    QStringRef aliasStrR;

    QStandardItem *aliasItem;
    QStandardItem *funcItem;
    QXmlStreamAttributes attributes;
    bstatus = file.open(QIODevice::ReadOnly);
    if (!bstatus) {
        str = "Error - unable to open file, " + fileName;
        QMessageBox::warning(this,GUI::Globals::appName,str,QMessageBox::Open,QMessageBox::NoButton);
        return;
    }
    int i = 0;
    QXmlStreamReader xmlReader(&file);
    int numOfImported = 0;
    while (!xmlReader.atEnd()) {
        QList <QStandardItem *> items;
        if  (xmlReader.readNextStartElement()) {
            if (xmlReader.name() == "function") {
                attributes = xmlReader.attributes();
                funcStr = xmlReader.readElementText();
                if (attributes.hasAttribute("alias")) {
                    aliasStr = attributes.value("alias").toString();
                    aliasItem = new QStandardItem(aliasStr);
                    funcItem  = new QStandardItem(funcStr);
                    items.append(aliasItem);
                    items.append(funcItem);
                    model->appendRow(items);
                    SearchFunction *sf = new SearchFunction();
                    sf->alias = aliasStr;
                    sf->function = funcStr;
                    QVariant var;
                    var.setValue((void *) sf);
                    aliasItem->setData(var);
                    funcItem->setData(var);
                    if (dialogType == SearchDialogType)
                        database->addSearchFunction(*sf);
                    else
                        database->addFilterFunction(*sf);

                    if (!searchFunctionList) {
                        searchFunctionList = new SearchFunctionList();
                    }
                    searchFunctionList->append(sf);
                    numOfImported ++;
                }
            }
        }
    }
    if (numOfImported > 0) {
        str = "Imported " + QString::number(numOfImported) + " functions.";
        setMessage(str,false);
    }
    else {
        str = "Import failed, no functions found.";
        setMessage(str);
    }
    validate();
}
