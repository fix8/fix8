//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WW_NO_TWOCOLORINDICATOR

#include "qwwtwocolorindicator.h"
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QMouseEvent>
#include <QColorDialog>
#include <QApplication>
#include <QMimeData>
#include <QDragEnterEvent>
#include "wwglobal_p.h"

/*!
 * \class QwwTwoColorIndicator
 * \mainclass
 * \inmodule wwWidgets
 * \brief The QwwTwoColorIndicator class provides a widget allowing to choose a foreground and background colour.
 *
 * \image qwwtwocolorindicator.png QwwTwoColorIndicator
 *
 * \ingroup colorclasses
 */
/*!
 * \property QwwTwoColorIndicator::fgColor
 * \brief    foreground colour
 */
/*!
 * \property QwwTwoColorIndicator::bgColor
 * \brief    background colour
 */
/*!
 * \property QwwTwoColorIndicator::active
 * \brief    whether mouse click enables colour select
 */
/*!
 *  \fn      void QwwTwoColorIndicator::fgChanged(const QColor &color)
 *  \brief   This signal is emitted when foreground colour changes to \a color.
 *  \sa      bgChanged()
 */
/*!
 *  \fn      void QwwTwoColorIndicator::bgChanged(const QColor &color)
 *  \brief   This signal is emitted when background colour changes to \a color.
 *  \sa      fgChanged()
 */
/*!
 *  \fn      void QwwTwoColorIndicator::fgClicked()
 *  \brief   This signal is emitted when the foreground button was clicked.
 *  \sa      bgClicked(), fgPressed()
 */
/*!
 *  \fn      void QwwTwoColorIndicator::bgClicked()
 *  \brief   This signal is emitted when the background button was clicked.
 *  \sa      fgClicked(), bgPressed()
 */
/*!
 *  \fn      void QwwTwoColorIndicator::fgPressed()
 *  \brief   This signal is emitted when the foreground button was pressed.
 *  \sa      bgPressed(), fgClicked()
 */
/*!
 *  \fn      void QwwTwoColorIndicator::bgPressed()
 *  \brief   This signal is emitted when the background button was pressed.
 *  \sa      fgPressed(), bgClicked()
 */

class QwwTwoColorIndicatorPrivate : public QwwPrivate {
public:
    QwwTwoColorIndicatorPrivate(QwwPrivatable *p) : QwwPrivate(p),
            fg(Qt::black), bg(Qt::white) {
        fgP = false;
        bgP = false;
        active = true;
        dragEnabled = false;
    }
    QRect foregroundRect() const {
        Q_Q(const QwwTwoColorIndicator);
        return QRect(0, 0, 2*q->width()/3, 2*q->height()/3);
    }
    QRect backgroundRect() const {
        Q_Q(const QwwTwoColorIndicator);
        return QRect(q->width()/3, q->height()/3, 2*q->width()/3, 2*q->height()/3);
    }
    QRect switchButtonRect() const {
        Q_Q(const QwwTwoColorIndicator);
        return QRect(1+q->width()/6, 1+4*q->height()/6, q->width()/6-1, q->height()/6-1);
    }
    QColor fg;
    QColor bg;
    QPoint pressPos;
    bool fgP;
    bool bgP;
    bool active;
    bool dragEnabled;
    WW_DECLARE_PUBLIC(QwwTwoColorIndicator);
};

/*!
 * Constructs a two color indicator with a given \a parent.
 */
