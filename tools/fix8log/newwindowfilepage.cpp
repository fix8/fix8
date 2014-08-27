#include "newwindowfilepage.h"
#include <QAbstractButton>
#include <QSettings>
NewWindowFilePage::NewWindowFilePage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle("<h1>New Window Wizard</h1>");
    setSubTitle("<h2>Select Initial FIX Log File</h2>");
    QVBoxLayout *fileBox= new QVBoxLayout(this);
    setLayout(fileBox);
    fileSelector = new EmbeddedFileSelector(this);

    connect(fileSelector,SIGNAL(haveFileSelected(bool)),
            this,SLOT(fileSelectedSlot(bool)));
    //QLabel *fileLabel = new QLabel("FILE");
    fileBox->addWidget(fileSelector);

}
void NewWindowFilePage::fileSelectedSlot(bool)
{
    emit completeChanged();

}
bool NewWindowFilePage::isComplete() const
{
    return fileSelector->isFileSelected();
}
void NewWindowFilePage::readSettings()
{
    QSettings settings("fix8","logviewerNewWindowFilePage");
    QRect defaultRect(0,100,900,700);
    QVariant defaultVar(defaultRect);
    setGeometry(settings.value("geometry",defaultRect).toRect());
    fileSelector->readSettings();
}
void NewWindowFilePage::saveSettings()
{
    QSettings settings("fix8","logviewerNewWindowFilePage");
    settings.setValue("geometry",geometry());
    fileSelector->saveSettings();
}
QString NewWindowFilePage::getSelectedFile()
{
   return fileSelector->getSelectedFile();
}
