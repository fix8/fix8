#ifndef GLOBALS_H
#define GLOBALS_H
#include <QColor>
#include <QString>
#include <QSize>

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
    static float   version;
    static QString versionStr;
    static QSize smallIconSize;
    static QSize regIconSize;
    static QSize largeIconSize;
    static QColor menubarDefaultColor;

private:
    Globals(){};
    Globals(Globals const&){};
    static Globals* m_pInstance;
};
};

#endif // GLOBALS_H
