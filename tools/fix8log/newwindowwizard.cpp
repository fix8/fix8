#include "newwindowwizard.h"
#include <QtWidgets>
#include "embeddedfileselector.h"
#include "newwindowschemapage.h"
#include "globals.h"
NewWindowWizard::NewWindowWizard(Fix8SharedLibList &shareLibs, QWidget *parent) :
    QWizard(parent),fix8SharedLibList(shareLibs)
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
    schemaPage = new NewWindowSchemaPage(fix8SharedLibList,this);

    schemaPage->loadSchemas(Fix8SharedLib::SystemLib);
    schemaPage->loadSchemas(Fix8SharedLib::UserLib);
    qDebug() << "NUM OF ITEMS " << schemaPage->schemaModel->rowCount() << __FILE__;
    if (schemaPage->schemaModel->rowCount() < 1) {
        schemaPage->schemaStack->setCurrentIndex(schemaPage->noSchemasID);
    }
    else {
        qDebug() << "RAISE SCHEMS LIST ID:" << schemaPage->schemasListID;
       schemaPage->schemaStack->setCurrentIndex(schemaPage->schemasListID);
    }
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
