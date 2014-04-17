#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QWidget>
#include <QModelIndex>
#include "globals.h"
class QFile;
class QLabel;
class QSplitter;
class QStackedLayout;
class QStandardItem;
class QStandardItemModel;
class FixTable;
class MessageArea;
class WorkSheet : public QWidget
{
    friend class MainWindow;
    Q_OBJECT
public:
    explicit WorkSheet(QWidget *parent = 0);
    WorkSheet(WorkSheet &,QWidget *parent = 0);
    ~WorkSheet();
    enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
    static QString headerLabel[NumColumns];

    //FixTable(QWidget * parent = 0);

    QString getFileName();
    QStandardItemModel *getModel();
    bool loadFileName(QString &fileName,QList <GUI::Message> &returnMessageList);
    void  hideColumn(int colNum, bool hideCol);
signals:
    void sendMessage(GUI::Message);
    void sendMessages(QList < GUI::Message>);
public slots:
    void rowSelectedSlot(QModelIndex);
protected:
    QSplitter *splitter;
    FixTable *fixTable;
    MessageArea   *messageArea; // temp for now - pace holder
private:
    QStackedLayout *stackLayout;
    QFile *fixFile;
    QString fixFileName;
    QStandardItemModel *_model;
    QStandardItem *headerItem[NumColumns];
};

#endif // WORKSHEET_H
