#ifndef SCHEMAEDITORDIALOG_H
#define SCHEMAEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets>
class SchemaEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SchemaEditorDialog(QWidget *parent = 0);
    void setCurrentTarget(QString &windowName, QString &tabName);

signals:

public slots:
    void applyButtonSlot(QAbstractButton*);
private:
    void populateMessageList();
    //SchemaEditorWidget *schemaWidget;
    QWidget *schemaArea;
    QButtonGroup *applyBG;
    QDialogButtonBox *buttonBox;
    QPushButton *applyB;
    QPushButton *closeB;
    QLabel  *iconL;
    QLabel  *titleL;
    QLabel  *windowL;
    QLabel  *windowV;
    QLabel  *tabL;
    QLabel  *tabV;
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
    QWidget *targetArea;



};

#endif // SCHEMAEDITORDIALOG_H
