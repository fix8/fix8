#ifndef FIXTOOLBAR_H
#define FIXTOOLBAR_H

#include <QToolBar>

class FixToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit FixToolBar(QString name, QWidget *parent = 0);
protected:
    bool event(QEvent *event);
signals:

public slots:

};

#endif // FIXTOOLBAR_H
