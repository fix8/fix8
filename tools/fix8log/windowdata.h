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
    qint32     currentTab;
};
#endif // WINDOWDATA_H
