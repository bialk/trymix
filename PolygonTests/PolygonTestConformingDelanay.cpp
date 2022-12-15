#include "PolygonTestConformingDelanay.h"

#include <CGAL/Partition_traits_2.h>
#include <CGAL/partition_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/random_convex_set_2.h>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h>
#include <CGAL/ch_graham_andrew.h>

#include <cassert>
#include <list>

typedef CGAL::Creator_uniform_2<int, Point_2>               Creator;
typedef CGAL::Random_points_in_square_2<Point_2, Creator>   Point_generator;

typedef CGAL::Triangulation_vertex_base_2<K> Vb;
typedef CGAL::Delaunay_mesh_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
typedef CGAL::Constrained_Delaunay_triangulation_2<K,Tds> CDT;
typedef CDT::Point Point;
typedef CDT::Vertex_handle Vertex_handle;
typedef CGAL::Delaunay_mesh_size_criteria_2<CDT> Criteria;


constexpr double M_PI = 3.14159265358979323846;


PolygonTestConformingDelanay::PolygonTestConformingDelanay(int polygonSize, float meshCellSize)
{
    Polygon_2    random_points;
    CGAL::random_polygon_2(polygonSize, std::back_inserter(polygon),
                               Point_generator(600));

#ifdef off
   CGAL::random_polygon_2(4, std::back_inserter(polygon),
                               Point_generator(60));
   CGAL::ch_graham_andrew( random_points.begin(), random_points.end(), std::back_inserter(polygon));
   CGAL::random_convex_set_2(polygonSize, std::back_inserter(polygon),
                              Point_generator(600));
#endif

#ifdef off
   //circle test example
   for(auto i=0; i< polygonSize; ++i){
      polygon.push_back(Point_2(600*sin(-i*2.*M_PI/polygonSize),600*cos(-i*2.*M_PI/polygonSize)));
   }
#endif

    for(auto& i: polygon){
        polyline.append({i.x(), i.y()});
    }
    polyline.append(polyline.first());

    CDT cdt;

    auto prevPoint = polygon[polygon.size()-1];
    for(auto& p: polygon){
        //cdt.insert(p);
        cdt.insert_constraint(prevPoint,p);
        prevPoint = p;
    }


    CGAL::make_conforming_Delaunay_2(cdt);
    //CGAL::make_conforming_Gabriel_2(cdt);
    //CGAL::step_by_step_conforming_Delaunay_2(cdt);
    //CGAL::refine_Delaunay_mesh_2(cdt, Criteria(0.125, 0.5));
    CGAL::refine_Delaunay_mesh_2(cdt, Criteria(0.125, meshCellSize));

    std::cout << "total edges: " <<  cdt.finite_edges().size() << std::endl;
    for(auto &e: cdt.finite_edges()){
        if(!e.first->is_in_domain())
            continue;
        auto v1 = e.first->vertex((e.second + 1) % 3);
        auto v2 = e.first->vertex((e.second + 2) % 3);
        pedges.append({v1->point().x(),v1->point().y()});
        pedges.append({v2->point().x(),v2->point().y()});
    }

    return;
}


QPolygonF const &PolygonTestConformingDelanay::getPolyline() { return polyline; }
QVector<QPointF> const &PolygonTestConformingDelanay::getEdges(){ return pedges; }

