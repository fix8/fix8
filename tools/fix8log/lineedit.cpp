#include "lineedit.h"
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QStyleOptionFrameV3>
#include <QCompleter>
#
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
LineEdit::LineEdit(QWidget *parent) :
    QTextEdit(parent),c(0)
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
void LineEdit::setCompleter(QCompleter *completer)
 {
     if (c)
         QObject::disconnect(c, 0, this, 0);

     c = completer;

     if (!c)
         return;

     c->setWidget(this);
     c->setCompletionMode(QCompleter::PopupCompletion);
     c->setCaseSensitivity(Qt::CaseInsensitive);
     QObject::connect(c, SIGNAL(activated(QString)),
                      this, SLOT(insertCompletion(QString)));
 }

 QCompleter *LineEdit::completer() const
 {
     return c;
 }

 void LineEdit::insertCompletion(const QString& completion)
 {
     if (c->widget() != this)
         return;
     QTextCursor tc = textCursor();
     int extra = completion.length() - c->completionPrefix().length();
     tc.movePosition(QTextCursor::Left);
     tc.movePosition(QTextCursor::EndOfWord);
     tc.insertText(completion.right(extra));
     setTextCursor(tc);
 }
 QString LineEdit::textUnderCursor() const
  {
      QTextCursor tc = textCursor();
      tc.select(QTextCursor::WordUnderCursor);
      return tc.selectedText();
  }
 void LineEdit::focusInEvent(QFocusEvent *e)
 {
     if (c)
         c->setWidget(this);
     QTextEdit::focusInEvent(e);
 }

void LineEdit::keyPressEvent (QKeyEvent *e)
{
    if (c && c->popup()->isVisible()) {
            // The following keys are forwarded by the completer to the widget
           switch (e->key()) {
           case Qt::Key_Enter:
           case Qt::Key_Return:
           case Qt::Key_Escape:
           case Qt::Key_Tab:
           case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
           default:
               break;
           }
        }

        bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
        if ((e->key () == Qt::Key_Enter) || (e->key () == Qt::Key_Return)) {
            e->ignore ();
            emit returnPressed();
            return;
        }
        if (!c || !isShortcut) // do not process the shortcut when we have a completer
            QTextEdit::keyPressEvent(e);

        const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
        if (!c || (ctrlOrShift && e->text().isEmpty()))
            return;

        static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
        bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
        QString completionPrefix = textUnderCursor();

        if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2
                          || eow.contains(e->text().right(1)))) {
            c->popup()->hide();
            return;
        }

        if (completionPrefix != c->completionPrefix()) {
            c->setCompletionPrefix(completionPrefix);
            c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
        }
        QRect cr = cursorRect();
        cr.setWidth(c->popup()->sizeHintForColumn(0)
                    + c->popup()->verticalScrollBar()->sizeHint().width());
        c->complete(cr); // popup it up!

    /*
    if ((e->key () == Qt::Key_Enter) || (e->key () == Qt::Key_Return)) {
        e->ignore ();
        emit returnPressed();
    }
    else
        QTextEdit::keyPressEvent (e);
        */
}
void LineEdit::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
