#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickItem>
#include "fixtable.h"
#include "globals.h"
#include "windowdata.h"
#include "worksheetdata.h"
#include <QList>
class FixToolBar;
class SearchLineEdit;
class QAction;
class QActionGroup;
class QDockWidget;
class QFileDialog;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QQuickView;
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
    MainWindow(bool showLoading=false);
    MainWindow(const MainWindow & sibling,bool copyAll = false);
    ~MainWindow();
    void addWorkSheet(QStandardItemModel *model,WorkSheetData &wsd);
    WindowData getWindowData();
    QList <WorkSheetData> getWorksheetData(int windowID);
    void setCurrentTabAndSelectedRow(int currentTab, int selectedRow);
    void setLoading(bool);
    void setLoadMessage(QString);
    void setWindowData(const WindowData &wd);

    protected slots:
    void autoSaveOnSlot(bool);
    void cancelSessionRestoreSlot();
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
    // time format travels up from work sheet
    void setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat);
    void setTimeFormatSlot(GUI::Globals::TimeFormat);
    QSize sizeHint() const;
    void setColorSlot(QColor color);
    void showMessageArea(bool);
    void tabCloseRequestSlot(int);
    void tabCurentChangedSlot(int);
    void tabNameModifiedSlot(QString);
    void tabNameReturnKeySlot();
    void toolbarOrientationChangedSlot(Qt::Orientation);
protected:
    enum {ShowNoDataLabel,ShowTab,ShowProgress};
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
    SearchLineEdit *searchLineEdit;
    QDockWidget *consoleDock;
    QFileDialog *fileDialog;
    QLabel   *searchL;
    QLabel   *searchLV; // searchLagle to show when vertical
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
    QQuickItem  *qmlObject;
    QQuickView *progressView;
    QSplitter *workAreaSplitter;
    QStackedWidget *stackW;
    QStandardItemModel *_model;
    QTabWidget *tabW;
    QTextBrowser *consoleArea;
    //QToggleButton editTabNamePB;

    QToolBar *mainToolBar;
    FixToolBar *searchToolBar;
    QList <FixTable *> fixTableLists;
    QWidget *progressWidget;
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
    void cancelSessionRestore();
    void createWindow(MainWindow *);
    void copyWindow(MainWindow *);
    void deleteWindow(MainWindow *);
    void exitApp();
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
private:
    void buildHideColumnMenu();
    QByteArray fileDirState;
    QString  lastSelectedDir;
    QString fileFilter;
    int windowDataID;
    bool loadingActive;

};

#endif // MAINWINDOW_H
