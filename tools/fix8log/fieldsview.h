#ifndef FIELDSVIEW_H
#define FIELDSVIEW_H

#include <QQuickWidget>

class FieldsView : public QQuickWidget
{
    Q_OBJECT
public:
    explicit FieldsView(QWidget *parent = 0);
    void setBackgroundColor(QColor &);
signals:

public slots:
private:
    QQuickItem *qmlObject;
};

#endif // FIELDSVIEW_H
