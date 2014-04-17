#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "fixtable.h"
#include <globals.h>
class QAction;
class QActionGroup;
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

public:
    MainWindow();
    MainWindow(const MainWindow & sibling,bool copyAll = false);
    ~MainWindow();
    protected slots:
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

    void setColorSlot(QColor color);
    //void showMessageSlot(FixTable::MessageKind mk,QString message);
    void showMessageArea(bool);
    void tabCloseRequestSlot(int);
    void tabCurentChangedSlot(int);
    void tabNameModifiedSlot(QString);
    void tabNameReturnKeySlot();
protected:
    void displayConsoleMessage(GUI::Message);
    void readSettings();
    void writeSettings();
signals:
    void createWindow(MainWindow *);
    void copyWindow(MainWindow *);
    void deleteWindow(MainWindow *);
protected:
    enum {ShowNoDataLabel,ShowTab};
    void showEvent(QShowEvent *);
    QAction  *closeA;
    QAction  *copyWindowA;
    QAction  *hideConsoleA;
    QAction  *hideToolBarA;
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
    QAction  *showMessageA;
    QActionGroup *hideColActionGroup;
    QActionGroup *iconSizeActionGroup;
    QActionGroup *iconsStyleGroup;
    QColor menubarColor;
    QDockWidget *consoleDock;
    QFileDialog *fileDialog;
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
    QList <FixTable *> fixTableLists;
    QWidget messageArea;
    QWidget *tabNameEditArea;
    void buildMainWindow();
private:
    void buildHideColumnMenu();
    QByteArray fileDirState;
    QString  lastSelectedDir;
    QString fileFilter;

};

#endif // MAINWINDOW_H
