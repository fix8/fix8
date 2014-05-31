#ifndef SELECTEDFIELDSTREEVIEW_H
#define SELECTEDFIELDSTREEVIEW_H

#include <QTreeView>
#include <QRadialGradient>
class SelectedFieldsTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit SelectedFieldsTreeView(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
private:
    QString emptyStr1;
    QString emptyStr2;
    QFont   emptyFont;
    QRadialGradient grad;
    QColor  bgColorStart;
    QColor  bgColorEnd;
    QColor  emptyStrColor;
    int     emptyX1,emptyY1;
    int     emptyX2,emptyY2;
    QPointF center;
};

#endif // SELECTEDFIELDSTREEVIEW_H
