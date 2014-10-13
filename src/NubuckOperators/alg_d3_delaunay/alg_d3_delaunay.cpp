#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\animation\animation.h>
#include <Nubuck\animation\move_vertex_anim.h>
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
    leda::queue<leda::Simpl_Triang> TQ;
    leda::sc_vertex                 PV;
    leda::list<leda::sc_simplex> Trl;

    nb::geometry                    trGeom;
    nb::geometry                    trCloud;

    leda::sc_simplex                rel; // head of list of relevant simplices

    // reused phases
    GEN::Pointer<OP::ALG::Phase>    phaseT0;
    GEN::Pointer<OP::ALG::Phase>    phaseTI;
    GEN::Pointer<OP::ALG::Phase>    phaseIdle;
};

void SetSimplexColor(leda::nb::RatPolyMesh& mesh, leda::sc_simplex simplex, const R::Color& color) {
    if(simplex->meshInf.isValid) {
        mesh.set_color(simplex->GetMeshFace(1), color);
        mesh.set_color(simplex->GetMeshFace(2), color);
        mesh.set_color(simplex->GetMeshFace(3), color);
        mesh.set_color(simplex->GetMeshFace(4), color);
    }
}

struct MoveSimplicesAnimation : A::Animation {
public:
    Globals&                g;

    double                  v0, v1;
    float                   time;
    float                   duration;

    MoveSimplicesAnimation(Globals& g, double from, double to, float duration)
        : g(g)
        , v0(from)
        , v1(to)
        , time(0.0f)
        , duration(duration)
    { }
protected:
    bool DoMove(float secsPassed) override {
        double l = time / duration;
        double scale = (1.0f - l) * v0 + l * v1;

        //std::cout << "DoMove! " << s << std::endl;

        g.C.set_scale(scale);

        leda::node v;
        leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(g.trGeom);
        forall_nodes(v, mesh) mesh.set_color(v, mesh.color_of(v));

        time += secsPassed;

        return duration <= time;
    }
};

struct ColorSimplicesAnimation : A::Animation {
public:
    Globals&            g;
    leda::sc_simplex    simp;

    R::Color            c0;
    R::Color            c1;
    float               time;
    float               duration;

    ColorSimplicesAnimation(Globals& g, leda::sc_simplex simp, const R::Color& c0, const R::Color& c1, float duration)
        : g(g)
        , simp(simp)
        , c0(c0)
        , c1(c1)
        , time(0.0f)
        , duration(duration)
    { }
protected:
    bool DoMove(float secsPassed) override {
        float l = time / duration;
        R::Color c = R::Lerp(c0, c1, l);
        SetSimplexColor(nubuck().poly_mesh(g.trGeom), simp, c);
        time += secsPassed;
        return duration <= time;
    }
};

class D3_Delaunay_Panel : public OP::ALG::StandardAlgorithmPanel {
};

void BuildRelevantMesh(Globals& g, bool update = true) {
    // build list of relevant simplices.
    leda::list_item it;
    const leda::list<leda::sc_simplex>& Sil = g.C.all_simplices();
    if(update) {
        g.rel = NULL;
        forall_items(it, Sil) {
            Sil[it]->adj = 0;
            Sil[it]->in_rel = false;
        }
        forall_items(it, Sil) {
            bool c0 = g.PV == g.C.vertex_a(Sil[it]);
            bool c1 = g.PV == g.C.vertex_b(Sil[it]);
            bool c2 = g.PV == g.C.vertex_c(Sil[it]);
            bool c3 = g.PV == g.C.vertex_d(Sil[it]);
            if(c0 || c1 || c2 || c3) {
                // simplex adjacient to ith point

                leda::sc_simplex simp = Sil[it];
                if(simp->in_rel) continue;
                simp->rel_prev = NULL;
                simp->rel_next = g.rel;
                if(g.rel) g.rel->rel_prev = simp;
                g.rel = simp;
                simp->in_rel = true;

                leda::sc_simplex other;
                if(other = g.C.link_abc(simp)) other->adj++;
                if(other = g.C.link_abd(simp)) other->adj++;
                if(other = g.C.link_acd(simp)) other->adj++;
                if(other = g.C.link_bcd(simp)) other->adj++;
            }
        }
        forall_items(it, Sil) {
            if(0 < Sil[it]->adj && !Sil[it]->in_rel) {
                leda::sc_simplex simp = Sil[it];
                simp->rel_prev = NULL;
                simp->rel_next = g.rel;
                if(g.rel) g.rel->rel_prev = simp;
                g.rel = simp;
                simp->in_rel = true;
            }
        }
    }
    leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(g.trGeom);

    leda::face f;
    forall_faces(f, mesh) {
        mesh.set_visible(f, false);
    }

    leda::sc_simplex Sit = g.rel;
    while(Sit) {
        if(Sit->meshInf.isValid) {
            mesh.set_visible(Sit->GetMeshFace(1), true);
            mesh.set_visible(Sit->GetMeshFace(2), true);
            mesh.set_visible(Sit->GetMeshFace(3), true);
            mesh.set_visible(Sit->GetMeshFace(4), true);
        }
        Sit = Sit->rel_next;
    }
}


