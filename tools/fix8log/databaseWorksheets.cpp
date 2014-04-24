#include "database.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>

QList <WorkSheetData> Database::getWorkSheets(int windowID)
{
  QList <WorkSheetData> wsdList;
  return wsdList;
}

bool Database::addWorkSheet(WorkSheetData &)
{
    bool bstatus = false;
    return bstatus;
}

bool Database::deleteWorkSheetByWindowID(int windowID)
{
    bool bstatus = false;
    return bstatus;
}
bool Database::deleteWorkSheet(int workSheetID)
{
    bool bstatus = false;
    return bstatus;
}
