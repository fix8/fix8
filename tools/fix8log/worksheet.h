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

#ifndef WORKSHEET_H
#define WORKSHEET_H
#include <QActionGroup>
#include <QQuickItem>
#include <QWidget>
#include <QModelIndex>
#include <QUuid>
#include "fixHeaderView.h"
#include "globals.h"
#include "messagefield.h"
#include "worksheetdata.h"
class TableSchema;
class WorkSheetModel;
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
#
class WorkSheet : public QWidget
{
    friend class MainWindow;
    Q_OBJECT
public:
    explicit WorkSheet(QWidget *parent = 0);
    WorkSheet(WorkSheetModel *model,const WorkSheetData &wsd,QWidget *parent = 0);
    WorkSheet(WorkSheet &,QWidget *parent = 0);
    typedef enum {SearchFirst,SearchBack,SearchNext,SearchLast,SearchOff,ResumeSearch} SearchType;
    enum {SearchFinished=0x0001,SearchHasPrevious=0x0002,SearchHasNext=0x0004,SearchEmpty=0x0008,SearchOk = 0x0010};
    enum {OK =0x0000,CANCEL = 0x0001,READ_ERROR=0x0002,FILE_NOT_FOUND=0x0004,
        OPEN_FAILED=0x0080,TERMINATED=0x0100};
    ~WorkSheet();
    bool copyFrom(WorkSheet &oldws);
    QVector <qint32> getSearchIndexes();
    bool loadCanceled();
    void setSearchString(const QString &);
    QString &getSearchString();
    void setWindowID( QUuid &);
    void setTableSchema(TableSchema *);
    void setSearchIndexes(const QVector <qint32> &indexes);
    TableSchema *getTableSchema();
    QUuid getID();
    quint32 doSearch(SearchType);
    QMessageList *getMessageList();
    //enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
   // static QString headerLabel[NumColumns];
    QString getFileName();
    WorkSheetModel *getModel();
    QMenu *getSenderMenu();
    WorkSheetData getWorksheetData();
    bool loadFileName(QString &fileName,
                      QList <GUI::ConsoleMessage> &returnMessageList,
                      quint32 &returnCode);
    void hideColumn(int colNum, bool hideCol);
    void setAlias(QString &);
    void showLoadProcess(bool isBeingLoaded, int numRecords=0);
    void setTimeFormat(GUI::Globals::TimeFormat);
    void terminate();; // called to stop loading file if it is
signals:
    void modelDropped(FixMimeData *);
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
    void sendMessage(GUI::ConsoleMessage);
    void sendMessages(QList < GUI::ConsoleMessage>);
    void rowSelected(int row);
    void terminateCopy(WorkSheet *);
public slots:
    void cancelLoadSlot();
    void popupHeaderMenuSlot(int col,const QPoint &);
    void rowSelectedSlot(QModelIndex);
    void timeFormatSelectedSlot(QAction *);
    void modelDroppedSlot(FixMimeData *);
    void senderActionGroupSlot(QAction *);
protected:
    QSplitter *splitter;
    FixTable *fixTable;
    QActionGroup *senderActionGroup;
    QAction      *showAllSendersA;
    QMenu        *senderMenu;
    MessageArea   *messageArea; // temp for now - pace holder
    void build();
    void buildHeader();
private:
    QActionGroup *timeActionGroup;
    QAction *timeFormatActions[GUI::Globals::NumOfTimeFormats];
    QStackedLayout *stackLayout;
    QFile *fixFile;
    QMenu *timeFormatMenu;
    QQuickView *progressView;
    QString fixFileName;
    WorkSheetModel *_model;
    // QStandardItem *headerItem[NumColumns];
    QVector <QStandardItem *> headerItems;
    QString alias;
    QWidget *progressWidget;
    QQuickItem  *qmlObject;
    bool cancelLoad;
    WorkSheetData origWSD;
    DateTimeDelegate *dateTimeDelegate;
    QUuid windowID;
    QUuid uuid;
    TableSchema *tableSchema;
    QMessageList *messageList;
    quint32 cancelReason;
    QVector<qint32> searchLogicalIndexes;
    QItemSelectionModel *sm;
    qint32 currentRow;
    QString searchString; // raw search string
};

class WorkSheetList : public QList <WorkSheet *>
{
  public:
    WorkSheetList(QWidget *parent = 0);
    bool removeByID(const QUuid &id);
};
#endif // WORKSHEET_H
