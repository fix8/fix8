#ifndef FIXMIMEDATA_H
#define FIXMIMEDATA_H

#include <QMimeData>
#include <QStandardItemModel>
#include <QUuid>
#include "globals.h"
class FixMimeData : public QMimeData
{
public:
    FixMimeData();
    bool hasFormat(const QString &mimeType);
    fix8logdata f8ld;
    QStandardItemModel *model;
    QUuid windowID;    // used for drag and drop
    QUuid worksheetID;
};

#endif // FIXMIMEDATA_H
