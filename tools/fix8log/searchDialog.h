#ifndef SEARCH_DIALOG_H
#define SEARCH_DIALOG_H

#include <QDialog>
#include <QtWidgets>

class Database;
class SearchDialog : public QDialog
{
    Q_OBJECT
public:
  explicit SearchDialog(Database *database,QWidget *parent = 0);
    typedef enum  {ViewMode,EditMode,NewMode} Mode;
  QSize sizeHint () const;
 protected slots:
  void newSearchSlot();
  void editSearchSlot();
  void rowSelectedSlot(QModelIndex);
  protected:
  void showEvent(QShowEvent *);

 private:
  void validate();
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

};
#endif // FILTERDIALOG_H
