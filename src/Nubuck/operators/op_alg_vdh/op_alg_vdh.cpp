#include <maxint.h>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\animation\move_vertex_anim.h>
#include "op_alg_vdh.h"

using namespace leda;
using namespace NB;

namespace OP {

void VDH_Operator::Register(Invoker& invoker) {
    AddMenuItem(AlgorithmMenu(), "VDH Demo", invoker);
}

static Point3 ProjectXY(const Point3& p) {
    return Point3(p.xcoord(), p.ycoord(), 0);
}

// assumes z(p0) = z(p1) = z(p2) = 0
static bool IsFrontFace(Graph& G, face f) {
    edge e = G.first_face_edge(f);
    Point3 p0 = G.position_of(source(e));
    Point3 p1 = G.position_of(target(e));
    Point3 p2 = G.position_of(target(G.face_cycle_succ(e)));
    return 0 > orientation(p0, p1, p2, Point3(0, 0, 1));
}

VDH_Operator::VDH_Operator()
    : _cloudMesh(NULL)
    , _hullMesh(NULL)
{ }

bool VDH_Operator::Invoke() {
    node v, w;

    SetOperatorName("VDH Demo");

    _cloudMesh = FirstSelectedMesh();
    if(!_cloudMesh) {
        LogPrintf("no input mesh selected.\n");
        return false;
    }

    Graph& cloudGraph = GetGraph(_cloudMesh);
    cloudGraph.del_all_edges();
    cloudGraph.del_all_faces();
    cloudGraph.force_rebuild();

    // project all vertices onto XY plane
    forall_nodes(v, cloudGraph) {
        cloudGraph.set_position(v, ProjectXY(cloudGraph.position_of(v)));
    }

    // create convex hull mesh
    _hullMesh = CreateMesh();
    SetMeshRenderMode(_hullMesh, RM_ALL);
    list<Point3> L;
    forall_nodes(v, cloudGraph) {
        L.push(cloudGraph.position_of(v));
    }
    Graph& hullGraph = GetGraph(_hullMesh);
    CONVEX_HULL(L, hullGraph);
    hullGraph.compute_faces();

    // remove backface of convex hull
    face f = hullGraph.first_face();
    if(!IsFrontFace(hullGraph, f)) f = hullGraph.next_face(f);
    hullGraph.set_visible(f, false);

    edge e;
    forall_edges(e, hullGraph) {
        hullGraph.set_color(e, R::Color::Blue);
    }

    return true;
}

void VDH_Operator::Finish() {
    _cloudMesh = NULL;
    _hullMesh = NULL;
}

} // namespace OP