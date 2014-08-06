#include "newwindowwizard.h"
#include <QtWidgets>
#include "embeddedfileselector.h"
#include "globals.h"
NewWindowWizard::NewWindowWizard(QWidget *parent) :
    QWizard(parent)
{
    setWindowTitle(GUI::Globals::appName + "New Window Wizard");
    setWindowIconText("New Window");
    setTitleFormat(Qt::RichText);
    setPixmap(QWizard::LogoPixmap,QPixmap(":/images/svg/newwindow.svg").scaledToHeight(48));
    desktopW  = qApp->desktop();
    int primaryScreenID = desktopW->primaryScreen();
    QRect rect = desktopW->screenGeometry(primaryScreenID);
    setMaximumHeight(rect.height()*0.75);
    setMaximumWidth(rect.width()*.66);
    createSchemaPage();
    createFilePage();
    readSettings();
    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(currentPageChangedSlot(int)));
}
void NewWindowWizard::createSchemaPage()
{
   schemaPage = new QWizardPage(this);
   schemaPage->setTitle("<h1>New Window Wizard</h1>");
   schemaPage->setSubTitle("<h2>Select FIX Schema</h2>");
   QGridLayout *schemaGrid= new QGridLayout(schemaPage);
   schemaPage->setLayout(schemaGrid);
   schemaStack = new QStackedLayout();
   schemaListView = new QListView(schemaPage);
   noSchemasFoundL = new QLabel(schemaPage);
   QString ss = "QLabel { color: rgb(255,255,255); border-color: rgba(255, 0, 0, 75%);  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #1a3994, stop: 1 #061a33); }";
   noSchemasFoundL->setStyleSheet(ss);
   noSchemasFoundL->setText("No Schemas\nFound");
   QFont fnt = noSchemasFoundL->font();
   fnt.setBold(true);
   fnt.setPointSize(fnt.pointSize()+2);
   noSchemasFoundL->setFont(fnt);
   noSchemasFoundL->setAlignment(Qt::AlignCenter);
   schemasListID = schemaStack->addWidget(schemaListView);
   noSchemasID = schemaStack->addWidget(noSchemasFoundL);
    schemaStack->setCurrentIndex(noSchemasID);
   QFontMetrics fm(schemaListView->font());
   schemaListView->setMaximumWidth(fm.averageCharWidth()*24);
   noSchemasFoundL->setMaximumWidth(fm.averageCharWidth()*24);

   schemaLabel = new QLabel(schemaPage);


   schemaLabel->setWordWrap(true);
   schemaLabel->setTextFormat(Qt::RichText);
   QString str = "<h1>Select Schema</h1>";
   str.append("Each window is associated with one FIX schema. These schema's (listed here) are loaded from two locations:");
   str.append("<p><ul><li>System Level- loaded from the  fix8logview install directory - <i>" + qApp->applicationDirPath() + "/fixschemas</i></li>");
   str.append("<li>User Level -loaded from user's subdirectory <i>" + QDir::homePath() + "/f8logview/schemas</i></li>");
   schemaLabel->setText(str);
   schemaGrid->addLayout(schemaStack,0,0);
   schemaGrid->addWidget(schemaLabel,0,1,Qt::AlignLeft);
   schemaGrid->setColumnStretch(0,0);
   schemaGrid->setColumnStretch(1,1);

   addPage(schemaPage);
}
void NewWindowWizard::createFilePage()
{
    filePage = new QWizardPage(this);
    filePage->setTitle("<h1>New Window Wizard</h1>");
    filePage->setSubTitle("<h2>Select Initial FIX Log File</h2>");
    QVBoxLayout *fileBox= new QVBoxLayout(filePage);
    filePage->setLayout(fileBox);
    fileSelector = new EmbeddedFileSelector(filePage);
    connect(fileSelector,SIGNAL(haveFileSelected(bool)),
            this,SLOT(fileSelectedSlot(bool)));
    //QLabel *fileLabel = new QLabel("FILE");
    fileBox->addWidget(fileSelector);
    QAbstractButton *finishedB = button(QWizard::FinishButton);
    finishedB->setToolTip("Select a valid file to enable this button");
    filePageID = addPage(filePage);
}
void NewWindowWizard::readSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    QRect defaultRect(00,100,600,500);
    QVariant defaultVar(defaultRect);
    setGeometry(settings.value("geometry",defaultRect).toRect());
    fileSelector->readSettings();
    QAbstractButton *finishedB = button(QWizard::FinishButton);
    finishedB->setEnabled(fileSelector->isFileSelected());
}
void NewWindowWizard::saveSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    settings.setValue("geometry",geometry());
    fileSelector->saveSettings();
}
void NewWindowWizard::fileSelectedSlot(bool haveFile)
{
    QAbstractButton *finishedB = button(QWizard::FinishButton);
    finishedB->setEnabled(haveFile);
}
void NewWindowWizard::currentPageChangedSlot(int id)
{
    QAbstractButton *finishedB;
    if (id == filePageID) {
         finishedB = button(QWizard::FinishButton);
        finishedB->setEnabled(fileSelector->isFileSelected());
    }
}
QString  NewWindowWizard::getSelectedFile()
{
   return fileSelector->getSelectedFile();
}