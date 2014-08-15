#include "newwindowwizard.h"
#include <QtWidgets>
#include "embeddedfileselector.h"
#include "newwindowfilepage.h"
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

    filePage = new NewWindowFilePage(this);
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
    filePage->readSettings();

}
void NewWindowWizard::saveSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    settings.setValue("geometry",geometry());
    filePage->saveSettings();
}

void NewWindowWizard::currentPageChangedSlot(int id)
{

}
QString  NewWindowWizard::getSelectedFile()
{
    return filePage->getSelectedFile();
}
