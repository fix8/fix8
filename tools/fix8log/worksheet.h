#ifndef WORKSHEET_H
#define WORKSHEET_H
#include <QQuickItem>
#include <QWidget>
#include <QModelIndex>
#include <QUuid>
#include "fixHeaderView.h"
#include "globals.h"
#include "worksheetdata.h"
class QFile;
class QLabel;
class QMenu;
class QQuickView;
class QSplitter;
class QStackedLayout;
class QStandardItem;
class QStandardItemModel;
class DateTimeDelegate;
class FixTable;
class FixMimeData;
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
    void setWindowID( QUuid &);
    QUuid getID();
    ~WorkSheet();
    enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
    static QString headerLabel[NumColumns];
    QString getFileName();
    QStandardItemModel *getModel();
    WorkSheetData getWorksheetData();
    bool loadFileName(QString &fileName,
                      QList <GUI::Message> &returnMessageList,
                      quint32 &returnCode);
    void hideColumn(int colNum, bool hideCol);
    void setAlias(QString &);
    void showLoadProcess(bool isBeingLoaded);
    void setTimeFormat(GUI::Globals::TimeFormat);
signals:
    void modelDropped(FixMimeData *);
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
    void sendMessage(GUI::Message);
    void sendMessages(QList < GUI::Message>);
public slots:
    void cancelLoadSlot();
    void popupHeaderMenuSlot(int col,const QPoint &);
    void rowSelectedSlot(QModelIndex);
    void timeFormatSelectedSlot(QAction *);
    void modelDroppedSlot(FixMimeData *);
protected:
    QSplitter *splitter;
    FixTable *fixTable;
    MessageArea   *messageArea; // temp for now - pace holder
    void build();
private:
    QActionGroup *timeActionGroup;
    QAction *timeFormatActions[GUI::Globals::NumOfTimeFormats];
    QStackedLayout *stackLayout;
    QFile *fixFile;
    QMenu *timeFormatMenu;
    QQuickView *progressView;
    QString fixFileName;
    QStandardItemModel *_model;
    QStandardItem *headerItem[NumColumns];
    QString alias;
    QWidget *progressWidget;
    QQuickItem  *qmlObject;
    bool cancelLoad;
    int linecount;
    WorkSheetData origWSD;
    DateTimeDelegate *dateTimeDelegate;
    QUuid windowID;
    QUuid uuid;
};

#endif // WORKSHEET_H
