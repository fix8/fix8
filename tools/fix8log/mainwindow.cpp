#include "fixmimedata.h"
#include "fixtoolbar.h"
#include "mainwindow.h"
#include "nodatalabel.h"
#include "worksheet.h"
#include "globals.h"
#include "searchlineedit.h"
#include "tableschema.h"
#include <QQuickView>
#include <QtWidgets>
#include <QStandardItemModel>

TableSchema *MainWindow::defaultTableSchema = 0;
TableSchemaList *MainWindow::schemaList = 0;

MainWindow::MainWindow(bool showLoading)
    : QMainWindow(0),fileDialog(0),qmlObject(0),windowDataID(-1),
      loadingActive(showLoading),tableSchema(0)
{
    buildMainWindow();
    if (loadingActive) {
        stackW->setCurrentIndex(ShowProgress);
    }
    copyTabA->setEnabled(false); // not enabled when no intital tab
    readSettings();
}

MainWindow::MainWindow(const MainWindow &mw,bool copyAll)
    : QMainWindow(0),fileDialog(0),qmlObject(0),windowDataID(-1),
      loadingActive(false)
{
    buildMainWindow();
    setAcceptDrops(true);
    QRect rect = mw.geometry();
    setGeometry(rect);
    int x = mw.x();
    int y = mw.y();
    restoreState(mw.saveState());
    if (copyAll) {
        for (int i=0;i<mw.tabW->count();i++) {
            WorkSheet *oldWorkSheet = qobject_cast <WorkSheet *> (mw.tabW->widget(i));
            QByteArray ba = oldWorkSheet->splitter->saveState();
            WorkSheet *newWorkSheet = new WorkSheet(*oldWorkSheet,this);
            newWorkSheet->setWindowID(uuid);
            connect(newWorkSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
                    this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
            connect(newWorkSheet,SIGNAL(modelDropped(FixMimeData*)),
                    this,SLOT(modelDroppedSlot(FixMimeData*)));
            newWorkSheet->splitter->restoreState(ba);
            tabW->addTab(newWorkSheet,mw.tabW->tabText(i));
        }
        if (tabW->count() > 0)
            stackW->setCurrentWidget(workAreaSplitter);
        else
            stackW->setCurrentWidget(noDataL);
    }
    readSettings();
    move(x+100,y+90); // offset from window copied
}
void MainWindow::setLoading(bool bstatus)
{
    loadingActive = bstatus;
    if (bstatus) {
        stackW->setCurrentIndex(ShowProgress);
     }
    else {
        if (tabW->count() > 0) {
            stackW->setCurrentIndex(ShowTab);
        }
        else {
            stackW->setCurrentIndex(ShowNoDataLabel);
        }
    }
}
void MainWindow::setLoadMessage(QString str)
{
    QVariant returnedValue;
    if (!qmlObject) {
        qWarning() << "Failed to set load message, qmlObject = 0"
                   << __FILE__ << __LINE__;
        return;
    }
    QMetaObject::invokeMethod (qmlObject, "setMessage", Q_RETURN_ARG(QVariant, returnedValue),  Q_ARG(QVariant,str));
}

void MainWindow::buildMainWindow()
{
    setAcceptDrops(true);
    uuid = QUuid::createUuid();
    setAnimated(true);
    mainMenuBar = menuBar();
    fileMenu = mainMenuBar->addMenu(tr("&File"));
    optionMenu = mainMenuBar->addMenu(tr("&Option"));
    schemaMenu = mainMenuBar->addMenu("&Schema");

    mainToolBar = new QToolBar("Main Toolbar",this);

    mainToolBar->setObjectName("MainToolBar");
    searchToolBar = new FixToolBar("Search Toolbar",this);
    connect(searchToolBar,SIGNAL(orientationChanged(Qt::Orientation)),
            this,SLOT(toolbarOrientationChangedSlot(Qt::Orientation)));
    searchToolBar->setObjectName("SearchToolBar");
    hideToolBarA = mainToolBar->toggleViewAction();
    hideSearchToolBarA = searchToolBar->toggleViewAction();
    mainToolBar->setMovable(true);
    searchToolBar->setMovable(true);
    addToolBar(Qt::TopToolBarArea,mainToolBar);
    addToolBar(Qt::TopToolBarArea,searchToolBar);

    autoSaveA = new  QAction(tr("&Auto Save"),this);
    QIcon autoIcon;
    autoIcon.addPixmap(QPixmap(":/images/svg/saveOn.svg"),QIcon::Normal,QIcon::On);
    autoIcon.addPixmap(QPixmap(":/images/svg/saveOff.svg"),QIcon::Normal,QIcon::Off);
    autoSaveA->setIcon(autoIcon);
    autoSaveA->setToolTip(tr("Automatically Save Session For Next Use"));
    autoSaveA->setCheckable(true);
    connect(autoSaveA,SIGNAL(triggered(bool)),this,SLOT(autoSaveOnSlot(bool)));
    closeA = new QAction(tr("&Close Window"),this);
    closeA->setIcon(QIcon(":/images/32x32/application-exit.png"));
    closeA->setToolTip(tr("Close This Window"));
    connect(closeA,SIGNAL(triggered()),this,SLOT(closeSlot()));

    quitA = new QAction(tr("&Quit"),this);
    quitA->setIcon(QIcon(":/images/32x32/exit.svg"));
    quitA->setToolTip(tr("Exit Application"));
    connect(quitA,SIGNAL(triggered()),this,SLOT(quitSlot()));

    newTabA = new QAction(tr("New Tab"),this);
    newTabA->setIcon((QIcon(":/images/svg/newspreadsheet.svg")));
    newTabA->setToolTip(tr("Create A New Empty Tab"));

    copyTabA = new QAction(tr("Copy Tab"),this);
    copyTabA->setIcon((QIcon(":/images/svg/spreadsheetCopy.svg")));
    copyTabA->setToolTip(tr("Create New Tab From Current Tab"));
    connect(copyTabA,SIGNAL(triggered()),this,SLOT(copyTabSlot()));

    cutTabA = new QAction(tr("Cut Tab"),this);
    cutTabA->setIcon((QIcon(":/images/svg/spreadsheetCopy.svg")));
    cutTabA->setToolTip(tr("Cut Tab For Pasting To Other Window"));
    newWindowA = new QAction("New &Window",this);
    newWindowA->setIcon((QIcon(":/images/32x32/newwindow.svg")));
    newWindowA->setToolTip(tr("Open New Window"));

    windowNameA = new QAction("Window Name",this);
    windowNameA->setToolTip("Set Window Name");
    connect(windowNameA,SIGNAL(triggered()),this,SLOT(setWindowNameSlot()));

    copyWindowA = new QAction("&Copy Window",this);
    copyWindowA->setIcon((QIcon(":/images/32x32/copywindow.svg")));
    copyWindowA->setToolTip(tr("Copy Window"));
    connect(copyWindowA,SIGNAL(triggered()),this,SLOT(copyWindowSlot()));
    editSchemaA= new QAction("&Schema Editor",this);
    editSchemaA->setIconText("Edit");
    editSchemaA->setIcon((QIcon(":/images/svg/editSchema.svg")));
     connect(editSchemaA,SIGNAL(triggered()),this,SLOT(editSchemaSlot()));
    showMessageA = new QAction(tr("Show/Hide Msgs"),this);
    showMessageA->setToolTip(tr("Show/Hide Message Area"));
    showMessageA->setCheckable(true);
    connect(showMessageA,SIGNAL(triggered(bool)),this,SLOT(showMessageArea(bool)));
    QIcon showIcon;
    showIcon.addPixmap(QPixmap(":/images/svg/showMessageArea.svg"),QIcon::Normal,QIcon::Off);
    showIcon.addPixmap(QPixmap(":/images/svg/hideMessageArea.svg"),QIcon::Normal,QIcon::On);
    showMessageA->setIcon(showIcon);

    searchBackA  = new QAction(tr("Back"),this);
    searchBackA->setIcon((QIcon(":/images/svg/back.svg")));
    searchBeginA = new QAction(tr("Begining"),this);
    searchBeginA->setIcon((QIcon(":/images/svg/begining.svg")));
    searchEndA   = new QAction(tr("End"),this);
    searchEndA->setIcon((QIcon(":/images/svg/end.svg")));
    searchNextA  = new QAction(tr("Next"),this);
    searchNextA->setIcon((QIcon(":/images/svg/forward.svg")));
    searchEditA  = new QAction(tr("Edit"),this);
    searchEditA->setIcon(QIcon(":/images/svg/edittabname.svg"));
    searchLV = new QLabel(searchToolBar);// only show when toobar is vertial
    searchArea = new QWidget(this);
    QHBoxLayout *searchBox = new QHBoxLayout();
    searchBox->setMargin(0);
    searchArea->setLayout(searchBox);
    searchL = new QLabel(searchArea);
    searchL->setText(tr("Search:"));
    searchLineEdit = new SearchLineEdit(searchArea);
    searchBox->addWidget(searchL,0);
    searchBox->addWidget(searchLineEdit,1);
    searchToolBar->addWidget(searchLV);
    searchToolBar->addWidget(searchArea);
    searchToolBar->addAction(searchEditA);
    searchToolBar->addAction(searchBeginA);
    searchToolBar->addAction(searchBackA);
    searchToolBar->addAction(searchNextA);
    searchToolBar->addAction(searchEndA);
    QHBoxLayout *space = new QHBoxLayout();
    space->addStretch(1);
    space->setMargin(0);
    QWidget *spaceW = new QWidget();
    spaceW->setLayout(space);

    searchArea->setLayout(searchBox);
    searchToolBar->addWidget(spaceW);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    consoleDock = new QDockWidget(tr("Console"),this);
    consoleDock->setObjectName("ConsoleDock");
    consoleArea = new QTextBrowser(consoleDock);
    addDockWidget(Qt::BottomDockWidgetArea,consoleDock);

    QPalette palette;
    QBrush brush(QColor(255, 170, 0, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
    QBrush brush1(QColor(252, 240, 228, 255));
    brush1.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::Text, brush1);
    QBrush brush2(QColor(0, 0, 0, 255));
    brush2.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::Base, brush2);
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Text, brush1);
    palette.setBrush(QPalette::Inactive, QPalette::Base, brush2);
    QBrush brush3(QColor(126, 125, 124, 255));
    brush3.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush3);
    palette.setBrush(QPalette::Disabled, QPalette::Text, brush3);
    QBrush brush4(QColor(255, 255, 255, 255));
    brush4.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Disabled, QPalette::Base, brush4);
    consoleArea->setPalette(palette);
    consoleArea->setMaximumHeight(340);
    consoleDock->setWidget(consoleArea);
    QFont fnt = consoleDock->font();
    fnt.setBold(true);
    consoleDock->setFont(fnt);
    consoleDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::TopDockWidgetArea);
    consoleDock->setFloating(false);

    hideColumMenu = new QMenu(this);
    hideColumMenu->setTitle(tr("Columns"));
    hideColumMenu->setTearOffEnabled(true);
    hideColActionGroup = new QActionGroup(this);
    hideColActionGroup->setExclusive(false);
    connect(hideColActionGroup,SIGNAL(triggered (QAction *)),this,SLOT(hideColumnActionSlot(QAction *)));

    configureIconsMenu = new QMenu(tr("Icons"),this);
    iconSizeMenu = new QMenu(tr("Size"),this);
    iconSizeSmallA = new QAction(tr("Small"),this);
    iconSizeRegA    = new QAction(tr("Regular"),this);
    iconSizeLargeA = new QAction(tr("Large"),this);
    iconSizeSmallA ->setCheckable(true);
    iconSizeRegA->setCheckable(true);
    iconSizeLargeA->setCheckable(true);
    iconSizeMenu->addAction(iconSizeSmallA);
    iconSizeMenu->addAction(iconSizeRegA);
    iconSizeMenu->addAction(iconSizeLargeA);
    iconSizeActionGroup = new QActionGroup(this);
    iconSizeActionGroup->setExclusive(true);
    connect(iconSizeActionGroup,SIGNAL(triggered (QAction *)),this,SLOT(iconSizeSlot(QAction *)));
    iconSizeActionGroup->addAction(iconSizeSmallA);
    iconSizeActionGroup->addAction(iconSizeRegA);
    iconSizeActionGroup->addAction(iconSizeLargeA);

    iconStyleMenu = new QMenu(tr("Style"),this);
    iconsOnlyA = new QAction(tr("Icons Only"),this);
    iconsWithTextA = new QAction(tr("Icons+Text"),this);
    iconsTextOnlyA = new QAction (tr("Text Only"),this);
    iconsOnlyA->setCheckable(true);
    iconsWithTextA->setCheckable(true);
    iconsWithTextA->setCheckable(true);
    iconStyleMenu->addAction(iconsOnlyA);
    iconStyleMenu->addAction(iconsWithTextA);
    iconStyleMenu->addAction(iconsTextOnlyA);
    iconsStyleGroup = new QActionGroup(this);
    connect(iconsStyleGroup,SIGNAL(triggered (QAction *)),this,SLOT(iconStyleSlot(QAction *)));
    iconsStyleGroup->setExclusive(true);
    iconsStyleGroup->addAction(iconsOnlyA);
    iconsStyleGroup->addAction(iconsWithTextA);
    iconsStyleGroup->addAction(iconsTextOnlyA);

    hideConsoleA = consoleDock->toggleViewAction();
    configureIconsMenu->addMenu(iconSizeMenu);
    configureIconsMenu->addMenu(iconStyleMenu);
    optionMenu->addAction(hideToolBarA);
    optionMenu->addAction(hideSearchToolBarA);
    optionMenu->addAction(hideConsoleA);
    optionMenu->addAction(editSchemaA);
    optionMenu->addAction(showMessageA);
    optionMenu->setTearOffEnabled(true);
    optionMenu->addMenu(hideColumMenu);
    optionMenu->addMenu(configureIconsMenu);


    fileMenu->addAction(windowNameA);
    fileMenu->addAction(autoSaveA);
    fileMenu->addAction(copyTabA);
    fileMenu->addAction(newTabA);
    fileMenu->addAction(copyWindowA);
    fileMenu->addAction(newWindowA);
    fileMenu->addAction(closeA);
    fileMenu->addSeparator();
    fileMenu->addAction(quitA);
    schemaMenu->addAction(editSchemaA);
    mainToolBar->addAction(closeA);
    mainToolBar->addAction(newWindowA);
    mainToolBar->addAction(copyWindowA);
    mainToolBar->addAction(newTabA);
    mainToolBar->addAction(copyTabA);
    mainToolBar->addAction(showMessageA);
    mainToolBar->addAction(autoSaveA);
    mainToolBar->addSeparator();
    mainToolBar->addAction(editSchemaA);


    configPB = new QPushButton(this);
    configPB->setIcon(QIcon(":/images/svg/preferences-color.svg"));
    configPB->setToolTip(tr("Set Menubar Color"));
    configPB->setFlat(true);
    menuBar()->setCornerWidget(configPB);
    QWidget *schemaArea = new QWidget(this);
    QHBoxLayout *schemaBox = new QHBoxLayout(schemaArea);
    schemaBox->setMargin(0);
    schemaBox->setSpacing(0);
    schemaArea->setLayout(schemaBox);
    schemaL = new QLabel("Schema: ",this);
    schemaL->setToolTip("Current Schema");
    schemaV = new QLabel("Default",this);
    schemaBox->addWidget(schemaL);
    schemaBox->addWidget(schemaV);
    schemaV->setToolTip("Current Schema");
    statusBar()->addWidget(schemaArea);
    connect(configPB,SIGNAL(clicked()),this,SLOT(configSlot()));
    // restore should be in settings but must come after
    stackW = new QStackedWidget(this);
    setCentralWidget(stackW);
    noDataL = new NoDataLabel("No Data Files\nLoaded");
    connect(noDataL,SIGNAL(modelDropped(FixMimeData*)),this,SLOT(modelDroppedSlot(FixMimeData*)));
    fnt = noDataL->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize() + 4);
    noDataL->setFont(fnt);
    noDataL->setAutoFillBackground(true);
    QPalette pal = noDataL->palette();
    pal.setColor(QPalette::Window,Qt::black);
    pal.setColor(QPalette::WindowText,Qt::white);
    noDataL->setPalette(pal);
    noDataL->setObjectName("No_Data_Label");
    noDataL->setAlignment(Qt::AlignCenter);
    progressView = new QQuickView(QUrl("qrc:qml/loadProgress.qml"));
    qmlObject = progressView->rootObject();
    if (!qmlObject) {
        qWarning() << "qml root object not found" << __FILE__ << __LINE__ ;
    }
    else
        connect(qmlObject,SIGNAL(cancel()),this,SLOT(cancelSessionRestoreSlot()));
    progressView->setResizeMode(QQuickView::SizeRootObjectToView);
    progressWidget = QWidget::createWindowContainer(progressView,this);

    workAreaSplitter = new QSplitter(Qt::Horizontal,this);
    tabW = new QTabWidget(workAreaSplitter);
    workAreaSplitter->addWidget(tabW);

    connect(tabW,SIGNAL(tabCloseRequested(int)),this,SLOT(tabCloseRequestSlot(int)));
    connect(tabW,SIGNAL(currentChanged(int)),this,SLOT(tabCurentChangedSlot(int)));

    // build tabname edit area
    tabNameEditArea = new QWidget(this);
    QHBoxLayout *tabNameBox = new QHBoxLayout(tabNameEditArea);
    tabNameBox->setMargin(0);
    tabNameBox->setSpacing(2);
    tabNameEditArea->setLayout(tabNameBox);
    fnt = tabNameEditArea->font();
    fnt.setPointSize(fnt.pointSize()-1);
    tabNameEditArea->setFont(fnt);
    cancelEditTabNamePB = new QPushButton(this);
    connect(cancelEditTabNamePB,SIGNAL(clicked()),this,SLOT(cancelTabNameSlot()));
    cancelEditTabNamePB->setIcon(QIcon(":/images/svg/cancel.svg"));
    tabNameLineEdit = new QLineEdit(this);
    tabNameLineEdit->setMaxLength(120);
    QRegExp regExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]+$");
    QValidator *val = new QRegExpValidator(regExp,this);
    tabNameLineEdit->setValidator(val);
    editTabNamePB = new QPushButton(this);
    connect(editTabNamePB,SIGNAL(clicked(bool)),this,SLOT(editTabNameSlot(bool)));
    editTabNamePB->setCheckable(true);
    editTabNamePB->setFlat(true);
    editTabNamePB->setToolTip(tr("Edit name of current tab"));
    QIcon editTabIcon;
    editTabIcon.addPixmap(QPixmap(":/images/svg/checkmark.svg"),QIcon::Normal,QIcon::On);
    editTabIcon.addPixmap(QPixmap(":/images/svg/edittabname.svg"),QIcon::Normal,QIcon::Off);
    editTabNamePB->setIcon(editTabIcon);
    tabNameBox->addWidget(tabNameLineEdit,1);
    tabNameBox->addWidget(cancelEditTabNamePB,0,Qt::AlignRight);
    tabNameBox->addWidget(editTabNamePB,0);
    cancelEditTabNamePB->hide();

    tabNameLineEdit->hide();
    cancelEditTabNamePB->setToolTip("Cancel edit of current tab name");
    editTabNamePB->setToolTip("Edit current tab name");
    connect(tabNameLineEdit,SIGNAL(textChanged(QString)),this,SLOT(tabNameModifiedSlot(QString)));
    connect(tabNameLineEdit,SIGNAL(returnPressed()),this,SLOT(tabNameReturnKeySlot()));

    tabW->setCornerWidget(tabNameEditArea);
    tabW->setTabPosition(QTabWidget::South);
    tabW->setTabsClosable(true);
    stackW->insertWidget(ShowNoDataLabel,noDataL);
    stackW->insertWidget(ShowTab,workAreaSplitter);
    stackW->insertWidget(ShowProgress,progressWidget);
    connect(newTabA,SIGNAL(triggered()),this,SLOT(createTabSlot()));
    connect(newWindowA,SIGNAL(triggered()),this,SLOT(createWindowSlot()));
    buildSchemaMenu();
    buildHideColumnMenu();
}
void MainWindow::buildSchemaMenu()
{

    schemaScopeGroup = new QActionGroup(this);
    schemaScopeGroup->setExclusive(true);
    schemaScopeMenu = schemaMenu->addMenu("Scope");
    schemaApplyAllA = new QAction("Apply Globally",this);
    schemaApplyWindowA = new QAction("Apply To Window",this);
    schemaApplyTabA = new QAction("Apply To Worksheet",this);
    schemaApplyAllA->setCheckable(true);
    schemaApplyWindowA->setCheckable(true);
    schemaApplyTabA->setCheckable(true);
    schemaScopeMenu->addAction(schemaApplyTabA);
    schemaScopeMenu->addAction(schemaApplyWindowA);
    schemaScopeMenu->addAction(schemaApplyAllA);

    schemaScopeGroup->addAction(schemaApplyTabA);
    schemaScopeGroup->addAction(schemaApplyWindowA);
    schemaScopeGroup->addAction(schemaApplyAllA);
    schemaMenu->addSeparator();
    TableSchema *tableSchema;
    if (!schemaList) {
        qDebug() << "Schema List is null " << __FILE__ << __LINE__;
        return;
    }
    qDebug() << "NUM of schema items = " << schemaList->count() << __FILE__ << __LINE__;
    schemaActionGroup = new QActionGroup(this);
    schemaActionGroup->setExclusive(true);

    QListIterator <TableSchema *> iter(*schemaList);
    while(iter.hasNext()) {
        tableSchema = iter.next();
        QAction *action = new QAction(tableSchema->name,this);
        action->setCheckable(true);
        QVariant var;
        var.setValue((void *) action);
        action->setData(var);
        schemaMenu->addAction(action);
        schemaActionGroup->addAction(action);

    }
}

