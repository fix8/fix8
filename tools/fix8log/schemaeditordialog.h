//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------

#ifndef SCHEMAEDITORDIALOG_H
#define SCHEMAEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "schemaitem.h"
#include "tableschema.h"
#include "messagefield.h"

class Database;
class SchemaEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SchemaEditorDialog(Database *database,bool globalSchemaOn, QWidget *parent = 0);
    void setCurrentTarget(QString &windowName, QString &tabName);
    void setTableSchemas(TableSchemaList *, TableSchema *defaultTableSchema);
    void populateMessageList(MessageFieldList *);
    void saveSettings();
    void restoreSettings();
signals:
    void newSchemaCreated(TableSchema *);
    void schemaDeleted(int schemaID);
public slots:
    void actionButtonSlot(QAbstractButton *button );
    void availableSchemasClickedSlot(QModelIndex);
    void availableTreeViewClickedSlot(QModelIndex);
    void expandAllSlot();
    void collapseAllSlot();
    void messageListClickedSlot(QModelIndex);
    void applyButtonSlot(QAbstractButton*);
    void cancelNewSlot();
    void deleteSchemaSlot();
    void nameEditedSlot(const QString &);
    void newSchemaSlot();
    void saveNewEditSlot();
protected:
    void showEvent(QShowEvent *);
private: 
    typedef enum {RegMode,NewMode,EditMode} ViewMode;
    void addItemToSelected(QStandardItem *,Qt::CheckState);
    QWidget  * buildSchemaArea();
    void setMessage(QString str, bool isError);
    bool validate();
    //SchemaEditorWidget *schemaWidget;
    QButtonGroup *applyBG;
    QColor editColor;
    QColor regularColor;
    QColor regMesssgeColor;
    QColor errorMessageColor;
    QDialogButtonBox *buttonBox;
    QGroupBox *descriptionBox;
    QLabel  *iconL;
    QLabel  *titleL;
    QLabel  *windowL;
    QLabel  *tabL;
    QLineEdit  *tabV;
    QLineEdit  *windowV;
    QListView *availableSchemasListView;
    QListView *messageListView;
    QTreeView *availableTreeView;
    QTreeView *selectedListView;
    QLabel *availableSchemasL;
    QLabel *messageListL;
    QLabel *newSchemaL;
    QLabel *availableListL;
    QLabel *selectedListL;
    QLineEdit *newSchemaLine;
    QMultiMap <QStandardItem *,QStandardItem *> selectedMap; //<avail,selected>
    QPushButton *applyB;
    QPushButton *closeB;
    QRadioButton *applyOnlyToCurrentRB;
    QRadioButton *applyToWindowRB;
    QRadioButton *applyToAllRB;
    QPushButton *newSchemaPB;
    QPushButton *copySchemaPB;
    QPushButton *editSchemaPB;
    QPushButton *deleteSchemaPB;
    QPushButton *cancelEditPB;
    QPushButton *saveEditPB;
    QPushButton *clearPB;
    QPushButton *clearAllPB;
    QPushButton *expandPB;
    QPushButton *collapsePB;
    QSplitter   *splitter;
    QScrollArea *schemaScrollArea;
    QStackedWidget *buttonStackArea;
    QStackedWidget *newSchemaStackArea;
    QStandardItem  *availableFieldHeaderItem;
    QStandardItem  *selectedHeaderItem;
    QStandardItemModel *messageModel;
    QStandardItemModel *schemaModel;
    QStandardItemModel *availableFieldModel;
    QStandardItemModel *selectedModel;


    QTextEdit *descriptionE;
    QWidget *newSchemaArea;
    QWidget *schemaArea;
    QWidget *targetArea;
    QLabel  *messageL;
    ViewMode viewMode;
    bool haveChanges;
    TableSchemaList *tableSchemaList;    
    TableSchema *defaultTableSchema;
    SchemaItem *currentSchemaItem;
    SchemaItem *defaultSchemaItem ;
    Database *database;
    bool globalSchemaOn;
    MessageFieldList *messageFieldList;
};

#endif // SCHEMAEDITORDIALOG_H
