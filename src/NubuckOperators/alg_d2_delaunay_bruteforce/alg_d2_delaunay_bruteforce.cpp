#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"
#include "phase0.h"

class D2_Delaunay_BruteForce : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;
protected:
    const char*     GetName() const override;
    OP::ALG::Phase* Init(const Nubuck& nb) override;
};

namespace {

/*
====================
BackFaceEdgeXY
    returns an edge incident to the backface. assumes 'mesh'
    lies in xy plane and faces +z direction
====================
*/
leda::edge BackFaceEdgeXY(const leda::nb::RatPolyMesh& mesh) {
    // find hull vertex
    leda::node hullVert = mesh.first_node();
    leda::node v;
    forall_nodes(v, mesh) {
        if(0 < leda::compare(mesh.position_of(hullVert), mesh.position_of(v))) {
            hullVert = v;
        }
    }

    // find hull edge
    leda::edge hullEdge = mesh.first_out_edge(hullVert);
    do {
        leda::edge succ = mesh.cyclic_adj_pred(hullEdge);

        const leda::d3_rat_point& p0 = mesh.position_of(hullVert);
        const leda::d3_rat_point& p1 = mesh.position_of(leda::target(hullEdge));
        const leda::d3_rat_point& p2 = mesh.position_of(leda::target(succ));

        if(0 <= leda::orientation_xy(p0, p1, p2)) break;

        hullEdge = succ;

    } while(true);

    return mesh.reversal(hullEdge);
}

IGeometry* CreateCircle(IWorld* world) {
    IGeometry* geom = world->CreateGeometry();

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    leda::list<leda::d3_rat_point> L;
    leda::random_d3_rat_points_on_circle(500, 1000, L);
    leda::list_item it;
    forall_items(it, L) {
        L[it] = leda::rational(1, 1000) * L[it].to_vector();
    }
    leda::D3_HULL(L, mesh);
    mesh.compute_faces();

    leda::edge bfaceEdge = BackFaceEdgeXY(mesh);
    mesh.set_visible(mesh.face_of(bfaceEdge), false);

    leda::nb::set_color(mesh, R::Color::Red);

    geom->SetRenderMode(IGeometry::RenderMode::FACES);
    geom->SetName("circle");
    geom->Hide();

    return geom;
}

} // unnamed namespace

const char* D2_Delaunay_BruteForce::GetName() const {
    return "Delaunay Triangulation (brute force)";
}

OP::ALG::Phase* D2_Delaunay_BruteForce::Init(const Nubuck& nb) {
    _g.nb = nb;

    std::vector<IGeometry*> geomSel = _g.nb.world->GetSelection()->GetList();
    if(geomSel.empty()) {
        _g.nb.log->printf("ERROR - no input mesh selected.\n");
        return NULL;
    }

    _g.inputGeom = geomSel[0];

    _g.circle = CreateCircle(_g.nb.world);

    return new Phase0(_g);
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new OP::ALG::StandardAlgorithmPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D2_Delaunay_BruteForce;
}