QwwTwoColorIndicator::QwwTwoColorIndicator(QWidget *parent)
        : QWidget(parent), QwwPrivatable(new QwwTwoColorIndicatorPrivate(this)) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAcceptDrops(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::paintEvent(QPaintEvent *) {
    Q_D(QwwTwoColorIndicator);
    QPainter painter(this);
    paintSection(&painter, d->backgroundRect(), d->bg);
    paintSection(&painter, d->foregroundRect(), d->fg);
}


/*!
 * \brief Paints a single section of the indicator.
 *
 *        The method paints to the rectangle \a rect with \a color using \a painter.
 *
 *        Reimplement to modify how the indicator looks.
 */
void QwwTwoColorIndicator::paintSection(QPainter * painter, const QRect & rect, const QColor & color) {
    Q_D(QwwTwoColorIndicator);
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.rect = rect;
    if ((rect.contains(QPoint(3,3)) && d->fgP) || (rect.contains(QPoint(width()-4, height()-4)) && d->bgP))
        opt.state |= QStyle::State_Sunken;
    else
        opt.state |= QStyle::State_Raised;
    if (rect==d->foregroundRect() && rect.contains(mapFromGlobal(QCursor::pos()))) {
        opt.state |= QStyle::State_MouseOver;
    } else if (rect==d->backgroundRect() && rect.contains(mapFromGlobal(QCursor::pos())) && !d->foregroundRect().contains(mapFromGlobal(QCursor::pos()))) {
        opt.state |= QStyle::State_MouseOver;
    } else opt.state &= ~(QStyle::State_MouseOver);
    style()->drawControl(QStyle::CE_PushButtonBevel, &opt, painter, this);
    QRect fillRect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
    int offset = (opt.rect.height() - fillRect.height())/2;
    fillRect = opt.rect.adjusted(offset,offset,-offset,-offset);
    if (opt.state & QStyle::State_Sunken) fillRect.translate(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal), style()->pixelMetric(QStyle::PM_ButtonShiftVertical));
    painter->setPen(Qt::NoPen);
    painter->setBrush(opt.state & QStyle::State_Enabled ? color : palette().color(QPalette::Disabled, QPalette::Window ));
    // check if (0,0) hits the button - if yes -> drawRect, otherwise drawRoundedRect
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundedRect(fillRect, 3, 3);
}


/*!
 *  Swaps foreground and background colors.
 */
void QwwTwoColorIndicator::switchColors() {
    Q_D(QwwTwoColorIndicator);
    if (d->fg==d->bg) return;
    qSwap(d->fg, d->bg);
    emit bgChanged(d->bg);
    emit fgChanged(d->fg);
    update();
}

/*!
 *  Sets the background color
 *  \param b color to set
 */
void QwwTwoColorIndicator::setBgColor(const QColor &b) {
    Q_D(QwwTwoColorIndicator);
    if (b==d->bg) return;
    d->bg = b;
    emit bgChanged(b);
    update();
}

/*!
 *  Sets the foreground color
 *  \param f color to set
 */
void QwwTwoColorIndicator::setFgColor(const QColor &f) {
    Q_D(QwwTwoColorIndicator);
    if (f==d->fg) return;
    d->fg = f;
    emit fgChanged(f);
    update();
}



const QColor & QwwTwoColorIndicator::bgColor() const {
    Q_D(const QwwTwoColorIndicator);
    return d->bg;
}

const QColor & QwwTwoColorIndicator::fgColor() const {
    Q_D(const QwwTwoColorIndicator);
    return d->fg;
}

bool QwwTwoColorIndicator::isActive() const {
    Q_D(const QwwTwoColorIndicator);
    return d->active;
}

void QwwTwoColorIndicator::setActive(bool a) {
    Q_D(QwwTwoColorIndicator);
    d->active = a;
}


/*!
 * \internal
 */
QSize QwwTwoColorIndicator::sizeHint() const {
    return QSize(50,50).expandedTo(QApplication::globalStrut()*1.5);
}


/*!
 * \internal
 */
