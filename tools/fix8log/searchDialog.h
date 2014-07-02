#ifndef SEARCH_DIALOG_H
#define SEARCH_DIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "lineedit.h"
class Database;
class SearchDialog : public QDialog
{
    Q_OBJECT
public:
  explicit SearchDialog(Database *database,QWidget *parent = 0);
    typedef enum  {ViewMode,EditMode,NewMode} Mode;
  QSize sizeHint () const;
 protected slots:
  void cancelSlot();
  void newSearchSlot();
  void editSearchSlot();
  void saveSlot();
  void rowSelectedSlot(QModelIndex);
  protected:
  void showEvent(QShowEvent *);

 private:
  void validate();
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
    QPushButton *undoB;
    QPushButton *exportB;
    QPushButton *importB;
    Database   *database;
    Mode mode;
    QGroupBox  *editArea;
    QLabel *aliasL;
    QLabel *functionL;
    QLineEdit *aliasEdit;
    LineEdit *functionEdit;
    QPushButton *saveB;
    QPushButton *cancelB;
    LineEdit *le;
    QCompleter *searchCompleter;
};
#endif // FILTERDIALOG_H
