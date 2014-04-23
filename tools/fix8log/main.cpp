#include "mainwindow.h"
#include <QApplication>
#include "fix8log.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Fix8Log *f8l = new Fix8Log(0);
    f8l->init();

    return a.exec();
}
