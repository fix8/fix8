#ifndef NEWWINDOWWIZARD_H
#define NEWWINDOWWIZARD_H
#include "fix8sharedlib.h"
#include <QWizard>
class QDesktopWidget;
class QFile;
class QListView;
class QLabel;
class QStandardItemModel;
class QStackedLayout;
class EmbeddedFileSelector;
class NewWindowFilePage;
class NewWindowSchemaPage;

class NewWindowWizard : public QWizard
{
    Q_OBJECT
public:
    explicit NewWindowWizard(Fix8SharedLibList &shareLibs, QWidget *parent = 0);
    QString getSelectedFile();
    QString  getSelectedLib();
    void readSettings();
    void saveSettings();
public slots:
void currentPageChangedSlot(int pageID);
protected:
    void createSchemaPage();
    void createFilePage();
private:
 NewWindowSchemaPage *schemaPage;
 NewWindowFilePage *filePage;

 QDesktopWidget *desktopW;
 int filePageID;
 int noSchemasID;
 int schemasListID;
 Fix8SharedLibList &fix8SharedLibList;
 QString systemDirName;
 QString userDirName;
 QStringList schemaErrorStrList;
};
#endif // NEWWINDOWWIZARD_H
