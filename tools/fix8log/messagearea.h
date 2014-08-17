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

#ifndef MESSAGEAREA_H
#define MESSAGEAREA_H
#include <fix8/f8includes.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"

#include <QWidget>
class QAbstractButton;
class QButtonGroup;
class QGroupBox;
class QLabel;
class QRadioButton;
class QStackedLayout;
class QStandardItem;
class QStandardItemModel;
class QTreeView;
class MessageFieldList;
class QMessage;
class Fix8SharedLib;
class MessageArea : public QWidget
{
Q_OBJECT
public:
    explicit MessageArea(QWidget *parent = 0);
    typedef enum {HeaderItem=0,FieldsItem=1,TrailerItem=2} TreeItem;
    typedef enum {ExpandNone,ExpandOne,ExpandAll,ExpandUnknown} ExpandMode;
    void setMessage(QMessage *);
    void setItemExpaned(TreeItem,bool);
    bool getExpansionState(TreeItem);   
    void setHeaderState(QByteArray &headerState);
    void setSharedLib(Fix8SharedLib *);
    quint32 getExpansion();
    void setExpansion(quint32 value);
    QByteArray getHeaderState();
protected slots:
  void expandMessageSlot(int);
private:
    void generateItems(FIX8::GroupBase *gb,QStandardItem *parent,FIX8::Message *, int *i);
    QStackedLayout *stackLayout;
    QTreeView     *treeView;
    QStandardItem *treeHeaderItem;
    QStandardItem *valueHeaderItem;

    QStandardItem *headerItem;
    QStandardItem *tailItem;
    QStandardItem *fieldItem;

    QStandardItemModel *model;
    QWidget *infoArea;
    QLabel  *seqNumV;
    QLabel  *messageTypeV;
    QMessage *currentMessage;
    QButtonGroup *expandButtonGroup;
    QRadioButton *expandNone,*level1,*expandAll;
    QGroupBox    *messageExpansionArea;
    ExpandMode expandMode;
    Fix8SharedLib *sharedLib;

};

#endif // MESSAGEAREA_H
