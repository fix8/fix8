#ifndef NEWWINDOWFILEPAGE_H
#define NEWWINDOWFILEPAGE_H

#include <QWizardPage>
#include "embeddedfileselector.h"
class NewWindowFilePage : public QWizardPage
{
    Q_OBJECT
    friend class NewWindowWizard;
public:
    explicit NewWindowFilePage(QWidget *parent = 0);
    virtual bool isComplete() const;
    void readSettings();
    void saveSettings();
    QString getSelectedFile();
protected slots:
    void fileSelectedSlot(bool);
protected:
    EmbeddedFileSelector *fileSelector;
};
#endif // NEWWINDOWFILEPAGE_H
