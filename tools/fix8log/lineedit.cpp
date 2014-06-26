#include "lineedit.h"
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QStyleOptionFrameV3>
LineEdit::LineEdit(QWidget *parent) :
    QTextEdit(parent)
{
    setTabChangesFocus(true);
    setWordWrapMode(QTextOption::NoWrap);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(sizeHint().height());

}

QSize LineEdit::sizeHint () const
{
    QFontMetrics fm(font());
    int h = qMax(fm.height(), 14) + 4;
    int w = fm.width(QLatin1Char('x')) * 17 + 4;
    QStyleOptionFrameV3 opt;
    opt.initFrom(this);
    return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).
                                      expandedTo(QApplication::globalStrut()), this));


}

void LineEdit::keyPressEvent (QKeyEvent *e)
{
    if ((e->key () == Qt::Key_Enter) || (e->key () == Qt::Key_Return))
        e->ignore ();
    else
        QTextEdit::keyPressEvent (e);
}
void LineEdit::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
