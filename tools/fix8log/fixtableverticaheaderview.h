#ifndef FIXTABLEVERTICAHEADERVIEW_H
#define FIXTABLEVERTICAHEADERVIEW_H

#include <QHeaderView>

class FixTableVerticaHeaderView : public QHeaderView
{
public:
    explicit FixTableVerticaHeaderView(QWidget *parent = 0);
    void setHighlightList(QVector <qint32>,bool turnOn=true);
    void turnOnSearchHighLight(bool on);
protected:
   virtual void  paintSection(QPainter *painter,const QRect &,int logicalIndex) const;
   QVector <qint32> hightlightRows;
   bool highLightOn;
};

#endif // FIXTABLEVERTICAHEADERVIEW_H
