#ifndef WORKSHEETDATA_H
#define WORKSHEETDATA_H
#include <QByteArray>
#include <QString>

class WorkSheetData
{
public:
    WorkSheetData();
    WorkSheetData(const WorkSheetData &);
    qint32     id;
    qint32     windowID;
    QString    tabAlias;
    QByteArray splitterState;
    QByteArray headerState;
    QString    fileName;
    QString    selectedRow;
};

#endif // WORKSHEETDATA_H
