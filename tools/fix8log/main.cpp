#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDataStream>
#include "fix8log.h"
#include "globals.h"
using namespace GUI;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    bool loadFile = false;
    QString loadFileName;
    QCoreApplication::setApplicationName("fix8log");
    QCoreApplication::setApplicationVersion(QString::number(Globals::version));
    QCommandLineParser parser;
    parser.parse(QCoreApplication::arguments());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription("FIX Log Viewer");
    QCommandLineOption  loadFileOption(QStringList() << "i" << "input", "Load log file <file>.", "file");
    parser.addOption(loadFileOption);
    parser.process(a);
    if (parser.isSet(loadFileOption)) {
        loadFileName= parser.value(loadFileOption);
        qDebug() << "Load File Name = " << loadFileName;
        QFileInfo fi(loadFileName);
        if (!fi.exists()) {
            qWarning() << "Error - " << loadFileName << " does not exist.";
            exit(-1);
        }
        loadFile = true;
    }
    qRegisterMetaType<fix8logdata>("fix8logdata");
    qRegisterMetaTypeStreamOperators<fix8logdata>("fix8logdata");
    Fix8Log *f8l = new Fix8Log(0);
    if (loadFile)
        f8l->init(loadFileName);
    else
        f8l->init();
    return a.exec();
}
