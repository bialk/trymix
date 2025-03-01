#ifndef POLYGONTESTCONFORMINGDELANAY_H
#define POLYGONTESTCONFORMINGDELANAY_H

#include "PolygonTest.h"

#include <CGAL/enum.h>
#include <CGAL/Partition_2/partition_y_monotone_2.h>

#include <CGAL/basic.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Partition_traits_2<K>                         Traits;
typedef Traits::Point_2                                     Point_2;
typedef Traits::Polygon_2                                   Polygon_2;
typedef std::list<Polygon_2>                                Polygon_list;

class PolygonTestConformingDelanay: public PolygonTest{
public:
    PolygonTestConformingDelanay(int polygonSize,  float meshCellSize);

    QPolygonF const &getPolyline() override;
    QVector<QPointF> const &getEdges() override;

private:
    QPolygonF polyline;
    QVector<QPointF> pedges;

    Polygon_2    polygon;
    Polygon_list partition_polys;
};

#endif // POLYGONTESTCONFORMINGDELANAY_H
