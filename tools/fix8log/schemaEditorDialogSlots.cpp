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
#include <QApplication>
#include "schemaeditordialog.h"
#include "fix8sharedlib.h"
#include "schemaitem.h"
#include "database.h"
#include "globals.h"
using namespace GUI;
void SchemaEditorDialog::addItemToSelected(QBaseEntry *qbe,bool isChecked)
{
    QStringList messageNameList;
    QString tooltipStr;
    FieldUse *fieldUse;
    MessageField *messageField;
    QStandardItem *selectItem ;
    setUpdatesEnabled(false);

    QMap<QString,QStandardItem *>::iterator  iter  = selectedMap.find(qbe->name);
    selectItem = new QStandardItem(qbe->name);
    if (fieldUseList) {
        if (isChecked)    {
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
            else if (messageNameList.count() == 1)
                tooltipStr = "Used in " + messageNameList.at(0) + " message" ;
            else if(messageNameList.count() < 5)
                tooltipStr = "Used in: " + messageNameList.join("\n\t");
            else
                tooltipStr = "Used in " + QString::number(messageNameList.count()) + " messages";
            selectItem->setToolTip(tooltipStr);

            QVariant var;
            var.setValue((void *) qbe);
            selectItem->setData(var);
            if (tempTableSchema)
                tempTableSchema->addField(qbe);
            selectedFieldModel->appendRow(selectItem);
            selectedMap.insert(qbe->name,selectItem);
        }
        else {
            qDebug() << "HEY LETS REMOVE ITEM..........." << __FILE__ << __LINE__;
            selectItem = (QStandardItem *) iter.value();
            if (selectItem) {
                qDebug() << "Remove selected item:" << __FILE__ << __LINE__;
                qDebug() << "\tSlect item row " << selectItem->row();
                selectedFieldModel->removeRow(selectItem->row());
                if(tempTableSchema) {
                    tempTableSchema->removeFieldByName(qbe->name);
                }
                else
                    qWarning() << "Error - tempTableSchema is null" << __FILE__ << __LINE__;
            }
            selectedMap.remove(qbe->name);
            selectedBaseEntryList.removeOne(qbe);
        }
    }
    Qt::SortOrder so = selectedFieldsTreeView->header()->sortIndicatorOrder();
    selectedFieldModel->sort(0,so);
    setUpdatesEnabled(true);
}
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
    setUpdatesEnabled(false);
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
            if (tempTableSchema)
                tempTableSchema->addField(be);
            selectedFieldModel->appendRow(selectItem);
            selectedMap.insert(be->name,selectItem);
            //selectedBaseEntryList.append(be);
        }
    }
    else if (availItem->checkState() == Qt::Unchecked) {
        selectItem = (QStandardItem *) iter.value();
        if (selectItem) {
            qDebug() << "Remove selected item:" << __FILE__ << __LINE__;
            qDebug() << "\tSlect item row " << selectItem->row();
            selectedFieldModel->removeRow(selectItem->row());
            if(tempTableSchema) {
                tempTableSchema->removeFieldByName(be->name);
            }
            else
                qWarning() << "Error - tempTableSchema is null" << __FILE__ << __LINE__;
        }
        selectedMap.remove(be->name);
        selectedBaseEntryList.removeOne(be);
    }

    updateFieldsView();
    updateStatusOfMessageList();

    Qt::SortOrder so = selectedFieldsTreeView->header()->sortIndicatorOrder();
    selectedFieldModel->sort(0,so);
    setUpdatesEnabled(true);
}

