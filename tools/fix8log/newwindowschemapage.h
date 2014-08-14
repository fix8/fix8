#ifndef NEWWINDOWSCHEMAPAGE_H
#define NEWWINDOWSCHEMAPAGE_H

#include <QWizardPage>
#include <QtWidgets>
#include "fix8sharedlib.h"

class NewWindowSchemaPage : public QWizardPage
{
    Q_OBJECT
    friend class NewWindowWizard;
public:
    explicit NewWindowSchemaPage(Fix8SharedLibList &shareLibs,QWidget *parent = 0);
    virtual bool isComplete() const;
    bool loadSchemas(Fix8SharedLib::LibType);
protected slots:
  void schemaListViewSlot(QModelIndex);
protected:
    QStackedLayout *schemaStack;
    QLabel       *noSchemasFoundL;
    QLabel       *legend;
    QListView   *schemaListView;
    QStandardItemModel *schemaModel;
    QItemSelectionModel *selectionModel;

    QLabel      *schemaLabel;
    int filePageID;
    int noSchemasID;
    int schemasListID;
    Fix8SharedLibList &fix8SharedLibList;
    QString systemDirName;
    QString userDirName;
    QStringList schemaErrorStrList;
};

#endif // NEWWINDOWSCHEMAPAGE_H
