#include "embeddedfileselector.h"
#include <QDebug>

EmbeddedFileSelector::EmbeddedFileSelector(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    setLayout(vbox);
    QHBoxLayout *toolLayout = new QHBoxLayout();
    toolLayout->setMargin(0);
    directoryL = new QLabel("Look in:", this);
    directoryL->setAlignment(Qt::AlignLeft);
    directoryCB = new QComboBox(this);
    directoryCB->setEditable(false);
    connect(directoryCB,SIGNAL(activated(QString)),SLOT(directoryCBSlot(QString )));
    directoryBG = new QButtonGroup(this);
    connect(directoryBG,SIGNAL(buttonClicked(int)),this,SLOT(goBackForwardSlot(int)));
    backB = new QToolButton(this);
    backB->setArrowType(Qt::LeftArrow);
    forwardB = new QToolButton(this);
    forwardB->setArrowType(Qt::RightArrow);
    directoryBG->addButton(backB,DirectoryBack);
    directoryBG->addButton(forwardB,DirectoryForward);

    viewModeBG  = new QButtonGroup(this);
    listViewB   = new QToolButton(this);
    listViewB->setIcon(QIcon(":/images/svg/listView.svg"));
    detailViewB = new QToolButton(this);
    detailViewB->setIcon(QIcon(":/images/svg/detailView.svg"));
    viewModeBG->addButton(listViewB,ListView);
    viewModeBG->addButton(detailViewB,DetailView);
    connect(viewModeBG,SIGNAL(buttonClicked(int)),SLOT(viewModeSlot(int)));
    toolLayout->addWidget(directoryL,0);
    toolLayout->addWidget(directoryCB,1);
    toolLayout->addWidget(backB,0);
    toolLayout->addWidget(forwardB,0);
    toolLayout->addWidget(listViewB,0);
    toolLayout->addWidget(detailViewB,0);
    splitter = new QSplitter(Qt::Horizontal,this);
    quickSelectView = new QListView(splitter);
    quickSelectView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    quickSelectView->setFlow(QListView::TopToBottom);
    QFontMetrics fm(quickSelectView->font());
    int quickSelectViewWidth = fm.width("computer")+4;
    quickSelectView->setMaximumWidth(quickSelectViewWidth + 12);
    quickSelectView->setGridSize(QSize(90,quickSelectViewWidth));
    quickSelectView->setUniformItemSizes(true);
    quickSelectView->setMovement(QListView::Static);
    quickSelectView->setResizeMode(QListView::Adjust);
    quickSelectModel = new QStandardItemModel(quickSelectView);
    connect(quickSelectView,SIGNAL(clicked(QModelIndex)),this,SLOT(quickSelectViewSlot(QModelIndex)));
    quickSelectView->setModel(quickSelectModel);
    quickSelectView->setViewMode(QListView::IconMode);
    quickRootItem = new QStandardItem(QIcon(":/images/svg/harddrive.svg"),"Computer");
    quickHomeItem = new QStandardItem(QIcon(":/images/svg/user-home.svg"),"Home");
    quickSelectModel->insertRow(0,quickHomeItem);
    quickSelectModel->insertRow(1,quickRootItem);
    stackW = new QStackedWidget(splitter);
    listView = new QListView(stackW);
    listView->setFlow(QListView::LeftToRight);
    listView->setGridSize(QSize(94,fm.averageCharWidth()*12));
    listView->setUniformItemSizes(true);
    listView->setWrapping(true);
    listView->setViewMode(QListView::IconMode);
    listView->setResizeMode(QListView::Adjust);
    detailView = new QTableView(stackW);
    QHeaderView *verticalH = detailView->verticalHeader();
    verticalH->setVisible(false);
    detailView->setShowGrid(false);
    detailView->setSortingEnabled(true);
    QHeaderView *horizontalH = detailView->horizontalHeader();
    horizontalH->setStretchLastSection(true);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    detailView->setSelectionMode(QAbstractItemView::SingleSelection);
    detailView->setSelectionBehavior(QAbstractItemView::SelectRows);
    dirModel = new QFileSystemModel (this);
    dirModel->setFilter(QDir::Files| QDir::Dirs|QDir::NoDotAndDotDot);
    listView->setModel(dirModel);
    detailView->setModel(dirModel);
    detailViewSelectionModel =  detailView->selectionModel();
    listViewSelectionModel   =  listView->selectionModel();
    connect(listView,SIGNAL(clicked(QModelIndex)),this,SLOT(viewClickedSlot(QModelIndex)));
    connect(detailView,SIGNAL(clicked(QModelIndex)),this,SLOT(viewClickedSlot(QModelIndex)));
    stackW->insertWidget(ListView,listView);
    stackW->insertWidget(DetailView,detailView);
    splitter->addWidget(quickSelectView);
    splitter->addWidget(stackW);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);
    splitter->setStretchFactor(0,0);
    splitter->setStretchFactor(1,1);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setMargin(0);
    filterTypeL = new QLabel("Files Filter:");
    filterTypeCB = new QComboBox(this);
    QStringList filterList;
    filterList << "*.*" << "*.log";
    filterTypeCB->addItems(filterList);
    filterTypeCB->setEditable(true);
    filterLayout->addWidget(filterTypeL,0);
    filterLayout->addWidget(filterTypeCB,1);

    vbox->addLayout(toolLayout,0);
    vbox->addWidget(splitter,1);
    vbox->addLayout(filterLayout,0);
}
void EmbeddedFileSelector::saveSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    settings.setValue("splitter",splitter->saveState());
    settings.setValue("currentDir",currentDir.path());
    settings.setValue("viewmode",viewMode);
    settings.setValue("detailView",detailView->horizontalHeader()->saveState());
}
void EmbeddedFileSelector::readSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    QList<int> defaultSizeList;
    defaultSizeList << 100 << 500;
    splitter->setSizes(defaultSizeList);
    QByteArray defba = splitter->saveState();
    QByteArray ba =  settings.value("splitter",defba).toByteArray();


    splitter->restoreState(ba);
    currentDir.setPath(settings.value("currentDir",QDir::homePath()).toString());
    directoryCB->addItem(currentDir.path());
    stackW->setCurrentIndex(settings.value("viewmode",ListView).toInt());
    QModelIndex rootIndex = dirModel->setRootPath(currentDir.path());
    listView->setRootIndex(rootIndex);
    detailView->setRootIndex(rootIndex);
    addParentDir(currentDir);
    detailView->horizontalHeader()->restoreState(settings.value("detailView").toByteArray());

    validate();
}
void EmbeddedFileSelector::addParentDir(QDir dir)
{
    if (dir == QDir::root())
        return;
    dir.cdUp();
    directoryCB->addItem(dir.path());
    addParentDir(dir);
}
void EmbeddedFileSelector::directoryCBSlot(QString path)
{
    if (path == currentDir.path()) {
        return;
    }
    lastDirPath = currentDir.path();
    if (backDirList.count() > 9)
        backDirList.pop_front();
    backDirList.append(lastDirPath);
    currentDir.setPath(path);
    directoryCB->clear();
    directoryCB->addItem(currentDir.path());
    addParentDir(currentDir);
    int index = directoryCB->findText(path);
    if (directoryCB->findText(lastDirPath) < 0)
        directoryCB->addItem(lastDirPath);
    directoryCB->setCurrentIndex(index);
    QModelIndex rootIndex = dirModel->setRootPath(path);
    listView->setRootIndex(rootIndex);
    detailView->setRootIndex(rootIndex);
}
void EmbeddedFileSelector::viewModeSlot(int vm)
{
    if (viewMode == (ViewMode) vm)
        return;
    else
        viewMode = (ViewMode ) vm;
    stackW->setCurrentIndex(viewMode);
}
void EmbeddedFileSelector::viewClickedSlot(QModelIndex mi)
{
    QFileInfo fi = dirModel->fileInfo(mi);
    if (fi.isDir()) {
        lastDirPath = currentDir.path();
        if (backDirList.count() > 9) {
            backDirList.pop_front();
        }
        forwardDirList.clear();
        backDirList.append(lastDirPath);
        //dirModel->setRootPath(fi.absoluteFilePath());
        listView->setRootIndex(mi);
        detailView->setRootIndex(mi);
        currentDir = QDir(fi.absoluteFilePath());
        directoryCB->clear();
        directoryCB->addItem(currentDir.path());
        addParentDir(currentDir);
        int index = directoryCB->findText(currentDir.path());
        if (directoryCB->findText(lastDirPath) < 0)
            directoryCB->addItem(lastDirPath);
        directoryCB->setCurrentIndex(index);
        detailViewSelectionModel->clearSelection();
        listViewSelectionModel->clearSelection();
    }
    else
        currentFile = fi.absoluteFilePath();
    validate();
}
void EmbeddedFileSelector::goBackForwardSlot(int buttonID)
{
    int numItems;
    QString path;
    int index;
    if (buttonID == DirectoryBack) {
        if (backDirList.count() < 1) {
            validate();
            return;
        }
        if (forwardDirList.count() > 9)
            forwardDirList.removeFirst();
        forwardDirList.append(currentDir.path());
        numItems = backDirList.count();
        path = backDirList.last();
        backDirList.removeAt(numItems-1);
    }
    else { // must be forward dir
        if (forwardDirList.count() < 1) {
            validate();
            return;
        }
        if (backDirList.count() > 9)
            backDirList.removeFirst();
        backDirList.append(currentDir.path());
        numItems = forwardDirList.count();
        path = forwardDirList.last();
        forwardDirList.removeAt(numItems-1);
    }
    detailViewSelectionModel->clearSelection();
    listViewSelectionModel->clearSelection();
    currentDir.setPath(path);
    directoryCB->clear();
    directoryCB->addItem(currentDir.path());
    addParentDir(currentDir);
    index = directoryCB->findText(path);
    if (directoryCB->findText(lastDirPath) < 0)
        directoryCB->addItem(lastDirPath);
    directoryCB->setCurrentIndex(index);
    QModelIndex rootIndex = dirModel->setRootPath(path);
    listView->setRootIndex(rootIndex);
    detailView->setRootIndex(rootIndex);
    detailViewSelectionModel->clear();
    listViewSelectionModel->clear();
    validate();
}
void EmbeddedFileSelector::validate()
{
    QList <QModelIndex> selectedIndexes;
    QModelIndex selectedIndex;
    bool isDir = false;
    bool fileSelected = false;
    if (backDirList.count() < 1)
        backB->setEnabled(false);
    else
        backB->setEnabled(true);
    if (forwardDirList.count() < 1)
        forwardB->setEnabled(false);
    else
        forwardB->setEnabled(true);
    bool listViewHasSelected = false;
    bool detailViewHasSelected = false;

    if (listViewSelectionModel->hasSelection()) {
        selectedIndexes = listViewSelectionModel->selectedIndexes();
        selectedIndex = selectedIndexes.at(0);
        if (selectedIndex.isValid()) {
            isDir = dirModel->isDir(selectedIndex);
            qDebug() << "IS DIR = " << isDir;
            if (!isDir)
                listViewHasSelected = true;
        }
    }
    if (!listViewHasSelected) {
        if (detailViewSelectionModel->hasSelection()) {
            selectedIndexes = detailViewSelectionModel->selectedIndexes();
            selectedIndex = selectedIndexes.at(0);
            if (selectedIndex.isValid()) {
                isDir = dirModel->isDir(selectedIndex);
                if (!isDir)
                    detailViewHasSelected = true;
            }
        }
    }
    if (detailViewHasSelected || listViewHasSelected)
        fileSelected = true;
    else
        currentFile = QString::null;
    emit haveFileSelected(fileSelected);
}
void EmbeddedFileSelector::quickSelectViewSlot(QModelIndex mi)
{
    int index;
    QStandardItem *item = quickSelectModel->itemFromIndex(mi);
    if (!item) {
        qWarning() << "Error - quick select item is null" << __FILE__ << __LINE__;
        return;
    }
    lastDirPath = currentDir.path();
    if (item == quickHomeItem)  {
        if (currentDir == QDir::home())
            return;
        currentDir = QDir::home();

    }
    else if (item == quickRootItem) {
        if (currentDir == QDir::root())
            return;
        currentDir = QDir::root();
    }
    detailViewSelectionModel->clearSelection();
    listViewSelectionModel->clearSelection();
    directoryCB->clear();
    directoryCB->addItem(currentDir.path());
    addParentDir(currentDir);
    index = directoryCB->findText(currentDir.path());
    if (directoryCB->findText(lastDirPath) < 0)
        directoryCB->addItem(lastDirPath);
    directoryCB->setCurrentIndex(index);
    QModelIndex rootIndex = dirModel->setRootPath(currentDir.path());
    listView->setRootIndex(rootIndex);
    detailView->setRootIndex(rootIndex);
    forwardDirList.clear();
    if (backDirList.count() > 9)
        backDirList.removeFirst();
    backDirList.append(currentDir.path());

    validate();
}
bool EmbeddedFileSelector::isFileSelected()
{
    QList <QModelIndex> selectedIndexes;
    QModelIndex selectedIndex;
    bool isDir = false;
    bool listViewHasSelected = false;
    bool detailViewHasSelected = false;
    bool haveFile = false;
    if (listViewSelectionModel->hasSelection()) {
        selectedIndexes = listViewSelectionModel->selectedIndexes();
        selectedIndex = selectedIndexes.at(0);
        if (selectedIndex.isValid()) {
            isDir = dirModel->isDir(selectedIndex);
            if (!isDir)
                listViewHasSelected = true;
        }
    }
    if (!listViewHasSelected) {
        if (detailViewSelectionModel->hasSelection()) {
            selectedIndexes = detailViewSelectionModel->selectedIndexes();
            selectedIndex = selectedIndexes.at(0);
            if (selectedIndex.isValid()) {
                isDir = dirModel->isDir(selectedIndex);
                if (!isDir)
                    detailViewHasSelected = true;
            }
        }
    }
    if (detailViewHasSelected || listViewHasSelected)
        haveFile = true;
    return haveFile;
}
QString EmbeddedFileSelector::getSelectedFile()
{
    return currentFile;
}
