#include "newwindowschemapage.h"
NewWindowSchemaPage::NewWindowSchemaPage(Fix8SharedLibList &shareLibs,QWidget *parent) :
    QWizardPage(parent),fix8SharedLibList(shareLibs)
{
    setTitle("<h1>New Window Wizard</h1>");
    setSubTitle("<h2>Select FIX Schema</h2>");
    QGridLayout *schemaGrid= new QGridLayout(this);
    setLayout(schemaGrid);
    schemaStack = new QStackedLayout();
    schemaListView = new QListView(this);
    schemaListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    schemaListView->setFlow(QListView::TopToBottom);
    QFontMetrics fm(schemaListView->font());
    int schemaListViewWidth = fm.width("FIX4402")+4;
    //schemaListView->setMaximumWidth(schemaListViewWidth + 12);
    schemaListView->setUniformItemSizes(true);
    connect(schemaListView,SIGNAL(clicked(QModelIndex)),this,SLOT(schemaListViewSlot(QModelIndex)));
    //schemaListView->setMovement(QListView::Static);
    //schemaListView->setResizeMode(QListView::Adjust);
    schemaModel = new QStandardItemModel(this);
    schemaListView->setModel(schemaModel);
    selectionModel = schemaListView->selectionModel();
    QFont fnt = schemaListView->font();
    fnt.setPointSize(fnt.pointSize()+2);
    fnt.setBold(true);
    schemaListView->setFont(fnt);
    noSchemasFoundL = new QLabel(this);
    QString ss = "QLabel { color: rgb(255,255,255); border-color: rgba(255, 0, 0, 75%); background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #1a3994, stop: 1 #061a33); }";
    noSchemasFoundL->setStyleSheet(ss);
    noSchemasFoundL->setText("No Schemas\nFound");
    fnt = noSchemasFoundL->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize()+2);
    noSchemasFoundL->setFont(fnt);
    noSchemasFoundL->setAlignment(Qt::AlignCenter);
    schemasListID = schemaStack->addWidget(schemaListView);
    noSchemasID = schemaStack->addWidget(noSchemasFoundL);
    schemaStack->setCurrentIndex(noSchemasID);
    fm = QFontMetrics(schemaListView->font());
    schemaListView->setMaximumWidth(fm.averageCharWidth()*24);
    noSchemasFoundL->setMaximumWidth(fm.averageCharWidth()*24);
    legend = new QLabel(this);
    fnt = legend->font();
    fnt.setItalic(true);
    legend->setFont(fnt);
#ifdef Q_OS_WIN
    legend->setText("*System Library");
#else
    legend->setText("\u002aSystem Library");
#endif
    schemaLabel = new QLabel(this);
    schemaLabel->setWordWrap(true);
    schemaLabel->setTextFormat(Qt::RichText);
    systemDirName = qApp->applicationDirPath() + "/fixschemas";
    userDirName = QDir::homePath() + "/f8logview/fixschemas";
    QString str = "<h1>Select Schema</h1>";
    str.append("Each window is associated with one FIX schema. These schema's (listed here) are loaded from two locations:");
    str.append("<p><ul><li>System Level- loaded from the fix8logview install directory - <i>" + systemDirName+ "</i></li>");
    str.append("<li>User Level -loaded from user's subdirectory <i>" + userDirName + "</i></li>");
    schemaLabel->setText(str);
    schemaGrid->addLayout(schemaStack,0,0);
    schemaGrid->addWidget(legend,1,0,Qt::AlignLeft);
    schemaGrid->addWidget(schemaLabel,0,1,2,1,Qt::AlignLeft);
    schemaGrid->setColumnStretch(0,0);
    schemaGrid->setColumnStretch(1,1);
    schemaGrid->setRowStretch(0,1);
    schemaGrid->setRowStretch(1,0);
}
bool NewWindowSchemaPage::isComplete() const
{
    bool haveSelection = false;
    if (selectionModel->hasSelection())
        haveSelection = true;
    return haveSelection;
}
void NewWindowSchemaPage::schemaListViewSlot(QModelIndex)
{
    emit completeChanged();
}
bool NewWindowSchemaPage::loadSchemas(Fix8SharedLib::LibType libType)
{
    QDir schemaDir;
    QString errorStr;
    if (libType == Fix8SharedLib::SystemLib)
        schemaDir = QDir(systemDirName);
    else
        schemaDir = QDir(userDirName);
    if (!schemaDir.exists()) {
        errorStr = "Schema Dir" + schemaDir.path() + " does not exists";
        qWarning() << errorStr << __FILE__ << __LINE__;
        schemaErrorStrList.append(errorStr);
        return false;
    }
    qDebug() << "Load Schemas: Dir = " << schemaDir.absolutePath() << __FILE__ << __LINE__;
    QFileInfoList fileInfoList = schemaDir.entryInfoList(QDir::Files |QDir::NoDotAndDotDot| QDir::NoSymLinks);
    QFileInfo fi;
    QStandardItem *si;
    QListIterator<QFileInfo> iter(fileInfoList);
    int i=0;
    QString nameList;
    while(iter.hasNext()) {
        QString baseName;
        fi = iter.next();
        qDebug() << "FILE PATH + " << fi.absoluteFilePath() << __FILE__ << __LINE__;
        baseName = fi.baseName();
        QString name;

#ifdef Q_OS_WIN
        name = baseName;
        if (libType == Fix8SharedLib::SystemLib)
            name.append("*");
#else
        QString libStr = baseName.left(3);
        if (libStr == "lib")
            name = baseName.right(baseName.length()-3);
        if (libType == Fix8SharedLib::SystemLib)
            name.append("\u002a");
#endif
        if (!nameList.contains(name)) {
            si = new QStandardItem(name);
            QVariant var;
            QString fileName = fi.absoluteFilePath();
            var.setValue(fileName);
            si->setData(var);
            schemaModel->insertRow(i++,si);
            nameList.append(name);
        }
    }
    return true;
}
QString  NewWindowSchemaPage::getSelectedLib()
{
    QString fileName;
    if (!selectionModel->hasSelection())
        return fileName;
    QList <QModelIndex> indexList = selectionModel->selectedIndexes();
    QModelIndex index = indexList.at(0);
    if (!index.isValid()) {
        qWarning() << "Invalid index " << __FILE__ << __LINE__;
        return fileName;
    }
    QVariant var = index.data(Qt::UserRole + 1);
    qDebug() << "GOT DATA = " << var.toString() << __FILE__ << __LINE__;
    fileName = var.toString();
    return fileName;
}