MainWindow::~MainWindow()
{

}
void MainWindow::createWindowSlot()
{
    emit createWindow(this);
}
void MainWindow::showFileDialog()
{
    createTabSlot();
}

void MainWindow::buildHideColumnMenu()
{
    for (int i=0;i<FixTable::NumColumns;i++) {
        QAction *hideA = new QAction(tr("Hide ") + FixTable::headerLabel[i],this);
        hideA->setCheckable(true);
        hideA->setData(i);
        hideColumMenu->addAction(hideA);
        hideColActionGroup->addAction(hideA);
    }
}
const QUuid &MainWindow::getUuid()
{
    return uuid;
}

void MainWindow::showEvent(QShowEvent *se)
{
    if (!loadingActive)  {
        if (tabW->count() > 0) {
            stackW->setCurrentIndex(ShowTab);
            copyTabA->setEnabled(true);
            showMessageA->setEnabled(true);
        }
        else {
            stackW->setCurrentIndex(ShowNoDataLabel);
            copyTabA->setEnabled(false);
            showMessageA->setEnabled(false);
        }
    }
    QMainWindow::showEvent(se);
}
QSize MainWindow::sizeHint() const
{
    // TODO: figure out default size based on pixel density
    return QSize(800,900);
}
WindowData MainWindow::getWindowData()
{
    WindowData wd;
    wd.color = menubarColor;
    wd.geometry = this->saveGeometry();
    wd.state    = this->saveState();
    wd.id       = this->windowDataID;
    wd.isVisible = this->isVisible();
    wd.currentTab = tabW->currentIndex();
    wd.name     = name;
    return wd;
}
void MainWindow::setWindowData(const WindowData &wd)
{
    windowDataID = wd.id;
    restoreGeometry(wd.geometry);
    restoreState(wd.state);
    setColorSlot(wd.color);
    setVisible(wd.isVisible);
    setWindowTitle(wd.name);
    name = wd.name;
}
QList <WorkSheetData> MainWindow::getWorksheetData(int windowID)
{
    QList <WorkSheetData> wsdList;
    WorkSheetData  wsd;
    if (tabW->count() > 0) {
        for (int i=0; i < tabW->count();i++) {
            WorkSheet *ws =  qobject_cast <WorkSheet *> (tabW->widget(i));
            if (ws) {
                wsd  = ws->getWorksheetData();
                wsd.windowID = windowID;
                wsdList.append(wsd);
            }
        }
    }
    return wsdList;
}
WorkSheetData MainWindow::getWorksheetData(QUuid &workSheetID, bool *ok)
{
    WorkSheetData  wsd;
    *ok = false;
    if (tabW->count() > 0) {
        for (int i=0; i < tabW->count();i++) {
            WorkSheet *ws =  qobject_cast <WorkSheet *> (tabW->widget(i));
            if (ws && (ws->getID() == workSheetID)) {
                wsd  = ws->getWorksheetData();
                *ok = true;
                break;
            }
        }
    }
    return wsd;
}
void MainWindow::addWorkSheet(QStandardItemModel *model,WorkSheetData &wsd)
{
    int currentIndex = stackW->currentIndex();
    stackW->setCurrentIndex(ShowProgress);
    WorkSheet *newWorkSheet;
    if (!model) {
        qWarning() << "Failed to add work sheet, as model is null" <<  __FILE__ << __LINE__;
        stackW->setCurrentIndex(currentIndex);
        return;
    }
    newWorkSheet = new WorkSheet(model,wsd,this);
    newWorkSheet->setWindowID(uuid);
    connect(newWorkSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
    connect(newWorkSheet,SIGNAL(modelDropped(FixMimeData *)),
            this,SLOT(modelDroppedSlot(FixMimeData *)));
    QString str = wsd.fileName;
    if (wsd.tabAlias.length() > 0)
        str = wsd.tabAlias;
    int index = tabW->addTab(newWorkSheet,str);
    tabW->setToolTip(wsd.fileName);
    if (tabW->count() > 0) {
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
        if (tabW->count() > 1)
            tabW->setCurrentIndex(index);
    }
    stackW->setCurrentIndex(ShowTab);
}
void MainWindow::setAutoSaveOn(bool on)
{
    autoSaveA->setChecked(on);
}
void MainWindow::setCurrentTabAndSelectedRow(int currentTab, int currentRow)
{
    if ((tabW->count() < 1) || (tabW->count() < currentTab))
        return;
    tabW->setCurrentIndex(currentTab);
}
void MainWindow::popupMenuSlot(const QModelIndex &,const QPoint &)
{

}
void MainWindow::finishDrop(WorkSheetData &wsd, FixMimeData *fmd)
{
    qDebug() << "MainWindow::finishDrop" << __FILE__ << __LINE__;
    if (!fmd) {
        qWarning() << "Failed to finish drop, fmd = 0" << __FILE__ << __LINE__;
    }
    addWorkSheet(fmd->model,wsd);
}
void MainWindow::setTableSchema(TableSchema *ts)
{
    tableSchema = ts;
}
void MainWindow::addNewSchema(TableSchema *ts)
{
    qDebug() << "HERE IN MAIN WINDOW ADD NEW SCHEA" << __FILE__ << __LINE__;
    if (!ts)
        return;
    QAction *action = new QAction(ts->name,this);
    action->setCheckable(true);
    QVariant var;
    var.setValue((void *) action);
    action->setData(var);
    schemaMenu->addAction(action);
    schemaActionGroup->addAction(action);
}

TableSchema * MainWindow::getTableSchema()
{
    return tableSchema;
}
