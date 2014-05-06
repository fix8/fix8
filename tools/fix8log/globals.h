#ifndef GLOBALS_H
#define GLOBALS_H
#include <QColor>
#include <QDataStream>
#include <QString>
#include <QSize>
#include <QUuid>

class QDesktopWidget;

namespace GUI {
class Message {
public:
    typedef enum {ErrorMsg,WarningMsg,InfoMsg} MessageType;
    Message(QString, MessageType mt= InfoMsg);
    Message(const Message &);
    Message();
    QString msg;
    MessageType messageType;
};

class Globals
{
public:
    static Globals* Instance();
    typedef enum {DAYMONYRHHMMSS,DAYMMMHHMMSS,HHMMSS,HHMM,NumOfTimeFormats} TimeFormat;
    static QString timeFormats[NumOfTimeFormats];
    static float   version;
    static QString versionStr;
    static QSize smallIconSize;
    static QSize regIconSize;
    static QSize largeIconSize;
    static QColor menubarDefaultColor;
    static TimeFormat timeFormat;
private:
    Globals(){};
    Globals(Globals const&){};
    static Globals* m_pInstance;
};
};
struct fix8logdata {
    QUuid  windowID;
    QUuid  worksheetID;
};

QDataStream &operator<<(QDataStream &out, const fix8logdata &data);
QDataStream &operator>>(QDataStream &in, fix8logdata &data);
Q_DECLARE_METATYPE(fix8logdata);

#endif // GLOBALS_H
