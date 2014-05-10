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
    void actionButtonSlot(QAbstractButton *button );
    void applyButtonSlot(QAbstractButton*);
    void cancelNewSlot();
    void nameEditedSlot(const QString &);
    void newSchemaSlot();
    void saveNewEditSlot();
private:
    typedef enum {RegMode,NewMode,EditMode} ViewMode;
    QWidget  * buildSchemaArea();
    void populateMessageList();
    bool validate();
    //SchemaEditorWidget *schemaWidget;
    QButtonGroup *applyBG;
    QColor editColor;
    QColor regularColor;
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
    QListView *availableListView;
    QListView *selectedListView;
    QLabel *availableSchemasL;
    QLabel *messageListL;
    QLabel *newSchemaL;
    QLabel *availableListL;
    QLabel *selectedListL;
    QLineEdit *newSchemaLine;
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
    QSplitter   *splitter;
    QScrollArea *schemaScrollArea;
    QStackedWidget *buttonStackArea;
    QStackedWidget *newSchemaStackArea;
    QStandardItemModel *messageModel;
    QStandardItemModel *schemaModel;
    QTextEdit *descriptionE;
    QWidget *newSchemaArea;
    QWidget *schemaArea;
    QWidget *targetArea;
    ViewMode viewMode;
    bool haveChanges;
};

#endif // SCHEMAEDITORDIALOG_H
