#include "worksheetdata.h"

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
