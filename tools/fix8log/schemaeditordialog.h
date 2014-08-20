/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/

#ifndef SCHEMAEDITORDIALOG_H
#define SCHEMAEDITORDIALOG_H

#include <QDialog>
#include <QMainWindow>
#include <QtWidgets>
#include "schemaitem.h"
#include "tableschema.h"
#include "mainwindow.h"
#include "messagefield.h"
#include "messageitemdelegate.h"
#include "selectedfieldstreeview.h"

class Database;
class Fix8SharedLib;
class SchemaEditorDialog : public QMainWindow
{
    Q_OBJECT
public:
    enum {MessageView,FieldView};
    explicit SchemaEditorDialog(Database *database, QWidget *parent = 0);
    void restoreSettings();
    void saveSettings();
    void setTableSchemaInUse(TableSchema *inUse);
    void setSharedLibrary(Fix8SharedLib *f8sl);
    bool setCurrentTarget(MainWindow *mainWindow,bool isEditRequest=false);
    void windowDeleted(MainWindow *mw);
protected slots:
    void applySlot();
    void availableSchemasClickedSlot(QModelIndex);
    void availableTreeItemChangedSlot(QStandardItem*);
    void cancelNewSlot();
    void clearAllSlot();
    void clearSelectedSlot();
    void closeSlot();
    void collapseAllSlot(bool);
    void defaultSlot();
    void deleteSchemaSlot();
    void editSchemaSlot();
    void expandAllSlot(bool);
    void fieldCheckedSlot(QStandardItem *);
    void messageListClickedSlot(QModelIndex);
    void nameEditedSlot(const QString &);
    void newSchemaSlot();
    void saveNewEditSlot();
    void saveSchemaSlot();
    void selectedListClickedSlot(QModelIndex);
    void undoSchemaSlot();
    void viewActionSlot(QAction *);
protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
signals:
    void finished(int);
    void newSchemaCreated(TableSchema *);
    void schemaDeleted(int schemaID);
    void tableSchemaUpdated(TableSchema *,bool onlyNameOrDescription);
private:
    enum {NoMods,HaveMods,Empty};
    typedef enum {RegMode,NewMode,EditMode} ViewMode;
    typedef enum {Ok,SaveNeeded,EmptyFields} StatusType;
    typedef enum {ExpandAll,CollapseAll,Anything} ExpandMode;
    void addItemToSelected(QStandardItem *,Qt::CheckState);
    void addItemToSelected(QBaseEntry *be,bool isChecked); // called from field view
    void buildMessageView();
    void buildSchemaArea();
    void buildSelectedListFromCurrentSchema();
    void checkEntryInFieldsView(QBaseEntry *);
    void clearFieldsView();
    void resetFieldsView();
    void setCheckState(QStandardItem *item,Qt::CheckState cs);
    void setMessage(QString str, bool isError);
    void setStatus(StatusType);
    bool setTableSchemas();
    void setUncheckedStateParent(QStandardItem *parentItem);
    void showWindowArea(QString windowName);
    void syncMessageViewWithFieldView();
    void uncheckEntryInFieldsView(QBaseEntry *);
    void updateFieldsView();
    void updateStatusOfMessageList();
    bool validate();
    MessageItemDelegate *messageDelegate;
    QAction  *closeA;
    QAction  *saveA;
    QAction  *undoA;
    QAction  *applyA;
    QAction *newSchemaA;
    QAction *copySchemaA;
    QAction *editSchemaA;
    QAction *deleteSchemaA;
    QAction *messageViewA;
    QAction *fieldViewA;
    QActionGroup *viewActionGroup;
    QButtonGroup *expandBG;
    QColor editColor;
    QColor regularColor;
    QColor regMesssgeColor;
    QColor errorMessageColor;
    QGroupBox *descriptionBox;
    QGroupBox *newDescriptionBox;
    QLabel  *windowL;
    QLabel  *windowV;
    QListView *availableSchemasListView;
    //QListView *messageListView;
    QTreeView *messageListTreeView;

    QTreeView *availableFieldsTreeView;
    SelectedFieldsTreeView *selectedFieldsTreeView;

    QLabel *availableSchemasL;
    QLabel *messageListL;
    QLabel *newAvailableSchemasL;
    QLabel *newSchemaL;
    QLabel *availableListL;
    QLabel *selectedListL;
    QLineEdit *newSchemaLine;
    QLabel  *fix8versionL;
    QLabel  *fix8versionV;
    QMenu    *fileMenu;
    QMenuBar *mainMenuBar;
    QMultiMap <QString ,QStandardItem *> selectedMap; //<fieldName,selected>
    QMultiMap <QString ,QStandardItem *> availableMap; //<fieldName,selected>
    QPushButton  *defaultPB;
    QPushButton *cancelEditPB;
    QPushButton *saveEditPB;
    QPushButton *clearPB;
    QPushButton *clearAllPB;
    QPushButton *expandPB;
    QPushButton *collapsePB;
    QSplitter   *splitter;
    QScrollArea *schemaScrollArea;
    QSpacerItem *messageSpacerItem;
    QStackedWidget *schemaArea;
    QStandardItem  *availableFieldHeaderItem;
    QStandardItem  *selectedHeaderItem;
    QStandardItemModel *messageModel;
    QStandardItemModel *availableSchemaModel;
    QStandardItemModel *availableFieldModel;
    QStandardItemModel *selectedFieldModel;
    QStandardItemModel *fieldsModel;
    QTextEdit *descriptionE;
    QTextEdit *newDescriptionE;
    QToolBar *mainToolBar;
    QWidget *statusArea;
    QWidget *availableArea;
    QWidget *newSchemaArea;
    QWidget *targetArea;
    QLabel  *messageL;
    ViewMode viewMode;
    bool haveChanges;
    TableSchemaList *tableSchemaList;
    TableSchema *defaultTableSchema;
    SchemaItem *currentSchemaItem;
    SchemaItem *defaultSchemaItem ;
    Database *database;
    MessageFieldList *messageFieldList;
    ExpandMode expandMode;
    QMap<QString, QBaseEntry *> *baseMap;
    QMultiMap <QString,FieldTrait *> *fieldsInUseMap;
    FieldUseList *fieldUseList;
    QBaseEntryList *defaultHeaderItems;
    QBaseEntryList selectedBaseEntryList;
    TableSchema *currentTableSchema;
    TableSchema *inUseTableSchema;
    TableSchema *tempTableSchema;
    unsigned int tableSchemaStatus;
    QLabel *statusL;
    QLabel *statusI;
    StatusType statusValue;
    bool undoBuild;  // set and unset in undoSlot
    MainWindow *currentMainWindow;
    QStackedWidget *viewStack;
    QListView *fieldListView;
    QWidget   *centerW;
    QWidget   *messageView;

    QStackedWidget *workAreaStack;
    QWidget *messageWorkArea;
    QWidget *fieldsWorkArea;
    QLabel *fieldListL;
    QList<QPair<QString ,FieldUse *>> *fieldUsePairList;
    QList <QStandardItem *> fieldItems;
    QMap  <QString,QStandardItem *> fieldItemsMap;
    QColor fix8RegColor;
    Fix8SharedLib *sharedLib;
};

#endif // SCHEMAEDITORDIALOG_H
