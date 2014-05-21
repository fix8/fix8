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

#include "schemaeditordialog.h"
#include "schemaitem.h"
#include "database.h"
#include "globals.h"
using namespace GUI;

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

void SchemaEditorDialog::actionButtonSlot(QAbstractButton *button )
{
    if (button == closeB)
        emit finished(QDialogButtonBox::Close);
}
void SchemaEditorDialog::newSchemaSlot()
{
    bool bstatus;
    viewMode = NewMode;
    buttonStackArea->setCurrentIndex(NewMode);
    newSchemaStackArea->setCurrentIndex(NewMode);
    QPalette pal = descriptionE->palette();
    pal.setColor(QPalette::Base,editColor);
    descriptionE->setPalette(pal);
    descriptionE->setReadOnly(false);
    availableSchemasL->setText("New Schema");
    newSchemaLine->setText("");
    descriptionE->setText("");
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
        tableSchema = new TableSchema(name,descriptionE->toPlainText(),false);
        bstatus = database->addTableSchema(*tableSchema);
        if (!bstatus) {
            setMessage("Error - save to database failed",true);
            return;
        }
        si = new SchemaItem(*tableSchema);
        schemaModel->appendRow(si);
        viewMode = RegMode;
        newSchemaStackArea->setCurrentIndex(RegMode);
        buttonStackArea->setCurrentIndex(RegMode);
        QPalette pal = descriptionE->palette();
        pal.setColor(QPalette::Base,regularColor);
        descriptionE->setPalette(pal);
        descriptionE->setReadOnly(true);
        availableSchemasL->setText("Available Schemas");
        validate();
        tableSchemaList->append(tableSchema);
        emit newSchemaCreated(tableSchema);
    }
}
void SchemaEditorDialog::cancelNewSlot()
{
    viewMode = RegMode;
    messageL->setText("");
    newSchemaStackArea->setCurrentIndex(RegMode);
    buttonStackArea->setCurrentIndex(RegMode);
    QPalette pal = descriptionE->palette();
    pal.setColor(QPalette::Base,regularColor);
    descriptionE->setPalette(pal);
    descriptionE->setText("");
    descriptionE->setReadOnly(true);
    availableSchemasL->setText("Available Schemas");
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
}
void SchemaEditorDialog::nameEditedSlot(const QString &)
{
    messageL->setText("");
    validate();
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
    newSchemaLine->setText(currentTableSchema->name);
    descriptionE->setText(currentTableSchema->description);

    validate();
}
void SchemaEditorDialog::messageListClickedSlot(QModelIndex mi)
{
    disconnect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));

    Qt::SortOrder so = availableTreeView->header()->sortIndicatorOrder();
    availableTreeView->setUpdatesEnabled(false);
    if (availableFieldModel->rowCount() >0)
        availableFieldModel->removeRows(0,availableFieldModel->rowCount() -1);
   //vailableFieldHeaderItem = new QStandardItem("Fields");

   //vailableFieldModel->setColumnCount(1);
   // availableFieldModel->setHorizontalHeaderItem(0,availableFieldHeaderItem);

    qDebug() << "MEssage Item Clicked " << __FILE__ << __LINE__;
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
        while(iter.hasNext()) {
            QBaseEntry *qbe = iter.next();
            QStandardItem *item = new QStandardItem(qbe->name);
            QVariant var;
            var.setValue((void *) qbe);
            item->setData(var);
            item->setCheckable(true);
            availableFieldModel->setItem(i,item);
            if (qbe->baseEntryList) {
                QListIterator <QBaseEntry *> iter2(*qbe->baseEntryList);
                while(iter2.hasNext()) {
                    QBaseEntry *qbe2 = iter2.next();
                    QStandardItem *item2 = new QStandardItem(qbe2->name);
                    QVariant var2;
                    var2.setValue((void *) qbe2);
                    item2->setData(var2);
                    item2->setCheckable(true);
                    item->appendRow(item2);
                }
            }
            i++;
        }
    }
    availableFieldModel->sort(0,so);
    availableTreeView->setUpdatesEnabled(true);
    validate();
    connect(availableFieldModel,SIGNAL(itemChanged(QStandardItem*)),
            this,SLOT(availableTreeItemChangedSlot(QStandardItem*)));


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

    if (item->hasChildren()) {
        numOfChildren = item->rowCount();
        for(i=0;i<numOfChildren;i++) {
            child = item->child(i,0);
            if (child) {
                child->setCheckState(item->checkState());
                addItemToSelected(child,item->checkState());
            }
        }
    }
    else {
        QStandardItem *parentItem = item->parent();
        if(parentItem) {
            addItemToSelected(item,item->checkState());
            if (item->checkState() != Qt::Checked)
                parentItem->setCheckState(Qt::Unchecked);
        }
        addItemToSelected(item,item->checkState());
    }
    validate();
}
void SchemaEditorDialog::expandAllSlot()
{
    availableTreeView->expandAll();
}

void SchemaEditorDialog::collapseAllSlot()
{
    availableTreeView->collapseAll();
}
void SchemaEditorDialog::selectedListClickedSlot(QModelIndex)
{
    validate();
}
void SchemaEditorDialog::addItemToSelected(QStandardItem *availItem,Qt::CheckState cs)
{
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
    QMap<QBaseEntry *,QStandardItem *>::iterator  iter  = selectedMap.find(be);
    if (iter == selectedMap.end()) {
     if (availItem->checkState() == Qt::Checked)    {
         selectItem = new QStandardItem(be->name);
         selectItem->setData(var);
         selectedModel->appendRow(selectItem);
         selectedMap.insert(be,selectItem);
        }
    }
    else if (availItem->checkState() == Qt::Unchecked) {
            selectItem = (QStandardItem *) iter.value();
            if (selectItem) {
                selectedModel->removeRow(selectItem->row());
            }
            selectedMap.remove(be);
    }
    else {
        qDebug() << "\tNothing to do, item in list ?";
    }
    qDebug() << "STill to be worked out on how add items and save them" << __FILE__ <<__LINE__;
}
