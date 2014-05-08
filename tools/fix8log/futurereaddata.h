#ifndef FUTUREREADDATA_H
#define FUTUREREADDATA_H
class QStandardItemModel;

#include "globals.h"
class FutureReadData
{
public:
    FutureReadData();
    QStandardItemModel *model;
    QList <GUI::Message> msgList;
    quint32 returnCode;
};

#endif // FUTUREREADDATA_H
