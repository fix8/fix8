#ifndef WORKSHEET_H
#define WORKSHEET_H
#include <QQuickItem>
#include <QWidget>
#include <QModelIndex>
#include "globals.h"
#include "worksheetdata.h"
class QFile;
class QLabel;
class QQuickView;
class QSplitter;
class QStackedLayout;
class QStandardItem;
class QStandardItemModel;
class FixTable;
class MessageArea;
#define OK             0x0000
#define CANCEL         0x0001
#define READ_ERROR     0x0002
#define FILE_NOT_FOUND 0x0004
#define OPEN_FAILED    0x0080

class WorkSheet : public QWidget
{
    friend class MainWindow;
    Q_OBJECT
public:
    explicit WorkSheet(QWidget *parent = 0);
    WorkSheet(QStandardItemModel *model,const WorkSheetData &wsd,QWidget *parent = 0);
    WorkSheet(WorkSheet &,QWidget *parent = 0);
    ~WorkSheet();
    enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
    static QString headerLabel[NumColumns];
    //FixTable(QWidget * parent = 0);
    QString getFileName();
    QStandardItemModel *getModel();
    WorkSheetData getWorksheetData();
    bool loadFileName(QString &fileName,
                      QList <GUI::Message> &returnMessageList,
                      quint32 &returnCode);
    void hideColumn(int colNum, bool hideCol);
    void setAlias(QString &);
    void showLoadProcess(bool isBeingLoaded);
signals:
    void sendMessage(GUI::Message);
    void sendMessages(QList < GUI::Message>);
public slots:
    void cancelLoadSlot();
    void rowSelectedSlot(QModelIndex);
protected:
    QSplitter *splitter;
    FixTable *fixTable;
    MessageArea   *messageArea; // temp for now - pace holder
    void build();
private:
    QStackedLayout *stackLayout;
    QFile *fixFile;
    QQuickView *progressView;
    QString fixFileName;
    QStandardItemModel *_model;
    QStandardItem *headerItem[NumColumns];
    QString alias;
    QWidget *progressWidget;
    QQuickItem  *qmlObject;
    bool cancelLoad;

};

#endif // WORKSHEET_H
