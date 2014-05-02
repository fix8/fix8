#include "worksheetdata.h"
#include <QDebug>
WorkSheetData::WorkSheetData():id(-1),windowID(-1),selectedRow(-1)
{
}
WorkSheetData::WorkSheetData(const WorkSheetData &wsd)
{
    id            = wsd.id;
    windowID      = wsd.windowID;
    tabAlias      = wsd.tabAlias;
    splitterState = wsd.splitterState;
    headerState   = wsd.headerState;
    fileName      = wsd.fileName;
    selectedRow   = wsd.selectedRow;
}
WorkSheetData &WorkSheetData::operator=( const WorkSheetData &rhs)
{
    if (this == &rhs)
       return(*this);
    id            = rhs.id;
    windowID      = rhs.windowID;
    tabAlias      = rhs.tabAlias;
    splitterState = rhs.splitterState;
    headerState   = rhs.headerState;
    fileName      = rhs.fileName;
    selectedRow   = rhs.selectedRow;
    return *this;
}
