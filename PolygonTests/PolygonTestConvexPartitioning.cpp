#include "PolygonTestConvexPartitioning.h"

#include <CGAL/Partition_traits_2.h>
#include <CGAL/partition_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/random_polygon_2.h>
#include <cassert>
#include <list>

typedef CGAL::Creator_uniform_2<int, Point_2>               Creator;
typedef CGAL::Random_points_in_square_2<Point_2, Creator>   Point_generator;

constexpr double M_PI = 3.14159265358979323846;


PolygonTestConvexPartitioning::PolygonTestConvexPartitioning(int polygonSize)
{
    CGAL::random_polygon_2(polygonSize, std::back_inserter(polygon),
                           Point_generator(600));

    //circle test example
    /*
    for(auto i=0; i< 29; ++i){
        polygon.push_back(Point_2(25*sin(-i*2.*M_PI/30),25*cos(-i*2.*M_PI/30)));
    }
    */


    CGAL::optimal_convex_partition_2(polygon.vertices_begin(),
                                     polygon.vertices_end(),
                                     std::back_inserter(partition_polys));

    for(auto& i: polygon){
        polyline.append({i.x(), i.y()});
    }
    polyline.append(polyline.first());

    // converting paritions into triangulation
    for(auto& edges: partition_polys){
        Point_2 prevPtn = {edges[edges.size()-1].x(),edges[edges.size()-1].y()};
        std::pair<float,float> centerPtn = {0.,0.};
        for(auto& e: edges){
            pedges.append({prevPtn.x(),prevPtn.y()});
            pedges.append({e.x(),e.y()});
            prevPtn = e;
            centerPtn = { centerPtn.first+e.x(), centerPtn.second+e.y()} ;
        }
        centerPtn.first /= edges.size();
        centerPtn.second /= edges.size();
        if(edges.size() > 3 )
            for(auto& e: edges){
                pedges.append({centerPtn.first,centerPtn.second});
                pedges.append({e.x(),e.y()});
            }
    }
}


QPolygonF const &PolygonTestConvexPartitioning::getPolyline() { return polyline; }
QVector<QPointF> const &PolygonTestConvexPartitioning::getEdges(){ return pedges; }

