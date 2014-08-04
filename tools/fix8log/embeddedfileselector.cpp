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
    stackW = new QStackedWidget(splitter);
    listView = new QListView(stackW);
    listView->setFlow(QListView::LeftToRight);
    listView->setGridSize(QSize(94,44));
    listView->setUniformItemSizes(true);
    listView->setWrapping(true);
    listView->setViewMode(QListView::IconMode);
    listView->setResizeMode(QListView::Adjust);
    detailView = new QTableView(stackW);

    QHeaderView *verticalH = detailView->verticalHeader();
    verticalH->setVisible(false);
    detailView->setShowGrid(false);
    detailView->setSortingEnabled(true);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    detailView->setSelectionMode(QAbstractItemView::SingleSelection);
    detailView->setSelectionBehavior(QAbstractItemView::SelectRows);
    dirModel = new QFileSystemModel (this);
    dirModel->setFilter(QDir::Files| QDir::Dirs);
    listView->setModel(dirModel);
    detailView->setModel(dirModel);
    connect(listView,SIGNAL(clicked(QModelIndex)),this,SLOT(viewClickedSlot(QModelIndex)));
    connect(detailView,SIGNAL(clicked(QModelIndex)),this,SLOT(viewClickedSlot(QModelIndex)));
    stackW->insertWidget(ListView,listView);
    stackW->insertWidget(DetailView,detailView);
    splitter->addWidget(quickSelectView);
    splitter->addWidget(stackW);
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
}
void EmbeddedFileSelector::readSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    qDebug() << "EMBED FILE SEL READ SETTINGS" << __FILE__ << __LINE__;
    QByteArray ba =  settings.value("splitter").toByteArray();
    splitter->restoreState(ba);
    currentDir.setPath(settings.value("currentDir",QDir::homePath()).toString());
    directoryCB->addItem(currentDir.path());
    stackW->setCurrentIndex(settings.value("viewmode",ListView).toInt());
    QModelIndex rootIndex = dirModel->setRootPath(currentDir.path());
    listView->setRootIndex(rootIndex);
    detailView->setRootIndex(rootIndex);
     addParentDir(currentDir);
}
void EmbeddedFileSelector::addParentDir(QDir dir)
{
    if (dir == QDir::root())
        return;
    dir.cdUp();
    qDebug() << "Adding parent: " << dir.path() << __FILE__ << __LINE__;
    directoryCB->addItem(dir.path());
    addParentDir(dir);
}
void EmbeddedFileSelector::directoryCBSlot(QString path)
{
    qDebug() << "Set Directory";
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
    qDebug() << "Set View Mode.." << __FILE__ << __LINE__;
    stackW->setCurrentIndex(viewMode);

}
void EmbeddedFileSelector::viewClickedSlot(QModelIndex mi)
{
    QFileInfo fi = dirModel->fileInfo(mi);
    qDebug() << "Clicked: PATH = " << fi.absoluteFilePath() << __FILE__ << __LINE__;
    if (fi.isDir()) {
        lastDirPath = currentDir.path();
        if (backDirList.count() > 9) {
            backDirList.pop_front();
        }
        backDirList.append(lastDirPath);
        //dirModel->setRootPath(fi.absoluteFilePath());
        listView->setRootIndex(mi);
        detailView->setRootIndex(mi);
        currentDir = QDir(fi.absoluteFilePath());
        qDebug() << "\tCurrent Dir Path = " << currentDir.path();
        directoryCB->clear();
        directoryCB->addItem(currentDir.path());
        addParentDir(currentDir);
        int index = directoryCB->findText(currentDir.path());
        if (directoryCB->findText(lastDirPath) < 0)
            directoryCB->addItem(lastDirPath);
        directoryCB->setCurrentIndex(index);

    }

}
void EmbeddedFileSelector::goBackForwardSlot(int buttonID)
{
    qDebug() << "GO BACK/FORWARD" << __FILE__ << __LINE__;
    if (buttonID == DirectoryBack) {
        if (backDirList.count() < 1) {
            qDebug() << "BACK COUNT < 1" << __FILE__ << __LINE__;
            // validate()
            return;
        }

        int numItems = backDirList.count();
        QString path = backDirList.last();
        qDebug() << "NUM OF ITEMS NOW: "  << numItems;
        backDirList.removeAt(numItems-1);
        qDebug() << "GO BACK TO:" << path;
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
    //validate();
}
