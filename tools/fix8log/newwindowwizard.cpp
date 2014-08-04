#include "newwindowwizard.h"
#include <QtWidgets>
#include "embeddedfileselector.h"
NewWindowWizard::NewWindowWizard(QWidget *parent) :
    QWizard(parent)
{
    createSchemaPage();
    createFilePage();
    readSettings();

}
void NewWindowWizard::createSchemaPage()
{
   schemaPage = new QWizardPage(this);
   schemaPage->setTitle("Select Fix Schema");
   QGridLayout *schemaGrid= new QGridLayout(schemaPage);
   schemaPage->setLayout(schemaGrid);

   schemaListView = new QListView(schemaPage);
   QFontMetrics fm(schemaListView->font());
   schemaListView->setMaximumWidth(fm.averageCharWidth()*24);
   schemaLabel = new QLabel(schemaPage);
   schemaLabel->setWordWrap(true);
   schemaLabel->setTextFormat(Qt::RichText);
   QString str = "Each window is assoicated with a FIX schema. These schema's are found in two places:";
   str.append("<p><ul><li>System Level- loaded from the  fix8logview install directory.</li>");
   str.append("<li>User Level -loaded from user's subdirectory <i>$HOME/f8logview/schemas<i></li>");
   schemaLabel->setText(str);
   QFont fnt = schemaLabel->font();
   fnt.setPointSize(fnt.pointSize()+3);
   fnt.setBold(true);
   fnt.setItalic(true);
   schemaLabel->setFont(fnt);
   schemaGrid->addWidget(schemaListView,0,0);
   schemaGrid->addWidget(schemaLabel,0,1,Qt::AlignCenter);
   addPage(schemaPage);
}
void NewWindowWizard::createFilePage()
{
    filePage = new QWizardPage(this);
    filePage->setTitle("Select Initial Log File");

    QVBoxLayout *fileBox= new QVBoxLayout(filePage);
    filePage->setLayout(fileBox);
    fileSelector = new EmbeddedFileSelector(filePage);
    //QLabel *fileLabel = new QLabel("FILE");
    fileBox->addWidget(fileSelector);
    addPage(filePage);
}
void NewWindowWizard::readSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    setGeometry(settings.value("geometry").toRect());
    fileSelector->readSettings();
}
void NewWindowWizard::saveSettings()
{
    QSettings settings("fix8","logviewerNewWindowWizard");
    settings.setValue("geometry",geometry());
    fileSelector->saveSettings();
}