/*
===============================================================================
    idle
===============================================================================
*/
struct PhaseIdle : OP::ALG::Phase {
    void Enter() override {
		std::cout << "entering idle phase" << std::endl;
	}

    StepRet::Enum Step() override {
		return StepRet::DONE;
	}

    bool IsWall() const { return true; }
};

/*
===============================================================================
    flipping
===============================================================================
*/
struct PhaseTI : OP::ALG::Phase {
    Globals& g;

    leda::Simpl_Triang Trl;
    leda::sc_simplex S1,S2;
    int wert;

    bool isExploded;

    enum StepMode {
        SEARCH, FLIP
    };
    StepMode stepMode;

    GEN::Pointer<OP::ALG::Phase> nextPhase;

    PhaseTI(Globals& g);

    StepRet::Enum StepSearch();
    StepRet::Enum StepFlip();

    StepRet::Enum Step() override;
    GEN::Pointer<OP::ALG::Phase> NextPhase() override;
};

PhaseTI::PhaseTI(Globals& g) : g(g), isExploded(false), stepMode(SEARCH) { }

PhaseTI::StepRet::Enum PhaseTI::StepSearch() {
    while (! g.TQ.empty()){
        Trl= g.TQ.pop();
        // still there ?
        S1=Trl.S1; S2=Trl.S2;
        if (S1!=NULL && S2!=NULL && g.C.complex_of(S1)==&g.C && g.C.complex_of(S2)==&g.C){

            if (g.C.flippable(S1,S2,Trl)){ // common triangle flippable ?
                nubuck().log_printf("found flippable face ...\n");

                g.C.compute_mesh(); // triggers rebuild
                BuildRelevantMesh(g);

                MoveSimplicesAnimation anim0(g, 1.0f, 2.0f, 2.0f); // scope!
                if(!isExploded) {
                    anim0.PlayUntilIsDone();
                    isExploded = true;
                } else {
                    g.C.set_scale(leda::rational(2, 1));
                }

                ColorSimplicesAnimation anim1(g, S1, R::Color::White, R::Color::Blue, 2.0f);
                ColorSimplicesAnimation anim2(g, S2, R::Color::White, R::Color::Blue, 2.0f);
                anim1.PlayUntilIsDone();
                anim2.PlayUntilIsDone();

                nubuck().wait_for_animations();

                stepMode = FLIP;
                return StepRet::CONTINUE;
            }

        }
    }

    if(isExploded) {
        MoveSimplicesAnimation anim0(g, 2.0f, 1.0f, 2.0f);
        anim0.PlayUntilIsDone();
        nubuck().wait_for_animations();
        isExploded = false;
    }

    return StepRet::DONE;
}

PhaseTI::StepRet::Enum PhaseTI::StepFlip() {
    nubuck().log_printf("flipping ...\n");

    // TODO: set color of flipped simplices
    wert=g.C.flip(S1,S2,g.TQ,Trl,g.PV);

    g.C.compute_mesh();
    BuildRelevantMesh(g);
    g.C.set_scale(leda::rational(2, 1));

    stepMode = SEARCH;
    return StepRet::CONTINUE;
}