void SchemaEditorDialog::availableSchemasClickedSlot(QModelIndex index)
{
    QStringList messageNameList;
    QString tooltipStr;
    FieldUse *fieldUse;
    MessageField *messageField;
    QBaseEntry *qbe;
    QVariant var;
    QStandardItem *selectItem;
    QStandardItem *availItem;
    QMap<QString,QStandardItem *>::iterator  iter;
    if (currentSchemaItem && (index == currentSchemaItem->index())) {
        return;
    }
    if (currentSchemaItem ) {
        if (tempTableSchema) {
            if (*currentTableSchema != *tempTableSchema) {
                QItemSelectionModel *selectionModel = availableSchemasListView->selectionModel();
                selectionModel->select(currentSchemaItem->index(),QItemSelectionModel::SelectCurrent);
                return;
            }
        }
    }
    currentSchemaItem = (SchemaItem *) availableSchemaModel->itemFromIndex(index);

    if (!currentSchemaItem)  {
        qWarning() << "Curent SchemaItem is null" << __FILE__ << __LINE__;
        return;
    }
    selectedMap.clear();
    selectedBaseEntryList.clear();
    selectedHeaderItem->setText(currentSchemaItem->text());
    currentTableSchema = currentSchemaItem->tableSchema;
    if (!currentTableSchema) {
        qWarning() << "No Current Table Schema Found" << __FILE__ << __LINE__;
        return;
    }
    if (tempTableSchema) {
        //tempTableSchema->clear();
        delete tempTableSchema;
    }
    tempTableSchema = currentTableSchema->clone();
    if (tempTableSchema->fieldList) {
        selectedBaseEntryList = *(tempTableSchema->fieldList);
    }
    int rowCount = selectedFieldModel->rowCount();
    if (rowCount > 0)
        selectedFieldModel->removeRows(0,rowCount);
    newSchemaLine->setText(currentTableSchema->name);
    descriptionE->setText(currentTableSchema->description);
    QList <QBaseEntry *> *bel = currentTableSchema->getFields();
    if (!bel) {
        qWarning() << "NO Field list found for current schema " << __FILE__ << __LINE__;
        validate();
        return;
    }
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    QListIterator <QBaseEntry *> iter2(*bel);
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
            selectedFieldModel->appendRow(selectItem);
            selectedMap.insert(qbe->name,selectItem);
            selectedBaseEntryList.append(qbe);
        }
        syncMessageViewWithFieldView();
    }
    QItemSelectionModel *selectionModel;
    selectionModel = messageListTreeView->selectionModel();
    selectionModel->select(messageModel->index(0,0),QItemSelectionModel::Select);
    disconnect(messageListTreeView,SIGNAL(clicked(QModelIndex)),
               this,SLOT(messageListClickedSlot(QModelIndex)));
    messageListClickedSlot(messageModel->index(0,0));
    connect(messageListTreeView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(messageListClickedSlot(QModelIndex)));
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    updateStatusOfMessageList();
    resetFieldsView();
    setUpdatesEnabled(true);
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
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    setCheckState(item,item->checkState());
    QStandardItem *parentItem = item->parent();
    if (item->checkState()  == Qt::Unchecked) {
        if (parentItem)
            setUncheckedStateParent(parentItem);

    }
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
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
    QItemSelectionModel *selectedSelModel =  selectedFieldsTreeView->selectionModel();
    QModelIndexList selectedList = selectedSelModel->selectedRows();
    QListIterator <QModelIndex> iter(selectedList);
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    setUpdatesEnabled(false);
    while(iter.hasNext()) {
        mi = iter.next();
        item = selectedFieldModel->itemFromIndex(mi);
        if (item) {
            var = item->data();
            selectedFieldModel->removeRow(item->row());
            QBaseEntry *be = (QBaseEntry *) var.value<void *>();
            selectedMap.remove(be->name);
            uncheckEntryInFieldsView(be);
            // selectedBaseEntryList.removeOne(be);
            tempTableSchema->removeFieldByName(be->name);
            QMultiMap<QString,QStandardItem *>::iterator aiter  = availableMap.find(be->name);
            while (aiter != availableMap.end() && aiter.key() == be->name) {
                availableItem = aiter.value();
                availableItem->setCheckState(Qt::Unchecked);
                aiter++;
            }
            break;
        }
    }
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    updateFieldsView();
    setUpdatesEnabled(true);
    validate();
}
void SchemaEditorDialog::clearAllSlot()
{
    int numOfRows = selectedFieldModel->rowCount();
    QStandardItem *selectedItem;
    QStandardItem *availableItem;
    selectedMap.clear();
    selectedFieldModel->removeRows(0,numOfRows);
    if (tempTableSchema) {
        tempTableSchema->removeAllFields();
    }
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    QMultiMap<QString,QStandardItem *>::iterator aiter  = availableMap.begin();
    while (aiter != availableMap.end()) {
        availableItem = aiter.value();
        availableItem->setCheckState(Qt::Unchecked);
        aiter++;
    }
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    selectedBaseEntryList.clear();
    //clearFieldsView();
    updateFieldsView();

    validate();
}
void SchemaEditorDialog::closeSlot()
{
    QSettings settings("fix8","logviewer");
    settings.setValue("SchemaEditorGeometry",saveGeometry());
    settings.setValue("SchemaEditorState",saveState());
    settings.setValue("SchemaEditorSplitter",splitter->saveState());
    if(tempTableSchema) {
        tempTableSchema->removeAllFields();
        delete tempTableSchema;
    }
    availableFieldModel->clear();
    messageModel->clear();
    availableSchemaModel->clear();
    selectedFieldModel->clear();
    emit finished(QDialogButtonBox::Close);
}
void SchemaEditorDialog::collapseAllSlot(bool on)
{
    if (on) {
        expandMode = CollapseAll;
        availableFieldsTreeView->collapseAll();
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
            tempTableSchema->addField(qbe);
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
            selectedFieldModel->appendRow(selectItem);
            selectedMap.insert(qbe->name,selectItem);
            //selectedBaseEntryList.append(qbe);
        }
    }
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    updateFieldsView();
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
        tempTableSchema->removeAllFields();
        delete tempTableSchema;
        tempTableSchema = 0;
    }
    setUpdatesEnabled(false);

    bstatus = database->deleteTableSchema(currentTableSchema->id);
    if (bstatus) {
        setMessage("Schema " + currentTableSchema->name + " deleted.",false);
        int id = currentTableSchema->id;
        availableSchemaModel->takeRow(currentSchemaItem->row());
        currentSchemaItem = 0;
        currentTableSchema = 0;
        emit schemaDeleted(id);
    }
    else {
        setMessage("Unable in removing schema: " + currentTableSchema->name + " from database.",false);
    }
    updateFieldsView();
    setUpdatesEnabled(true);
    validate();
}
void SchemaEditorDialog::expandAllSlot(bool on)
{
    if (on) {
        expandMode = ExpandAll;
        availableFieldsTreeView->expandAll();
        collapsePB->setChecked(false);
    }
    else
        expandMode = Anything;
}
void SchemaEditorDialog::messageListClickedSlot(QModelIndex mi)
{
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    Qt::SortOrder so = availableFieldsTreeView->header()->sortIndicatorOrder();
    availableFieldsTreeView->setUpdatesEnabled(false);
    if (availableFieldModel->rowCount() >0)
        availableFieldModel->removeRows(0,availableFieldModel->rowCount());
    QStandardItem *item = messageModel->itemFromIndex(mi);
    if (!item) {
        qWarning() << "Item is null " << __FILE__ << __LINE__;
        availableFieldsTreeView->setUpdatesEnabled(true);
        connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
                this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
        return;
    }
    QVariant var = item->data();
    MessageField *mf = (MessageField *) var.value<void *>();
    if (!mf) {
        qWarning() << "Error - MessageField = null" << __FILE__ << __LINE__;
        availableFieldsTreeView->setUpdatesEnabled(true);
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
            if (!qbe) {
                qWarning () << "Error - QBE = null " << __FILE__ << __LINE__;
                return;
            }
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
                //qDebug() << "!!!!!!!!!!!!!! HAVE LEVEL TWO" << __FILE__ << __LINE__;
                //qDebug() << "\tcount = " << qbe->baseEntryList->count();
                QListIterator <QBaseEntry *> iter2(*(qbe->baseEntryList));
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
                    //qDebug() << "APPEND ITEM 2 TO ROW " << item->text() << __FILE__ << __LINE__;
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
        availableFieldsTreeView->expandAll();
    else if (expandMode == CollapseAll)
        availableFieldsTreeView->collapseAll();
    availableFieldsTreeView->setUpdatesEnabled(true);
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
    newAvailableSchemasL->setText("Create New Table Schema");
    validate();
}
void SchemaEditorDialog::editSchemaSlot()
{
    bool bstatus;
    viewMode = EditMode;
    schemaArea->setCurrentIndex(NewMode);
    if(!currentTableSchema) {
        qWarning() << "Edit failed, current table schema is null" << __FILE__ << __LINE__;
        return;
    }
    newSchemaLine->setText(currentTableSchema->name);
    newDescriptionE->setText(currentTableSchema->description);
    newSchemaLine->setFocus();
    newAvailableSchemasL->setText("Edit Table Schema");
    validate();
}
void SchemaEditorDialog::saveNewEditSlot()
{// called for both save new and edit
    SchemaItem *si;
    bool bstatus;
    QString str;
    TableSchema *tableSchema;
    tableSchemaStatus = NoMods;
    QString name = newSchemaLine->text();
    QString description = newDescriptionE->toPlainText();
    if (!database) {
        qWarning() << "Error in saving shema - database is null" << __FILE__ << __LINE__;
        setMessage("Prgramming Error - Database is not defined",true);
        return;
    }
    if(!tableSchemaList) {
        qWarning() << "table schema list is null" << __FILE__ << __LINE__;
        setMessage("Proramming Error - Schema List Is Null",true);
        return;
    }
    descriptionE->setText("");
    if (viewMode == EditMode) {
        if (!currentSchemaItem) {
            return;
        }
        if (!currentSchemaItem->tableSchema) {
            return;
        }

        messageL->setText("");
        newSchemaLine->setText("");
        tableSchema = tableSchemaList->findByName(name);
        if (tableSchema) {
            if (tableSchema->id != currentSchemaItem->tableSchema->id) {
                str = "Schema name " + name + " is already in use.";
                QMessageBox::warning(this,"Error", str);
                setMessage("Error - Schema name" + name + " already in use",true);
                return;
            }
        }
        descriptionE->setText(description);
        currentSchemaItem->setText(name);
        currentSchemaItem->tableSchema->name = name;
        currentSchemaItem->tableSchema->description = description;
        if (tempTableSchema) {
            tempTableSchema->name = name;
            tempTableSchema->description = description;
        }
        viewMode = RegMode;
        schemaArea->setCurrentIndex(RegMode);
        bstatus = database->updateTableSchema(*(currentSchemaItem->tableSchema));
        if (!bstatus) {
            setMessage("Error - update schema faild in database",true);
            return;
        }
        emit tableSchemaUpdated(currentSchemaItem->tableSchema,true);
    }
    if(viewMode == NewMode) {
        if (tableSchemaList) {
            tableSchema = tableSchemaList->findByName(name);
            if (tableSchema) {
                str = "Schema name " + name + " is already in use.";
                QMessageBox::warning(this,"Error", str);
                setMessage("Error - Schema name" + name + " already in use",true);
                return;
            }
        }
        messageL->setText("");
        newSchemaLine->setText("");
        if (sharedLib && (sharedLib->fileName.length() > 0)) {
            tableSchema = new TableSchema(name,description,false,sharedLib->fileName);
            bstatus = database->addTableSchema(*tableSchema);
            if (!bstatus) {
                setMessage("Error - save to database failed",true);
                return;
            }
        }
        else {
            str = "Error - failed to create new schema as shard lib is currently not set";
            QMessageBox::warning(this,Globals::appName,str);
            return;
        }
        si = new SchemaItem(*tableSchema);
        availableSchemaModel->appendRow(si);
        viewMode = RegMode;
        schemaArea->setCurrentIndex(RegMode);
        tableSchemaList->append(tableSchema);
        QItemSelectionModel *selectedSelModel =  availableSchemasListView->selectionModel();
        selectedSelModel->setCurrentIndex(si->index(),QItemSelectionModel::ClearAndSelect);
        if (tempTableSchema) {
            tempTableSchema->removeAllFields();
            delete tempTableSchema;
        }
        tempTableSchema = tableSchema->clone();
        descriptionE->setText(tableSchema->description);
        currentSchemaItem = si;
        currentTableSchema = tableSchema;
        clearAllSlot();
        emit newSchemaCreated(tableSchema);
    }
}
void SchemaEditorDialog::applySlot()
{
    inUseTableSchema = tempTableSchema->clone();
    //inUseTableSchema->fieldList = tempTableSchema->fieldList;
    //currentTableSchema = tempTableSchema->clone();
    //currentTableSchema->fieldList = tempTableSchema->fieldList;
    //qDebug() << "*****CURRENT SCHEMA AFTER ASSIGN = " << currentTableSchema->name;
    if (currentMainWindow) {
        currentMainWindow->setTableSchema(inUseTableSchema);
    }
    validate();

}
// saves field changes
void SchemaEditorDialog::saveSchemaSlot()
{

    QString str;
    bool bstatus;
    if (!database) {
        qWarning() << "Save failed, as database is null " << __FILE__ << __LINE__;
        return;
    }
    if(!tempTableSchema) {
        qWarning() << "Save failed, current schema is null " << __FILE__ << __LINE__;
        return;
    }
    setCursor(Qt::BusyCursor);

    //currentTableSchema = tempTableSchema->clone();
    *currentTableSchema = *tempTableSchema;

    if (tempTableSchema->fieldList) {
        currentTableSchema->fieldList = tempTableSchema->fieldList->clone();
        qDebug() << "\tSetting currentTableSchea Field List, count =  " << currentTableSchema->fieldList->count() << __FILE__ << __LINE__;

    }

    if (currentSchemaItem)
        currentSchemaItem->tableSchema = currentTableSchema;
    //currentSchemaItem->tableSchema = tempTableSchema->clone();
    qDebug() << "SAVE SCHEMA TO DATABASE"  << __FILE__ << __LINE__;
    bstatus = database->saveTableSchemaFields(*tempTableSchema);

    qDebug() << "\tsaved " << tempTableSchema->name << __FILE__ << __LINE__;
    if (tempTableSchema->fieldList)
        qDebug() << "\tNum of fields" << tempTableSchema->fieldList->count();
    else
        qWarning() << "NO FIELD LIST ITEMS TO SAVE FOR SCHEMA" << __FILE__ << __LINE__;
    if (!bstatus) {
        str = "Failed in saving " + tempTableSchema->name + " to database";
        QMessageBox::warning(this,"Fix8LogViewer Error",str,QMessageBox::Ok,QMessageBox::NoButton);

    }
    validate();
    unsetCursor();
    qDebug() << "EMIT TABLE SCHEMA UPDATED, FIELD LIST = " << currentTableSchema->fieldList->count() << __FILE__ << __LINE__;
    emit tableSchemaUpdated(currentTableSchema,false);
}
void SchemaEditorDialog::selectedListClickedSlot(QModelIndex)
{
    validate();
}
void SchemaEditorDialog::setCheckState(QStandardItem *item,Qt::CheckState cs)
{
    QStandardItem *child;
    if (!item) {
        qWarning() << "ITEM IS NULL IN setCheckState" << __FILE__ << __LINE__;
        return;
    }
    qDebug() << "Check if items has children " << __FILE__ << __LINE__;
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
void SchemaEditorDialog::undoSchemaSlot()
{
    QStringList messageNameList;
    QString tooltipStr;
    FieldUse *fieldUse;
    MessageField *messageField;
    QBaseEntry *qbe;
    QVariant var;
    QStandardItem *selectItem;
    QStandardItem *availItem;
    QMap<QString,QStandardItem *>::iterator  iter;
    selectedMap.clear();
    selectedBaseEntryList.clear();
    selectedHeaderItem->setText(currentSchemaItem->text());
    currentTableSchema = currentSchemaItem->tableSchema;
    if (!currentTableSchema) {
        qWarning() << "No Current Table Schema Found" << __FILE__ << __LINE__;
        return;
    }
    if (tempTableSchema) {
        delete tempTableSchema;
        tempTableSchema = 0;
    }
    tempTableSchema = currentTableSchema->clone();
    if (tempTableSchema->fieldList)
        selectedBaseEntryList = *(tempTableSchema->fieldList);

    int rowCount = selectedFieldModel->rowCount();
    if (rowCount > 0)
        selectedFieldModel->removeRows(0,rowCount);
    newSchemaLine->setText(currentTableSchema->name);
    descriptionE->setText(currentTableSchema->description);
    QList <QBaseEntry *> *bel = currentTableSchema->getFields();
    if (!bel) {
        qWarning() << "NO Field list found for current schema " << __FILE__ << __LINE__;
        return;
    }
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
               this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    QListIterator <QBaseEntry *> iter2(*bel);
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
            selectedFieldModel->appendRow(selectItem);
            selectedMap.insert(qbe->name,selectItem);
            selectedBaseEntryList.append(qbe);
        }
    }
    QItemSelectionModel *selectionModel;
    selectionModel = messageListTreeView->selectionModel();
    selectionModel->select(messageModel->index(0,0),QItemSelectionModel::Select);
    disconnect(messageListTreeView,SIGNAL(clicked(QModelIndex)),
               this,SLOT(messageListClickedSlot(QModelIndex)));
    messageListClickedSlot(messageModel->index(0,0));
    connect(messageListTreeView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(messageListClickedSlot(QModelIndex)));
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));
    resetFieldsView();
    setUpdatesEnabled(true);
    validate();
}
void SchemaEditorDialog::viewActionSlot(QAction *action)
{
    qDebug() << "SET VIEW" << __FILE__ << __LINE__;
    if (action == messageViewA)
        workAreaStack->setCurrentIndex(MessageView);
    else
        workAreaStack->setCurrentIndex(FieldView);
}
