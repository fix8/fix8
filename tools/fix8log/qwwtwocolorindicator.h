//
// C++ Interface: qwwtwocolorindicator
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTWOCOLORINDICATOR_H
#define QWWTWOCOLORINDICATOR_H

#ifndef WW_NO_TWOCOLORINDICATOR

#include <QWidget>
#include <wwglobal.h>

class QwwTwoColorIndicatorPrivate;
class Q_WW_EXPORT QwwTwoColorIndicator : public QWidget, QwwPrivatable
{
Q_OBJECT
Q_PROPERTY(QColor fgColor READ fgColor WRITE setFgColor)
Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
Q_PROPERTY(bool active READ isActive WRITE setActive)
Q_PROPERTY(bool dragEnabled READ dragEnabled WRITE setDragEnabled)
public:
    QwwTwoColorIndicator(QWidget *parent = 0);

    const QColor &fgColor() const;
    const QColor &bgColor() const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    bool isActive() const;
    bool dragEnabled() const;
    void setDragEnabled(bool);
public slots:
    void setFgColor(const QColor &);
    void setBgColor(const QColor &);
    void switchColors();
    void setActive(bool);
signals:
    void fgChanged(const QColor &);
    void bgChanged(const QColor &);
    void fgClicked();
    void bgClicked();
    void fgPressed();
    void bgPressed();
protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
    virtual void paintSection(QPainter *p, const QRect &rect, const QColor &color);
private:
    WW_DECLARE_PRIVATE(QwwTwoColorIndicator);

};

#endif // WW_NO_TWOCOLORINDICATOR

#endif // QWWTWOCOLORINDICATOR_H
