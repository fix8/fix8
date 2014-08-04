#ifndef NEWWINDOWWIZARD_H
#define NEWWINDOWWIZARD_H

#include <QWizard>
class QDesktopWidget;
class QListView;
class QLabel;
class EmbeddedFileSelector;
class NewWindowWizard : public QWizard
{
    Q_OBJECT
public:
    explicit NewWindowWizard(QWidget *parent = 0);
    QString getSelectedFile();
    void readSettings();
    void saveSettings();
public slots:
void currentPageChangedSlot(int pageID);
void fileSelectedSlot(bool haveFile);
protected:
    void createSchemaPage();
    void createFilePage();
private:
 QWizardPage *schemaPage;
 QWizardPage *filePage;

 EmbeddedFileSelector *fileSelector;
 QListView   *schemaListView;
 QLabel      *schemaLabel;
 QDesktopWidget *desktopW;
 int filePageID;

};

#endif // NEWWINDOWWIZARD_H
