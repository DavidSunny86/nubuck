#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\standard_algorithm.h>
#include "d3_delaunay.h"

typedef leda::d3_rat_point point_t;

struct Simplex {
    leda::node          verts[4];
    leda::rat_vector    localPos[4];
    leda::rat_vector    center;
};

struct Globals {
    leda::list<point_t>             L; // input points
    leda::d3_simpl_complex          C;
    leda::list<leda::sc_simplex>    trhull;
    point_t                         lastp;
    leda::sc_simplex                last;
    int                             counter;

    nb::geometry                    trGeom;
    nb::geometry                    trCloud;
};

class D3_Delaunay_Panel : public OP::ALG::StandardAlgorithmPanel {
};

/*
===============================================================================
    connects new point with current triangulation
===============================================================================
*/
struct PhaseT0 : OP::ALG::Phase {
    Globals& g;

    leda::sc_vertex PV;
    leda::list<leda::sc_simplex> Trl;
    leda::list<leda::Simpl_Triang> Border;
    leda::queue<leda::Simpl_Triang> TQ;
    point_t p;

    enum StepMode {
        FIND_TRIANGLES,
        ADD_SIMPLICES
    };
    StepMode stepMode;

    PhaseT0(Globals& g);

    StepRet::Enum StepFindTriangles();
    StepRet::Enum StepAddSimplices();

    StepRet::Enum Step() override;
};

PhaseT0::PhaseT0(Globals& g) : g(g), stepMode(FIND_TRIANGLES) { }

void SetColor(leda::nb::RatPolyMesh& mesh, leda::sc_simplex simplex, const R::Color& color) {
    if(simplex->meshInf.isValid) {
        mesh.set_color(simplex->GetMeshFace(1), color);
        mesh.set_color(simplex->GetMeshFace(2), color);
        mesh.set_color(simplex->GetMeshFace(3), color);
        mesh.set_color(simplex->GetMeshFace(4), color);
    }

    std::cout << mesh.face_of(simplex->meshInf.fcycles[0])->id() << std::endl;
    std::cout << mesh.face_of(simplex->meshInf.fcycles[1])->id() << std::endl;
    std::cout << mesh.face_of(simplex->meshInf.fcycles[2])->id() << std::endl;
    std::cout << mesh.face_of(simplex->meshInf.fcycles[3])->id() << std::endl;
}

PhaseT0::StepRet::Enum PhaseT0::StepFindTriangles() {
    p = g.L.pop();

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(g.trCloud);
    cloudMesh.set_color(cloudMesh.first_node(), R::Color::Yellow);

    find_visible_triangles(g.C,Trl,Border,g.last,p,g.counter,g.trhull);

    g.C.compute_mesh();
    g.C.set_scale();

    // colorize visible hull triangles
    assert(0 != Trl.size());
    leda::list_item it;
    forall_items(it, Trl) {
        assert(0 == g.C.vertex_d(Trl[it]));

        leda::sc_simplex adjSimp = g.C.opposite_simplex(Trl[it], 4);
        assert(!g.C.get_hull_info(adjSimp));

        const int vertIdx = adjSimp->FindLinkVertex(Trl[it]);
        assert(Trl[it] == g.C.opposite_simplex(adjSimp, vertIdx));
        nubuck().poly_mesh(g.trGeom).set_color(adjSimp->GetMeshFace(vertIdx), R::Color::Red);
    }

    stepMode = ADD_SIMPLICES;

    return StepRet::CONTINUE;
}

PhaseT0::StepRet::Enum PhaseT0::StepAddSimplices() {
    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(g.trCloud);
    cloudMesh.del_node(cloudMesh.first_node());

    TQ.clear();
    g.trhull.clear();

    PV=g.C.add_vertex(p);
    g.last=triangulate_new(g.C,Trl,Border,PV,TQ,g.trhull);

    assert(g.C.get_hull_info(g.last));

    g.C.compute_mesh();
    g.C.set_scale();

    stepMode = FIND_TRIANGLES;

    if(g.L.empty()) return StepRet::DONE;
    return StepRet::CONTINUE;
}

PhaseT0::StepRet::Enum PhaseT0::Step() {
    nubuck().log_printf("PhaseT0: stepping\n");
    if(FIND_TRIANGLES == stepMode) return StepFindTriangles();
    else return StepAddSimplices();
}

/*
===============================================================================
    create initial tetrahedron
===============================================================================
*/
struct PhaseInit : OP::ALG::Phase {
    Globals& g;

    PhaseInit(Globals& g);

    StepRet::Enum       Step() override;
    GEN::Pointer<Phase> NextPhase() override;
};

PhaseInit::PhaseInit(Globals& g) : g(g) { }

PhaseInit::StepRet::Enum PhaseInit::Step() {
    nubuck().log_printf("PhaseInit: stepping\n");

    g.last = leda::comp_initial_tr(g.L, g.C, g.lastp, g.trhull);

    g.C.compute_mesh();
    g.C.set_scale();

    // create cloud geometry
    g.trCloud = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(g.trCloud, Nubuck::RenderMode::NODES);
    nubuck().set_geometry_name(g.trCloud, "input cloud");

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(g.trCloud);
    leda::list_item it;
    forall_items(it, g.L) {
        leda::node v = cloudMesh.new_node();
        cloudMesh.set_position(v, g.L[it]);
    }

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> PhaseInit::NextPhase() {
    return GEN::MakePtr(new PhaseT0(g));
}

class D3_Delaunay : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;
protected:
    const char*     GetName() const override;
    OP::ALG::Phase* Init() override;
};

const char* D3_Delaunay::GetName() const {
    return "Delaunay Triangulation 3D";
}

OP::ALG::Phase* D3_Delaunay::Init() {
    nb::geometry inputGeom = nubuck().first_selected_geometry();
    if(0 == inputGeom) {
        nubuck().log_printf("no input geometry entity selected.\n");
        return 0;
    }

    // clear previous complex
    _g.C.clear();

    // create list of input points from first selected geometry entity
    _g.L.clear();
    const leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(inputGeom);
    leda::node v;
    forall_nodes(v, mesh) {
        _g.L.push(mesh.position_of(v));
    }
    _g.L.sort();
    _g.L.unique();

    // create triangulation geometry
    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;

    _g.trGeom = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(_g.trGeom, renderAll);

    // link complex to mesh
    _g.C.SetMesh(&nubuck().poly_mesh(_g.trGeom));

    return new PhaseInit(_g);
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new D3_Delaunay_Panel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D3_Delaunay;
}