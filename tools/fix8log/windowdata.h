#ifndef WINDOWDATA_H
#define WINDOWDATA_H
#include <QByteArray>
#include <QColor>

class WindowData {
public:
    WindowData();
    WindowData(const WindowData &);
    int id;
    QByteArray geometry;
    QByteArray state;
    QColor     color;
    bool       isVisible;
    qint32     currentTab;
    QString    name;
    qint32     tableSchemaID;
};
#endif // WINDOWDATA_H