PhaseTI::StepRet::Enum PhaseTI::Step() {
    nubuck().log_printf("PhaseTI: stepping.\n");

    if(SEARCH == stepMode) return StepSearch();
    else return StepFlip();
}

GEN::Pointer<OP::ALG::Phase> PhaseTI::NextPhase() {
    return g.phaseT0;
}

/*
===============================================================================
    connects new point with current triangulation
===============================================================================
*/
struct PhaseT0 : OP::ALG::Phase {
    Globals& g;

    leda::list<leda::Simpl_Triang> Border;
    point_t p;

    enum StepMode {
        FIND_TRIANGLES,
        ADD_SIMPLICES
    };
    StepMode stepMode;

    PhaseT0(Globals& g);

    StepRet::Enum StepFindTriangles();
    StepRet::Enum StepAddSimplices();

    void Enter() override;

    StepRet::Enum Step() override;
    GEN::Pointer<Phase> NextPhase() override;
};

PhaseT0::PhaseT0(Globals& g) : g(g), stepMode(FIND_TRIANGLES) { }

PhaseT0::StepRet::Enum PhaseT0::StepFindTriangles() {
    nubuck().log_printf("PhaseT0: StepFindTriangles\n");

    if(g.L.empty()) {
        nubuck().log_printf("PhaseT0: L is empty, done\n");
        return StepRet::CONTINUE;
    }

    p = g.L.pop();

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(g.trCloud);
    cloudMesh.set_color(cloudMesh.first_node(), R::Color::Yellow);

    g.Trl.clear();
    find_visible_triangles(g.C,g.Trl,Border,g.last,p,g.counter,g.trhull);

    g.C.compute_mesh();

    // colorize visible hull triangles
    assert(0 != g.Trl.size());
    leda::list_item it;
    forall_items(it, g.Trl) {
        assert(0 == g.C.vertex_d(g.Trl[it]));

        leda::sc_simplex adjSimp = g.C.opposite_simplex(g.Trl[it], 4);
        assert(!g.C.get_hull_info(adjSimp));

        const int vertIdx = adjSimp->FindLinkVertex(g.Trl[it]);
        assert(g.Trl[it] == g.C.opposite_simplex(adjSimp, vertIdx));
        nubuck().poly_mesh(g.trGeom).set_color(adjSimp->GetMeshFace(vertIdx), R::Color::Red);

        leda::sc_simplex simp = adjSimp;
        if(simp->in_rel) continue;
        simp->rel_prev = NULL;
        simp->rel_next = g.rel;
        if(g.rel) g.rel->rel_prev = simp;
        g.rel = simp;
        simp->in_rel = true;
    }

    BuildRelevantMesh(g, false);

    stepMode = ADD_SIMPLICES;

    return StepRet::CONTINUE;
}

PhaseT0::StepRet::Enum PhaseT0::StepAddSimplices() {
    nubuck().log_printf("PhaseT0: StepAddSimplices\n");

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(g.trCloud);
    cloudMesh.del_node(cloudMesh.first_node());

    g.TQ.clear();
    g.trhull.clear();

    g.PV=g.C.add_vertex(p);
    g.last=triangulate_new(g.C,g.Trl,Border,g.PV,g.TQ,g.trhull);

    assert(g.C.get_hull_info(g.last));

    // triangulate_new ,expands' hull triangles Trl to full simplices

    g.C.compute_mesh();
    BuildRelevantMesh(g);

    return StepRet::DONE;
}

void PhaseT0::Enter() {
    stepMode = FIND_TRIANGLES;
}

PhaseT0::StepRet::Enum PhaseT0::Step() {
    if(FIND_TRIANGLES == stepMode) return StepFindTriangles();
    else return StepAddSimplices();
}

GEN::Pointer<OP::ALG::Phase> PhaseT0::NextPhase() {
    return g.phaseTI;
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
    return g.phaseT0;
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

    // setup phases
    _g.phaseT0 = GEN::MakePtr(new PhaseT0(_g));
    _g.phaseTI = GEN::MakePtr(new PhaseTI(_g));
    _g.phaseIdle = GEN::MakePtr(new PhaseIdle);

    // clear previous complex
    _g.C.clear();

    _g.rel = 0;

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