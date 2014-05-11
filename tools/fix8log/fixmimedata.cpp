#include "fixmimedata.h"

FixMimeData::FixMimeData():QMimeData(),model(0)
{
}
bool FixMimeData::hasFormat(const QString &str)
{
    if (str == "Fix8Log")
        return true;
    else
        return QMimeData::hasFormat(str);
}
