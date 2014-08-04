#ifndef EMBEDDEDFILESELECTOR_H
#define EMBEDDEDFILESELECTOR_H
#include <QtWidgets>
#include <QWidget>

class EmbeddedFileSelector : public QWidget
{
    Q_OBJECT
public:
    explicit EmbeddedFileSelector(QWidget *parent = 0);
    typedef enum  {ListView,DetailView} ViewMode;
    enum {DirectoryBack,DirectoryForward};
    void saveSettings();
    void readSettings();
signals:

public slots:
    void directoryCBSlot(QString path);
    void goBackForwardSlot(int buttonID);
    void viewClickedSlot(QModelIndex );
    void viewModeSlot(int viewModeSelected);

protected:
    QLabel      *directoryL;
    QComboBox   *directoryCB;
    QButtonGroup *directoryBG;
    QToolButton *backB;
    QToolButton *forwardB;
    QButtonGroup *viewModeBG;
    QToolButton *listViewB;
    QToolButton *detailViewB;
    QListView   *quickSelectView;
    QStackedWidget *stackW;
    QListView   *listView;
    QTableView  *detailView;
    QSplitter   *splitter;
    QLabel      *filterTypeL;
    QComboBox   *filterTypeCB;
    QDir        currentDir;
    QString     lastDirPath;
    ViewMode    viewMode;
    QFileSystemModel    *dirModel;
    QStringList backDirList;
    QStringList forwardDirList;
private:
    void addParentDir(QDir dir);
    void validate();
};

#endif // EMBEDDEDFILESELECTOR_H
