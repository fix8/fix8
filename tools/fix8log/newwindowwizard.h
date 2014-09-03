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
class WelcomePage;
class NewWindowWizard : public QWizard
{
    Q_OBJECT
public:
    explicit NewWindowWizard(Fix8SharedLibList &shareLibs, bool isFirstTime = false,QWidget *parent = 0);
    QString getSelectedFile();
    QString  getSelectedLib();
    void readSettings();
    void saveSettings();
    virtual QSize	sizeHint() const;

public slots:
void currentPageChangedSlot(int pageID);
protected:
    void createWelcomePage();
    void createSchemaPage();
    void createFilePage();
private:
 WelcomePage *welcomePage;
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
 bool firstTime;
};
#endif // NEWWINDOWWIZARD_H
