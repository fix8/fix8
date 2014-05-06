#include "mainwindow.h"
#include <QApplication>
#include <QDataStream>
#include "fix8log.h"
#include "globals.h"
using namespace GUI;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<fix8logdata>("fix8logdata");
    qRegisterMetaTypeStreamOperators<fix8logdata>("fix8logdata");
    Fix8Log *f8l = new Fix8Log(0);
    f8l->init();

    return a.exec();
}
