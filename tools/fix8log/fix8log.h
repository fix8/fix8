#ifndef FIX8LOG_H
#define FIX8LOG_H

#include <QObject>
#include "mainwindow.h"
class Fix8Log : public QObject
{
    Q_OBJECT
public:
    explicit Fix8Log(QObject *parent = 0);

public slots:
    void createNewWindowSlot(MainWindow *mw=0);
    void copyWindowSlot(MainWindow *mw);

    void deleteMainWindowSlot(MainWindow *mw);
protected:
    QList <MainWindow *> mainWindows;

};

#endif // FIX8LOG_H
