#include "worksheetdata.h"

WorkSheetData::WorkSheetData()
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
