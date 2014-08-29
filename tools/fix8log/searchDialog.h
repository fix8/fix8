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

#ifndef SEARCH_DIALOG_H
#define SEARCH_DIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "editHighLighter.h"
#include "lineedit.h"
#include "mainwindow.h"
#include "searchfunction.h"
class Database;
class TableSchema;
class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    typedef enum  {SearchDialogType,FilterDialogType} DialogType;

  explicit SearchDialog(Database *database,TableSchema *tableSchema, DialogType dt = SearchDialogType,QWidget *parent = 0);
    typedef enum  {ViewMode,EditMode,NewMode} Mode;
    void setNewMode(QString searchStr);
    void setTableSchema(TableSchema *ts);
    void setMainWindow(MainWindow *mw);
    MainWindow *getMainWindow();
  QSize sizeHint () const;
 protected slots:
  void aliasChangedSlot(QString);
  void cancelSlot();
  void closeSlot();
  void deleteSlot();
  void exportSlot();
  void functionChangedSlot();
  void functionReturnSlot();
  void importSlot();
  void newSearchSlot();
  void editSearchSlot();
  void editSearchItemSlot(QModelIndex mi);
  void saveSlot();
  void rowSelectedSlot(QModelIndex);
  protected:
  void closeEvent(QCloseEvent *);
  void showEvent(QShowEvent *);
  void keyPressEvent(QKeyEvent *);
signals:
  void updatedSearchFunctions(SearchFunctionList *);
 private:
  QString createSearchRoutine(bool &bstatus);
  void populateSearchFunctions();
  void readXML(QString &fileName);
  void setMessage(QString str,bool isError = true);
  void validate();
  void writeXML(QString &fileName);
    QStyledItemDelegate *delegate;
    QLabel     *titleL;
    QDialogButtonBox *buttonBox;
    QWidget *workArea;
    QVBoxLayout *bbox;
    QTableView *tableView;
    QStandardItemModel *model;
    QLabel   *iconL;
    QPushButton *closeB;
    QPushButton *newB;
    QPushButton *deleteB;
    QPushButton *editB;

    QPushButton *exportB;
    QPushButton *importB;
    Database   *database;
    TableSchema *tableSchema;
    Mode mode;
    QGroupBox  *editArea;
    QLabel *aliasL;
    QLabel *functionL;
    QLabel *messageL;
    QLineEdit *aliasEdit;
    LineEdit *functionEdit;
    QPushButton *saveB;
    QPushButton *cancelB;
    LineEdit *le;
    QCompleter *searchCompleter;
    QPalette regPal;
    QPalette errorPal;
    bool haveError;
    QStringListModel *strModel;
    EditHighLighter *editHighlighter;
    QStandardItem *aliasItem;
    QStandardItem *functionItem;
    MainWindow    *mainWindow;
    SearchFunctionList *searchFunctionList;
    DialogType dialogType;
};
#endif // FILTERDIALOG_H
