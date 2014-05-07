#ifndef SCHEMAEDITORDIALOG_H
#define SCHEMAEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets>
class SchemaEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SchemaEditorDialog(QWidget *parent = 0);

signals:

public slots:

private:
    void populateMessageList();
    //SchemaEditorWidget *schemaWidget;
    QWidget *schemaArea;
    QDialogButtonBox *buttonBox;
    QPushButton *applyB;
    QPushButton *closeB;
    QLabel  *iconL;
    QLabel  *titleL;
    QListView *availableSchemasListView;
    QListView *messageListView;
    QListView *availableListView;
    QListView *selectedListView;
    QLabel *availableSchemasL;
    QLabel *messageListL;
    QLabel *availableListL;
    QLabel *selectedListL;
    QRadioButton *applyOnlyToCurrentRB;
    QRadioButton *applyToWindowRB;
    QRadioButton *applyToAllRB;
    QPushButton *newSchemaPB;
    QPushButton *copySchemaPB;
    QPushButton *editSchemaPB;
    QPushButton *deleteSchemaPB;
    QStandardItemModel *messageModel;




};

#endif // SCHEMAEDITORDIALOG_H
