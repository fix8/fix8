#include "mainwindow.h"
#include "worksheet.h"
#include "globals.h"
#include <QtWidgets>
#include <QStandardItemModel>

MainWindow::MainWindow()
    : QMainWindow(0),fileDialog(0)
{
    buildMainWindow();
    copyTabA->setEnabled(false); // not enabled when no intital tab
    readSettings();
}

MainWindow::MainWindow(const MainWindow &mw,bool copyAll)
    : QMainWindow(0),fileDialog(0)
{
    buildMainWindow();
    QRect rect = mw.geometry();
    setGeometry(rect);
    int x = mw.x();
    int y = mw.y();
    restoreState(mw.saveState());
    if (copyAll) {
        for (int i=0;i<mw.tabW->count();i++) {
            WorkSheet *oldWorkSheet = qobject_cast <WorkSheet *> (mw.tabW->widget(i));
            WorkSheet *newWorkSheet = new WorkSheet(*oldWorkSheet,this);
            tabW->addTab(newWorkSheet,mw.tabW->tabText(i));

        }
        if (tabW->count() > 0)
            stackW->setCurrentWidget(workAreaSplitter);
        else
            stackW->setCurrentWidget(noDataL);
    }

    readSettings();
    move(x+100,y+90);

}
void MainWindow::buildMainWindow()
{
    mainMenuBar = menuBar();
    fileMenu = mainMenuBar->addMenu(tr("&File"));
    optionMenu = mainMenuBar->addMenu(tr("&Option"));
    mainToolBar = new QToolBar("Tool Bar",this);
    mainToolBar->setObjectName("MainToolBar");
    hideToolBarA = mainToolBar->toggleViewAction();
    mainToolBar->setMovable(true);
    addToolBar(Qt::TopToolBarArea,mainToolBar);

    closeA = new QAction(tr("&Close Window"),this);
    closeA->setIcon(QIcon(":/images/32x32/application-exit.png"));
    closeA->setToolTip(tr("Close This Window"));

    quitA = new QAction(tr("&Quit"),this);
    connect(quitA,SIGNAL(triggered()),this,SLOT(quitSlot()));
    quitA->setIcon(QIcon(":/images/32x32/exit.svg"));
    quitA->setToolTip(tr("Exit Application"));

    newTabA = new QAction(tr("New Tab"),this);
    newTabA->setIcon((QIcon(":/images/svg/newspreadsheet.svg")));
    newTabA->setToolTip(tr("Create A New Empty Tab"));

    copyTabA = new QAction(tr("Copy Tab"),this);
    connect(copyTabA,SIGNAL(triggered()),this,SLOT(copyTabSlot()));
    copyTabA->setIcon((QIcon(":/images/svg/spreadsheetCopy.svg")));
    copyTabA->setToolTip(tr("Create New Tab From Current Tab"));
    newWindowA = new QAction("New &Window",this);
    newWindowA->setIcon((QIcon(":/images/32x32/newwindow.svg")));
    newWindowA->setToolTip(tr("Open New Window"));
    copyWindowA = new QAction("&Copy Window",this);
    copyWindowA->setIcon((QIcon(":/images/32x32/copywindow.svg")));
    copyWindowA->setToolTip(tr("Copy Window"));
    showMessageA = new QAction(tr("Show/Hide Msgs"),this);
    connect(showMessageA,SIGNAL(triggered(bool)),this,SLOT(showMessageArea(bool)));
    showMessageA->setToolTip(tr("Show/Hide Message Area"));
    showMessageA->setCheckable(true);
    QIcon messageIcon;
    messageIcon.addPixmap(QPixmap(":/images/svg/showMessageArea.svg"),QIcon::Normal,QIcon::Off);
    messageIcon.addPixmap(QPixmap(":/images/svg/hideMessageArea.svg"),QIcon::Normal,QIcon::On);
    showMessageA->setIcon(messageIcon);

    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    connect(copyWindowA,SIGNAL(triggered()),this,SLOT(copyWindowSlot()));
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
    optionMenu->addAction(hideToolBarA);
    optionMenu->setTearOffEnabled(true);
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

    configureIconsMenu->addMenu(iconSizeMenu);
    configureIconsMenu->addMenu(iconStyleMenu);

    optionMenu->addMenu(hideColumMenu);
    optionMenu->addMenu(configureIconsMenu);
    optionMenu->addSeparator();
    optionMenu->addAction(showMessageA);
    connect(closeA,SIGNAL(triggered()),this,SLOT(closeSlot()));
    fileMenu->addAction(copyTabA);
    fileMenu->addAction(newTabA);
    fileMenu->addAction(copyWindowA);

    fileMenu->addAction(newWindowA);

    fileMenu->addAction(closeA);
    fileMenu->addSeparator();
    fileMenu->addAction(quitA);

    mainToolBar->addAction(closeA);
    mainToolBar->addAction(newWindowA);
    mainToolBar->addAction(copyWindowA);
    mainToolBar->addAction(newTabA);
    mainToolBar->addAction(copyTabA);
    mainToolBar->addSeparator();
    mainToolBar->addAction(showMessageA);

    configPB = new QPushButton(this);
    configPB->setIcon(QIcon(":/images/svg/preferences-color.svg"));
    configPB->setToolTip(tr("Set Menubar Color"));
    configPB->setFlat(true);
    menuBar()->setCornerWidget(configPB);
    connect(configPB,SIGNAL(clicked()),this,SLOT(configSlot()));

    // restore should be in settings but must come after

    hideConsoleA = consoleDock->toggleViewAction();
    optionMenu->addAction(hideConsoleA);


    stackW = new QStackedWidget(this);
    setCentralWidget(stackW);
    noDataL = new QLabel("No Data Files\nLoaded");
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
    workAreaSplitter = new QSplitter(Qt::Horizontal,this);
    tabW = new QTabWidget(workAreaSplitter);
    //QLabel *label = new QLabel("TBD\nMessage Area");
   // label->setAlignment(Qt::AlignCenter);
    workAreaSplitter->addWidget(tabW);
    //workAreaSplitter->addWidget(label);

    connect(tabW,SIGNAL(tabCloseRequested(int)),this,SLOT(tabCloseRequestSlot(int)));
    connect(tabW,SIGNAL(currentChanged(int)),this,SLOT(tabCurentChangedSlot(int)));

    // build tabnname edit area
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
    editTabNamePB = new QPushButton(this);
    connect(editTabNamePB,SIGNAL(clicked(bool)),this,SLOT(editTabNameSlot(bool)));
    editTabNamePB->setCheckable(true);
    editTabNamePB->setFlat(true);
    editTabNamePB->setToolTip(tr("Edit name of current tab"));
    QIcon icon;
    icon.addPixmap(QPixmap(":/images/svg/checkmark.svg"),QIcon::Normal,QIcon::On);
    icon.addPixmap(QPixmap(":/images/svg/edittabname.svg"),QIcon::Normal,QIcon::Off);
    editTabNamePB->setIcon(icon);
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
    connect(newTabA,SIGNAL(triggered()),this,SLOT(createTabSlot()));
    connect(newWindowA,SIGNAL(triggered()),this,SLOT(createWindowSlot()));
    connect(closeA,SIGNAL(triggered()),this,SLOT(closeSlot()));


    buildHideColumnMenu();

}

MainWindow::~MainWindow()
{

}

void MainWindow::createWindowSlot()
{
    emit createWindow(this);
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
void MainWindow::showEvent(QShowEvent *se)
{
    qDebug() << "\t1 Main Window Show Event" << __FILE__ << __LINE__;
    if (tabW->count() > 0) {
        stackW->setCurrentWidget(tabW);
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
    }
    else {
        stackW->setCurrentWidget(noDataL);
        copyTabA->setEnabled(false);
        showMessageA->setEnabled(false);
    }
    qDebug() << "\t2 Main Window Show Event" << __FILE__ << __LINE__;
    QMainWindow::showEvent(se);
}
QSize MainWindow::sizeHint() const
{
    return QSize(800,900);
}
