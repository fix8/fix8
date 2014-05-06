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
    QString    fileName;
    int        selectedRow;
    QByteArray splitterState;
    QByteArray headerState;
    WorkSheetData      &operator=( const WorkSheetData &s);

};

#endif // WORKSHEETDATA_H
