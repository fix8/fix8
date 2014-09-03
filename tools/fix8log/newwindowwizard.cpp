#include "newwindowwizard.h"
#include <QApplication>
#include <QtWidgets>
#include "embeddedfileselector.h"
#include "newwindowfilepage.h"
#include "newwindowschemapage.h"
#include "globals.h"
#include "welcomepage.h"

NewWindowWizard::NewWindowWizard(Fix8SharedLibList &shareLibs, bool FirstTime,QWidget *parent) :
    QWizard(parent),fix8SharedLibList(shareLibs),firstTime(FirstTime)
{
    int x,y;
    setWindowTitle(GUI::Globals::appName);
    setWindowIconText("New Window");
    setTitleFormat(Qt::RichText);
    setPixmap(QWizard::LogoPixmap,QPixmap(":/images/svg/newwindow.svg").scaledToHeight(48));
    desktopW  = qApp->desktop();
    int primaryScreenID = desktopW->primaryScreen();
    QRect rect = desktopW->screenGeometry(primaryScreenID);
    setMaximumHeight(rect.height()*0.75);
    setMaximumWidth(rect.width()*.66);
    if (firstTime) {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect rect = desktop->screenGeometry(desktop->primaryScreen());
        x = (rect.width() -860)/2;
        y = (rect.height() - 640)/2;
        createWelcomePage();
    }
    createSchemaPage();
    createFilePage();
    readSettings();
    if (firstTime) {
        move(x,y);
        resize(QSize(860,640));
    }
    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(currentPageChangedSlot(int)));
}
void NewWindowWizard::createWelcomePage()
{
    QWizardPage *welcomeWizardPage= new QWizardPage(this);
    welcomeWizardPage->setTitle("<h3>Welcome</h3>");
    QGridLayout *welcomeGrid= new QGridLayout(welcomeWizardPage);
    welcomeWizardPage->setLayout(welcomeGrid);
    welcomeGrid->setRowStretch(0,1);
    welcomeGrid->setColumnStretch(0,1);

    welcomePage = new WelcomePage(welcomeWizardPage);
    welcomeGrid->addWidget(welcomePage);
    addPage(welcomeWizardPage);
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
    QSize sh = sizeHint();
    QRect defaultRect(0,100,sh.width(),sh.height());
    QVariant defaultVar(defaultRect);
    filePage->readSettings();

    QRect rect = settings.value("geometry",defaultRect).toRect();
    setGeometry(rect);
    resize(rect.width(),rect.height());
}
void NewWindowWizard::saveSettings()
{
    filePage->saveSettings();

    QSettings settings("fix8","logviewerNewWindowWizard");
    settings.setValue("geometry",geometry());
}

void NewWindowWizard::currentPageChangedSlot(int id)
{

}
QString  NewWindowWizard::getSelectedFile()
{
    return filePage->getSelectedFile();
}
QString NewWindowWizard::getSelectedLib()
{
    return schemaPage->getSelectedLib();
}
QSize	NewWindowWizard::sizeHint() const
{
    QSize size;
    QDesktopWidget *desktop = QApplication::desktop();
    QRect rect = desktop->screenGeometry(desktop->primaryScreen());
    size.setWidth(860);
    size.setHeight(640);
    return size;
}
