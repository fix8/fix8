#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "fixtable.h"
#include "globals.h"
#include "windowdata.h"

class QAction;
class QActionGroup;
class QComboBox;
class QDockWidget;
class QFileDialog;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QSplitter;
class QStackedWidget;
class QStandardItemModel;
class QTabWidget;
class QTextBrowser;
class QTogleButton;
class QToolBar;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class Fix8Log;
public:
    MainWindow();
    MainWindow(const MainWindow & sibling,bool copyAll = false);
    ~MainWindow();
    WindowData getWindowData();
    void setWindowData(const WindowData &wd);
    protected slots:
    void autoSaveOnSlot(bool);
    void cancelTabNameSlot();
    void closeSlot();
    void currentColorChangedSlot(QColor);
    void hideColumnActionSlot(QAction *);
    void createWindowSlot();
    void configSlot();
    void copyTabSlot();
    void copyWindowSlot();
    void createTabSlot();
    void editTabNameSlot(bool isOn);
    void fileDirChangedSlot(const QString &);
    void fileFilterSelectedSlot(QString);
    void fileSelectionFinishedSlot(int returnCode);
    void iconStyleSlot(QAction *);
    void iconSizeSlot(QAction *);
    void quitSlot();
    QSize sizeHint() const;
    void setColorSlot(QColor color);
    void showMessageArea(bool);
    void tabCloseRequestSlot(int);
    void tabCurentChangedSlot(int);
    void tabNameModifiedSlot(QString);
    void tabNameReturnKeySlot();
protected:
    enum {ShowNoDataLabel,ShowTab};
    QAction  *autoSaveA;
    QAction  *closeA;
    QAction  *copyWindowA;
    QAction  *filterOnA;
    QAction  *hideConsoleA;
    QAction  *hideToolBarA;
    QAction  *hideSearchToolBarA;
    QAction  *iconSizeSmallA;
    QAction  *iconSizeRegA;
    QAction  *iconSizeLargeA;
    QAction  *iconsOnlyA;
    QAction  *iconsWithTextA;
    QAction  *iconsTextOnlyA;
    QAction  *newWindowA;
    QAction  *newTabA;
    QAction  *copyTabA;
    QAction  *quitA;
    QAction  *saveA;
    QAction  *searchBackA;
    QAction  *searchBeginA;
    QAction  *searchEndA;
    QAction  *searchNextA;
    QAction  *searchEditA;
    QAction  *showMessageA;
    QActionGroup *hideColActionGroup;
    QActionGroup *iconSizeActionGroup;
    QActionGroup *iconsStyleGroup;
    QByteArray messageSplitterSettings;
    QColor menubarColor;
    QComboBox *searchCB;
    QDockWidget *consoleDock;
    QFileDialog *fileDialog;
    QLabel   *searchL;
    QLabel   *noDataL;
    QLineEdit *tabNameLineEdit;
    QMenu    *fileMenu;
    QMenu    *hideColumMenu;
    QMenu    *configureIconsMenu;
    QMenu    *iconSizeMenu;
    QMenu    *iconStyleMenu;
    QMenu    *optionMenu;
    QMenuBar *mainMenuBar;
    QPushButton *cancelEditTabNamePB;
    QPushButton *configPB;
    QPushButton *editTabNamePB;
    QSplitter *workAreaSplitter;
    QStackedWidget *stackW;
    QStandardItemModel *_model;
    QTabWidget *tabW;
    QTextBrowser *consoleArea;
    //QToggleButton editTabNamePB;
    QToolBar *mainToolBar;
    QToolBar *searchToolBar;
    QList <FixTable *> fixTableLists;
    QWidget *searchArea;
    QWidget *tabNameEditArea;
    void setAutoSaveOn(bool);
    void buildMainWindow();
    void displayConsoleMessage(GUI::Message);
    void showEvent(QShowEvent *);
    void readSettings();
    void writeSettings();
signals:
    void autoSaveOn(bool);
    void createWindow(MainWindow *);
    void copyWindow(MainWindow *);
    void deleteWindow(MainWindow *);
    void exitApp();
private:
    void buildHideColumnMenu();
    QByteArray fileDirState;
    QString  lastSelectedDir;
    QString fileFilter;
    int windowDataID;

};

#endif // MAINWINDOW_H
