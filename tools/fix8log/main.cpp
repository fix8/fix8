//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

#include "mainwindow.h"
#include <qtsingleapplication.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDataStream>
#include "fix8log.h"
#include "globals.h"
using namespace GUI;
int main(int argc, char *argv[])
{
    QtSingleApplication instance(argc, argv);
        if (instance.sendMessage("Wake up!"))
            return 0;

    //bool loadFile = false;
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
    parser.process(instance);
    if (parser.isSet(loadFileOption)) {
        loadFileName= parser.value(loadFileOption);
        qDebug() << "Load File Name = " << loadFileName;
        QFileInfo fi(loadFileName);
        if (!fi.exists()) {
            qWarning() << "Error - " << loadFileName << " does not exist.";
            exit(-1);
        }
        //loadFile = true;
    }
    qRegisterMetaType<fix8logdata>("fix8logdata");
    qRegisterMetaTypeStreamOperators<fix8logdata>("fix8logdata");
    Fix8Log *f8l = new Fix8Log(&instance);
    QObject::connect(&instance,SIGNAL(messageReceived(const QString&)),
                     f8l,SLOT(wakeupSlot(const QString&)));

    //QString fixLib = "/home/f8log/"
    //if (loadFile)
    //    f8l->init(loadFileName);
   // else
        f8l->init();
    return instance.exec();

}
