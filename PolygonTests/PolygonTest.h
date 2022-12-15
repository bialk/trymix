#ifndef POLYGONTEST_H
#define POLYGONTEST_H

#include <QPolygonF>
#include <QPointF>

class PolygonTest{
public:
    virtual QPolygonF const &getPolyline() = 0;
    virtual QVector<QPointF> const &getEdges() = 0;
    virtual ~PolygonTest(){};
};

#endif // POLyGONTEST_H
