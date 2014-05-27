//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free so->ware: you can  redistribute it and / or modify  it under the  terms of the
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

#include "schemaeditordialog.h"
#include "schemaitem.h"
#include "database.h"
#include "globals.h"
using namespace GUI;
void SchemaEditorDialog::addItemToSelected(QStandardItem *availItem,Qt::CheckState cs)
{
    QStringList messageNameList;
    QString tooltipStr;
    FieldUse *fieldUse;
    MessageField *messageField;
    QStandardItem *selectItem ;
    if (!availItem) {
        qWarning() << "Item is null" << __FILE__ << __LINE__;
        return;
    }
    QVariant var = availItem->data();
    QBaseEntry *be = (QBaseEntry *) var.value<void *>();
    if (!be) {
        qWarning() << "Base Entry not found for item" << __FILE__ << __LINE__;
        return;
    }
    QMap<QString,QStandardItem *>::iterator  iter  = selectedMap.find(be->name);
    if (iter == selectedMap.end()) {
        if (availItem->checkState() == Qt::Checked)    {
            selectItem = new QStandardItem(be->name);
            if (fieldUseList) {
                fieldUse = fieldUseList->findByName(be->name);
                if (fieldUse) {
                    QListIterator <MessageField *> iter(fieldUse->messageFieldList);
                    while(iter.hasNext()) {
                        messageField = iter.next();
                        messageNameList << messageField->name;
                    }
                }
                if (messageNameList.count() == 0)
                    tooltipStr = "Not used in any messages";
                else if (messageNameList.count() == 1)
                    tooltipStr = "Used in " + messageNameList.at(0) + " message" ;
                else if(messageNameList.count() < 5)
                    tooltipStr = "Used in: " + messageNameList.join("\n\t");
                else
                    tooltipStr = "Used in " + QString::number(messageNameList.count()) + " messages";

                selectItem->setToolTip(tooltipStr);
            }
            selectItem->setData(var);
            selectedModel->appendRow(selectItem);
            selectedMap.insert(be->name,selectItem);
            selectedBaseEntryList.append(be);
        }
    }
    else if (availItem->checkState() == Qt::Unchecked) {
        selectItem = (QStandardItem *) iter.value();
        if (selectItem) {
            selectedModel->removeRow(selectItem->row());
        }
        selectedMap.remove(be->name);
        selectedBaseEntryList.removeOne(be);
    }
    Qt::SortOrder so = selectedListView->header()->sortIndicatorOrder();
    selectedModel->sort(0,so);
}
void SchemaEditorDialog::applyButtonSlot(QAbstractButton *button)
{
    if (button == applyOnlyToCurrentRB) {
        windowL->show();
        windowV->show();
        tabL->show();
        tabV->show();
    }
    else if (button == applyToWindowRB) {
        windowL->show();
        windowV->show();
        tabL->hide();
        tabV->hide();
    }
    else { // must be apply to all
        windowL->hide();
        windowV->hide();
        tabL->hide();
        tabV->hide();
    }
}
void SchemaEditorDialog::availableSchemasClickedSlot(QModelIndex index)
{
    messageL->setText("");
    currentSchemaItem = (SchemaItem *) schemaModel->itemFromIndex(index);
    if (!currentSchemaItem)  {
        qWarning() << "Curent SchemaItem is null" << __FILE__ << __LINE__;
        return;
    }
    selectedHeaderItem->setText(currentSchemaItem->text());

    TableSchema *currentTableSchema = currentSchemaItem->tableSchema;
    if (!currentTableSchema) {
        qWarning() << "No Current Table Schema Found" << __FILE__ << __LINE__;
        return;
    }
    if (tempTableSchema) {
        //tempTableSchema->clear();
        qDebug() << "MEMORY LEAK NEED TO CLEAR ITEMS OUT OF FIELD LIST HERE" << __FILE__ << __LINE__;
        delete tempTableSchema;
    }
    tempTableSchema = currentTableSchema->clone();
    newSchemaLine->setText(currentTableSchema->name);
    descriptionE->setText(currentTableSchema->description);

    validate();
}
void SchemaEditorDialog::availableTreeItemChangedSlot(QStandardItem* item)
{
    int i;
    int numOfChildren;
    QStandardItem *child=0;
    if (!item) {
        qWarning() << "Item is null " << __FILE__ << __LINE__;
        return;
    }
    setCheckState(item,item->checkState());
    QStandardItem *parentItem = item->parent();
    if (item->checkState()  == Qt::Unchecked) {
        disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
                   this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
        setUncheckedStateParent(parentItem);
        connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
                this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    }
    validate();
}
void SchemaEditorDialog::cancelNewSlot()
{
    viewMode = RegMode;
    messageL->setText("");
    schemaArea->setCurrentIndex(RegMode);

    validate();
}
void SchemaEditorDialog::clearSelectedSlot()
{
    QModelIndex mi;
    QStandardItem *item;
    QStandardItem *availableItem;
    QVariant var;
    QItemSelectionModel *selectedSelModel =  selectedListView->selectionModel();
    QModelIndexList selectedList = selectedSelModel->selectedRows();
    QListIterator <QModelIndex> iter(selectedList);
    while(iter.hasNext()) {
        mi = iter.next();
        item = selectedModel->itemFromIndex(mi);
        if (item) {
            var = item->data();
            selectedModel->removeRow(item->row());
            QBaseEntry *be = (QBaseEntry *) var.value<void *>();
            selectedMap.remove(be->name);
            selectedBaseEntryList.removeOne(be);
            QMultiMap<QString,QStandardItem *>::iterator aiter  = availableMap.find(be->name);
            while (aiter != availableMap.end() && aiter.key() == be->name) {
                availableItem = aiter.value();
                availableItem->setCheckState(Qt::Unchecked);
                aiter++;
            }
            break;
        }
    }
}
void SchemaEditorDialog::clearAllSlot()
{
  int numOfRows = selectedModel->rowCount();
  QStandardItem *selectedItem;
  QStandardItem *availableItem;
  selectedMap.clear();
  selectedBaseEntryList.clear();
  selectedModel->removeRows(0,numOfRows);
  QMultiMap<QString,QStandardItem *>::iterator aiter  = availableMap.begin();
  while (aiter != availableMap.end()) {
      availableItem = aiter.value();
      availableItem->setCheckState(Qt::Unchecked);
      aiter++;
  }
}
void SchemaEditorDialog::closeSlot()
{
    QSettings settings("fix8","logviewer");
    settings.setValue("SchemaEditorGeometry",saveGeometry());
    settings.setValue("SchemaEditorState",saveState());
    settings.setValue("SchemaEditorSplitter",splitter->saveState());

   emit finished(QDialogButtonBox::Close);
}
void SchemaEditorDialog::collapseAllSlot(bool on)
{
    if (on) {
        expandMode = CollapseAll;
        availableTreeView->collapseAll();
        expandPB->setChecked(false);
    }
    else
        expandMode = Anything;
}
void SchemaEditorDialog::defaultSlot()
{
    if (!defaultHeaderItems) {
        qWarning() << "ERROR- DEFAULT HEADER ITEMS IS NULL" << __FILE__ << __LINE__;
        return;
    }
    setUpdatesEnabled(false);
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    QStringList messageNameList;
    QString tooltipStr;
    FieldUse *fieldUse;
    MessageField *messageField;
    QBaseEntry *qbe;
    QVariant var;
    QStandardItem *selectItem;
    QStandardItem *availItem;
    QMap<QString,QStandardItem *>::iterator  iter;
    QListIterator <QBaseEntry *> iter2(*defaultHeaderItems);
    while(iter2.hasNext()) {
        qbe = iter2.next();
        iter  = selectedMap.find(qbe->name);
        if (iter == selectedMap.end()) { // not found
             // see if in available and if so mark it checked
             QMultiMap<QString,QStandardItem *>::iterator availIter = availableMap.find(qbe->name);
            if (availIter != availableMap.end()) {
                availItem = availIter.value();
                availItem->setCheckState(Qt::Checked);
            }
            selectItem = new QStandardItem(qbe->name);
            if (fieldUseList) {
                fieldUse = fieldUseList->findByName(qbe->name);
                if (fieldUse) {
                    QListIterator <MessageField *> iter(fieldUse->messageFieldList);
                    while(iter.hasNext()) {
                        messageField = iter.next();
                        messageNameList << messageField->name;
                    }
                }
                if (messageNameList.count() == 0)
                    tooltipStr = "Not used in any messages";

                if (messageNameList.count() == 1)
                    tooltipStr = "Used in " + messageNameList.at(0) + " message" ;
                else if(messageNameList.count() < 5)
                    tooltipStr = "Used in: " + messageNameList.join("\n\t");
                else
                    tooltipStr = "Used in " + QString::number(messageNameList.count()) + " messages";

                selectItem->setToolTip(tooltipStr);
            }
            var.setValue((void *) qbe);
            selectItem->setData(var);
            selectedModel->appendRow(selectItem);
            selectedMap.insert(qbe->name,selectItem);
            selectedBaseEntryList.append(qbe);
        }
    }
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    setUpdatesEnabled(true);
    validate();
}
void SchemaEditorDialog::deleteSchemaSlot()
{
    bool bstatus;
    if (!currentSchemaItem)
        return;
    TableSchema *currentTableSchema = currentSchemaItem->tableSchema;
    if (!currentTableSchema) {
        qWarning() << "No Current Table Schema Found" << __FILE__ << __LINE__;
        return;
    }
    if (currentTableSchema == defaultTableSchema)
        return;
    if (tempTableSchema) {
        qDebug() << "FIX THIS MEMORY LEAK HERE, DELETE LIST ITEMS" << __FILE__ << __LINE__;
        delete tempTableSchema;
        tempTableSchema = 0;
    }
    bstatus = database->deleteTableSchema(currentTableSchema->id);
    if (bstatus) {
        setMessage("Schema " + currentTableSchema->name + " deleted.",false);
        int id = currentTableSchema->id;
        schemaModel->takeRow(currentSchemaItem->row());
        currentSchemaItem = 0;
        currentTableSchema = 0;
        emit schemaDeleted(id);
        validate();
    }
    else {
        setMessage("Unable in removing schema: " + currentTableSchema->name + " from database.",false);
    }
    validate();
}
void SchemaEditorDialog::expandAllSlot(bool on)
{
    if (on) {
        expandMode = ExpandAll;
        availableTreeView->expandAll();
        collapsePB->setChecked(false);
    }
    else
        expandMode = Anything;
}
void SchemaEditorDialog::messageListClickedSlot(QModelIndex mi)
{
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    Qt::SortOrder so = availableTreeView->header()->sortIndicatorOrder();
    availableTreeView->setUpdatesEnabled(false);
    if (availableFieldModel->rowCount() >0)
        availableFieldModel->removeRows(0,availableFieldModel->rowCount() -1);
    QStandardItem *item = messageModel->itemFromIndex(mi);
    if (!item) {
        qWarning() << "Item is null " << __FILE__ << __LINE__;
        availableTreeView->setUpdatesEnabled(true);
        connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
                this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
        return;
    }
    QVariant var = item->data();
    MessageField *mf = (MessageField *) var.value<void *>();

    if (!mf) {
        qWarning() << "Error - MessageField = null" << __FILE__ << __LINE__;
        availableTreeView->setUpdatesEnabled(true);
        connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
                this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
        return;
    }
    int i=0;
    if (mf->qbel) {
        availableFieldHeaderItem->setText(item->text());
        availableFieldModel->setRowCount(mf->qbel->count());
        QListIterator <QBaseEntry *> iter(*(mf->qbel));
        availableMap.clear();
        while(iter.hasNext()) {
            QBaseEntry *qbe = iter.next();
            QStandardItem *item = new QStandardItem(qbe->name);
            availableMap.insert(qbe->name,item);
            QVariant var;
            var.setValue((void *) qbe);
            item->setData(var);
            item->setCheckable(true);
            QMap<QString,QStandardItem *>::iterator  siter  = selectedMap.find(qbe->name);
            if (siter != selectedMap.end()) {
                item->setCheckState(Qt::Checked);
            }
            // redo this code make it recursive
            availableFieldModel->setItem(i,item);
            if (qbe->baseEntryList) {
                QListIterator <QBaseEntry *> iter2(*qbe->baseEntryList);
                while(iter2.hasNext()) {
                    QBaseEntry *qbe2 = iter2.next();
                    QStandardItem *item2 = new QStandardItem(qbe2->name);
                    availableMap.insert(qbe2->name,item2);
                    QVariant var2;
                    var2.setValue((void *) qbe2);
                    item2->setData(var2);
                    item2->setCheckable(true);
                    siter = selectedMap.find(qbe2->name);
                    if (siter != selectedMap.end()) {
                        item2->setCheckState(Qt::Checked);
                    }
                    item->appendRow(item2);
                    if (qbe2->baseEntryList) {
                        QListIterator <QBaseEntry *> iter3(*qbe2->baseEntryList);
                        while(iter3.hasNext()) {
                            QBaseEntry *qbe3 = iter3.next();
                            QStandardItem *item3 = new QStandardItem(qbe3->name);
                            availableMap.insert(qbe3->name,item3);

                            QVariant var3;
                            var3.setValue((void *) qbe3);
                            item3->setData(var2);
                            item3->setCheckable(true);
                            siter = selectedMap.find(qbe3->name);
                            if (siter != selectedMap.end()) {
                                item3->setCheckState(Qt::Checked);
                            }
                            item2->appendRow(item3);
                        }
                    }
                }
            }
            i++;
        }
    }
    availableFieldModel->sort(0,so);
    if (expandMode == ExpandAll)
        availableTreeView->expandAll();
    else if (expandMode == CollapseAll)
        availableTreeView->collapseAll();
    availableTreeView->setUpdatesEnabled(true);
    validate();
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
}
void SchemaEditorDialog::nameEditedSlot(const QString &)
{
    messageL->setText("");
    validate();
}
void SchemaEditorDialog::newSchemaSlot()
{
    bool bstatus;
    viewMode = NewMode;
    schemaArea->setCurrentIndex(NewMode);
    newSchemaLine->setText("");
    newDescriptionE->setText("");
    newSchemaLine->setFocus();
    validate();
}
void SchemaEditorDialog::saveNewEditSlot()
{// called for both save new and edit
    SchemaItem *si;
    bool bstatus;
    TableSchema *tableSchema;
    QString name = newSchemaLine->text();
    descriptionE->setText("");
    if(viewMode == NewMode) {
        if (currentSchemaItem && currentSchemaItem->tableSchema)
            descriptionE->setText(currentSchemaItem->tableSchema->description);
        if (tableSchemaList) {
            tableSchema = tableSchemaList->findByName(name);
            if (tableSchema) {
                setMessage("Error - Name already in use",true);
                return;
            }
        }
        else {
            qWarning() << "table schema list is null" << __FILE__ << __LINE__;
            setMessage("Prgramming Error - Schema List Is Null",true);
            return;
        }
        if (!database) {
            qWarning() << "Error in saving shema - database is null" << __FILE__ << __LINE__;
            setMessage("Prgramming Error - Database is not defined",true);
            return;
        }
        messageL->setText("");
        newSchemaLine->setText("");
        tableSchema = new TableSchema(name,newDescriptionE->toPlainText(),false);
        bstatus = database->addTableSchema(*tableSchema);
        if (!bstatus) {
            setMessage("Error - save to database failed",true);
            return;
        }
        si = new SchemaItem(*tableSchema);
        schemaModel->appendRow(si);
        viewMode = RegMode;
        schemaArea->setCurrentIndex(RegMode);
        validate();
        tableSchemaList->append(tableSchema);
        QItemSelectionModel *selectedSelModel =  availableSchemasListView->selectionModel();
        selectedSelModel->setCurrentIndex(si->index(),QItemSelectionModel::ClearAndSelect);
         descriptionE->setText(tableSchema->description);
         currentSchemaItem = si;
         currentTableSchema = tableSchema;
         clearAllSlot();
         validate();

        emit newSchemaCreated(tableSchema);
    }
}
void SchemaEditorDialog::saveSchemaSlot()
{
    qDebug() << "Save Schema Slot " << __FILE__ << __LINE__;
    if (!database) {
        qWarning() << "Save failed, as database is null " << __FILE__ << __LINE__;
        return;
    }
    if(!currentSchemaItem) {
        qWarning() << "Save failed, current schema is null " << __FILE__ << __LINE__;
        return;
    }
    TableSchema * tableSchema = currentSchemaItem->tableSchema;
    if (!tableSchema) {
        qWarning() << "Save failed, current table schema is null " << __FILE__ << __LINE__;
        return;
    }
    tableSchema->fieldList = new  QBaseEntryList(selectedBaseEntryList);
    database->saveTableSchemaFields(*tableSchema);
}

void SchemaEditorDialog::selectedListClickedSlot(QModelIndex)
{
    validate();
}
void SchemaEditorDialog::setCheckState(QStandardItem *item,Qt::CheckState cs)
{
    QStandardItem *child;
    if (!item) {
        return;
    }
    if (!item->hasChildren()) {
        addItemToSelected(item,cs);
        return;
    }
    int numOfChildren = item->rowCount();
    for(int i=0;i<numOfChildren;i++) {
        child = item->child(i,0);
        if (child) {
            child->setCheckState(cs);
            addItemToSelected(child,cs);
            setCheckState(child,cs);
        }
    }
}
void SchemaEditorDialog::setUncheckedStateParent(QStandardItem *parentItem)
{
    if (!parentItem)
        return;
    parentItem->setCheckState(Qt::Unchecked);
    setUncheckedStateParent(parentItem->parent());
}

