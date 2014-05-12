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
    TableSchema *currentTableSchema = currentSchemaItem->tableSchema;
    if (!currentTableSchema) {
        qWarning() << "No Current Table Schema Found" << __FILE__ << __LINE__;
        return;
    }
    newSchemaLine->setText(currentTableSchema->name);
    descriptionE->setText(currentTableSchema->description);

    validate();
}