QSize QwwTwoColorIndicator::minimumSizeHint() const {
    return QSize(20,20).expandedTo(QApplication::globalStrut());
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::mousePressEvent(QMouseEvent *ev) {
    Q_D(QwwTwoColorIndicator);
    d->pressPos = ev->pos();
    if (ev->button() == Qt::LeftButton) {
        if (d->foregroundRect().contains(ev->pos())) {
            d->fgP = true;
            d->bgP = false;
            emit fgPressed();
            update();
        } else if (d->backgroundRect().contains(ev->pos())) {
            d->bgP = true;
            d->fgP = false;
            emit bgPressed();
            update();
        }
    }
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::mouseMoveEvent(QMouseEvent *ev) {
    Q_D(QwwTwoColorIndicator);
    if (d->fgP || d->bgP) {
        if (dragEnabled() && (ev->pos() - d->pressPos).manhattanLength() >= QApplication::startDragDistance()) {
            QColor col = d->fgP ? fgColor() : bgColor();
            ColorDrag *drag = new ColorDrag(this, col, col.name());
            drag->exec();
            d->fgP = d->bgP = false;
            update();
        }
    } else if (testAttribute(Qt::WA_Hover)) update();
    QWidget::mouseMoveEvent(ev);
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::mouseReleaseEvent(QMouseEvent *ev) {
    Q_D(QwwTwoColorIndicator);
    if (ev->button() == Qt::LeftButton) {
        if (d->fgP && d->foregroundRect().contains(ev->pos())) {
            emit fgClicked();
            if (d->active) {
#if QT_VERSION >= 0x040500
                QColor c = QColorDialog::getColor(d->fg, this, tr("Choose foreground color"), QColorDialog::ShowAlphaChannel);
#else
                QColor c = QColorDialog::getColor(d->fg, this);
#endif
                if (c.isValid()) {
                    setFgColor(c);
                }
            }
        } else if (d->bgP && d->backgroundRect().contains(ev->pos())) {
            emit bgClicked();
            if (d->active) {
#if QT_VERSION >= 0x040500
                QColor c = QColorDialog::getColor(d->bg, this, tr("Choose background color"), QColorDialog::ShowAlphaChannel);
#else
                QColor c = QColorDialog::getColor(d->bg, this);
#endif
                if (c.isValid()) {
                    setBgColor(c);
                }
            }
        }
        d->fgP = d->bgP = false;
        update();
    }
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::dragEnterEvent(QDragEnterEvent *ev) {
    if (ev->mimeData()->hasColor()) {
        QColor col(qvariant_cast<QColor>(ev->mimeData()->colorData()));
        if (col.isValid())
            ev->acceptProposedAction();
    } else if (ev->mimeData()->hasFormat("text/plain")) {
        QColor col;
        col.setNamedColor(ev->mimeData()->text());
        if (col.isValid())
            ev->acceptProposedAction();
    } else ev->ignore();
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::dragMoveEvent(QDragMoveEvent *ev) {
    Q_D(QwwTwoColorIndicator);
    bool acc = false;
    if (ev->mimeData()->hasColor()) {
        QColor col(qvariant_cast<QColor>(ev->mimeData()->colorData()));
        if (col.isValid())
            acc = true;
    } else if (ev->mimeData()->hasFormat("text/plain")) {
        QColor col;
        col.setNamedColor(ev->mimeData()->text());
        if (col.isValid())
            acc = true;
    } else ev->ignore();
    if (d->foregroundRect().intersects(ev->answerRect())) {
        ev->acceptProposedAction();
        ev->accept(d->foregroundRect());
    } else if (d->backgroundRect().intersects(ev->answerRect())) {
        ev->acceptProposedAction();
//         ev->accept(d->backgroundRect());
    } else ev->ignore();
}

/*!
 * \internal
 */
void QwwTwoColorIndicator::dropEvent(QDropEvent *ev) {
    Q_D(QwwTwoColorIndicator);
    QColor col;
    if (ev->mimeData()->hasColor())
        col = qvariant_cast<QColor>(ev->mimeData()->colorData());
    else
        col.setNamedColor(ev->mimeData()->text());
    if (d->foregroundRect().contains(ev->pos())) {
        setFgColor(col);
    } else if (d->backgroundRect().contains(ev->pos())) {
        setBgColor(col);
    }
    ev->setDropAction(Qt::CopyAction);
}

/*!
 * \property QwwTwoColorIndicator::dragEnabled
 * This property holds whether dragging from the widget is enabled.
 */
bool QwwTwoColorIndicator::dragEnabled() const {
    Q_D(const QwwTwoColorIndicator);
    return d->dragEnabled;
}

void QwwTwoColorIndicator::setDragEnabled(bool v) {
    Q_D(QwwTwoColorIndicator);
    d->dragEnabled = v;
}



#endif
