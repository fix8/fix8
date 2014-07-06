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
  explicit SearchDialog(Database *database,TableSchema *tableSchema, QWidget *parent = 0);
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
};
#endif // FILTERDIALOG_H
