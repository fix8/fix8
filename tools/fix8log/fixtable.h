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

#ifndef FIX_TABLE_VIEW_H
#define FIX_TABLE_VIEW_H
class QFile;
class QKeyEvent;

#include <QDate>
#include <QMap>
#include <QRadialGradient>
#include <QTableView>
#include <QUuid>
class QHeaderView;
class FixHeaderView;
class FixMimeData;
class QStandardItem;
class QStandardItemModel;



class FixTable: public QTableView {
  Q_OBJECT
 public:
  enum {MsgSeqNum,SenderCompID,TargetCompID,SendingTime,BeginStr,BodyLength,CheckSum,EncryptMethod,HeartBtInt,MessageType,NumColumns};
  FixTable(QUuid &windowID, QUuid &workSheetID,QWidget * parent = 0);
  //FixTable(const FixTable &);
  void setWindowID(QUuid &uuid);
  ~FixTable();
  static QString headerLabel[NumColumns];
  QSize sizeHint () const;
 protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent *event);
  void mousePressEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
 signals:
  void doPopup(const QModelIndex &,const QPoint &);
  void modelDropped(FixMimeData  *);
  //  void doHeaderPopup(GradeHeaderItem *,const QPoint &);
 private:
  QString emptyStr1;
  QString emptyStr2;
  QFont   emptyFont;
  QColor  bgColorStart;
  QColor  bgColorEnd;
  QRadialGradient grad;
  QColor  emptyStrColor;
  int     emptyX1,emptyY1;
  int     emptyX2,emptyY2;
  QPointF center;
  //QStandardItemModel *_model;
  //QStandardItem *headerItem[NumColumns];
  QFile *dataFile;
  FixHeaderView *fixHeader;
  QPoint dragStartPosition;
  QUuid windowID;
  QUuid worksheetID;
};
#endif
