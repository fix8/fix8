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
    setMaximumHeight(rect.height()*0.66);
    setMaximumWidth(rect.width()*.5);
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

   schemaListView = new QListView(schemaPage);
   QFontMetrics fm(schemaListView->font());
   schemaListView->setMaximumWidth(fm.averageCharWidth()*24);
   schemaLabel = new QLabel(schemaPage);
   schemaLabel->setWordWrap(true);
   schemaLabel->setTextFormat(Qt::RichText);
   QString str = "<h1>Select Schema</h1>";
   str.append("Each window is associated with one FIX schema. These schema's (list here) are loaded from two locations:");
   str.append("<p><ul><li>System Level- loaded from the  fix8logview install directory - <i>" + qApp->applicationDirPath() + "/fixschemas</i></li>");
   str.append("<li>User Level -loaded from user's subdirectory <i>" + QDir::homePath() + "/f8logview/schemas</i></li>");
   schemaLabel->setText(str);
   QFont fnt = schemaLabel->font();
   fnt.setPointSize(fnt.pointSize()+3);
   fnt.setBold(true);
   fnt.setItalic(true);
   schemaLabel->setFont(fnt);
   schemaGrid->addWidget(schemaListView,0,0);
   schemaGrid->addWidget(schemaLabel,0,1);
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
    setGeometry(settings.value("geometry").toRect());
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
