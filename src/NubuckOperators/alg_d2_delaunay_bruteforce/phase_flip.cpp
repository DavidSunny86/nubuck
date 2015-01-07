#include <LEDA\geo\rat_circle.h>
#include <Nubuck\polymesh.h>
#include "phase_flip.h"

Phase_Flip::Quadrilateral::Quadrilateral()
    : e(NULL)
    , r(NULL)
    , e1(NULL)
    , e2(NULL)
    , e3(NULL)
    , e4(NULL)
{ }

Phase_Flip::Quadrilateral::Quadrilateral(const leda::nb::RatPolyMesh& mesh, leda::edge e)
    : e(e)
{
    r   = mesh.reversal(e);
    e1  = mesh.face_cycle_succ(r);
    e2 	= mesh.face_cycle_succ(e1);
    e3 	= mesh.face_cycle_succ(e);
    e4 	= mesh.face_cycle_succ(e3);

    p0 = mesh.position_of(leda::source(e)).project_xy();
    p1 = mesh.position_of(leda::target(e1)).project_xy();
    p2 = mesh.position_of(leda::target(e)).project_xy();
    p3 = mesh.position_of(leda::target(e3)).project_xy();
}

bool Phase_Flip::Quadrilateral::IsNull() const {
    if(NULL == e) {
        assert(
            NULL == r  &&
            NULL == e1 &&
            NULL == e2 &&
            NULL == e3 &&
            NULL == e4);
    }
    return NULL == e;
}

void Phase_Flip::Enter() {
    NB::LogPrintf("entering phase 'flip'\n");

    leda::nb::RatPolyMesh& mesh = NB::GetGraph(_g.delaunay);

    leda::edge e;
    forall_edges(e, mesh) {
        if(Color::BLACK == GetColor(mesh, e)) {
            _S.insert(e);
        }
    }
}

void ApplyEdgeColor(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    static R::Color colors[] = {
        R::Color::Black,
        R::Color::Blue
    };
    mesh.set_color(e, colors[GetColor(mesh, e)]);
}

Phase_Flip::StepRet::Enum Phase_Flip::StepSearch() {
    typedef leda::rat_point point2_t;

    leda::nb::RatPolyMesh& mesh = NB::GetGraph(_g.delaunay);

    // reset colors of previous flip
    if(!_q.IsNull()) {
        ApplyEdgeColor(mesh, _q.e);
        ApplyEdgeColor(mesh, _q.r);
        ApplyEdgeColor(mesh, _q.e1);
        ApplyEdgeColor(mesh, _q.e2);
        ApplyEdgeColor(mesh, _q.e3);
        ApplyEdgeColor(mesh, _q.e4);

        // color chull mesh
        leda::nb::RatPolyMesh& chullMesh = NB::GetGraph(_g.chull);
        chullMesh.set_color(_g.emap[_q.e ], mesh.color_of(_q.e ));
        chullMesh.set_color(_g.emap[_q.r ], mesh.color_of(_q.r ));
        chullMesh.set_color(_g.emap[_q.e1], mesh.color_of(_q.e1));
        chullMesh.set_color(_g.emap[_q.e2], mesh.color_of(_q.e2));
        chullMesh.set_color(_g.emap[_q.e3], mesh.color_of(_q.e3));
        chullMesh.set_color(_g.emap[_q.e4], mesh.color_of(_q.e4));
    }

    while(!_S.empty()) {
        leda::edge e = _S.choose();
        _S.del(e);

        Quadrilateral q(mesh, e);

        const int orient_130 = leda::orientation(q.p1, q.p3, q.p0);
        const int orient_132 = leda::orientation(q.p1, q.p3, q.p2);

        const bool isStrictlyConvex = 0 != orient_130 && 0 != orient_132 && orient_130 != orient_132;

        const leda::rat_circle C(q.p0, q.p1, q.p2);

        const bool inCircle = C.inside(q.p3);

        if(isStrictlyConvex && inCircle) {
            const leda::circle Cf(C.to_circle());
            NB::SetMeshPosition(_g.circle, NB::GetMeshPosition(_g.delaunay) + M::Vector3(Cf.center().xcoord(), Cf.center().ycoord(), 0.0f));
            NB::SetMeshScale(_g.circle, static_cast<float>(Cf.radius()) * M::Vector3(1.0f, 1.0f, 1.0f));
            NB::ShowMesh(_g.circle);

            // color delaunay mesh
            mesh.set_color(q.e1, R::Color::Yellow);
            mesh.set_color(q.e2, R::Color::Yellow);
            mesh.set_color(q.e3, R::Color::Yellow);
            mesh.set_color(q.e4, R::Color::Yellow);

            // color chull mesh
            leda::nb::RatPolyMesh& chullMesh = NB::GetGraph(_g.chull);
            chullMesh.set_color(_g.emap[q.e1], R::Color::Yellow);
            chullMesh.set_color(_g.emap[q.e2], R::Color::Yellow);
            chullMesh.set_color(_g.emap[q.e3], R::Color::Yellow);
            chullMesh.set_color(_g.emap[q.e4], R::Color::Yellow);


            _q = q;

            _stepMode = StepMode::PERFORM_FLIP;
            return StepRet::CONTINUE;
        }
    }

    NB::HideMesh(_g.circle);

    return StepRet::DONE;
}

Phase_Flip::StepRet::Enum Phase_Flip::StepPerformFlip() {
    leda::nb::RatPolyMesh& mesh = NB::GetGraph(_g.delaunay);
    leda::nb::RatPolyMesh& chullMesh = NB::GetGraph(_g.chull);

    // flip delaunay edge
    mesh.move_edge(_q.e, _q.e2, leda::source(_q.e4));
    mesh.move_edge(_q.r, _q.e4, leda::source(_q.e2));

    // flip chull edge
    mesh.move_edge(_g.emap[_q.e], _g.emap[_q.e2], leda::source(_g.emap[_q.e4]));
    mesh.move_edge(_g.emap[_q.r], _g.emap[_q.e4], leda::source(_g.emap[_q.e2]));

    _q = Quadrilateral(mesh, _q.e);

    if(Color::BLACK == GetColor(mesh, _q.e1)) _S.insert(_q.e1);
    if(Color::BLACK == GetColor(mesh, _q.e2)) _S.insert(_q.e2);
    if(Color::BLACK == GetColor(mesh, _q.e3)) _S.insert(_q.e3);
    if(Color::BLACK == GetColor(mesh, _q.e4)) _S.insert(_q.e4);

    const leda::rat_circle C(_q.p0, _q.p1, _q.p2);

    const leda::circle Cf(C.to_circle());
    NB::SetMeshPosition(_g.circle, NB::GetMeshPosition(_g.delaunay) + M::Vector3(Cf.center().xcoord(), Cf.center().ycoord(), 0.0f));
    NB::SetMeshScale(_g.circle, static_cast<float>(Cf.radius()) * M::Vector3(1.0f, 1.0f, 1.0f));
    NB::ShowMesh(_g.circle);

    // force rebuild
    mesh.compute_faces();
    chullMesh.compute_faces();
    chullMesh.set_visible(chullMesh.face_of(_g.emap[_g.hullEdge]), false);

    _stepMode = StepMode::SEARCH;
    return StepRet::CONTINUE;
}

Phase_Flip::StepRet::Enum Phase_Flip::Step() {
    if(StepMode::SEARCH == _stepMode) return StepSearch();
    else return StepPerformFlip();
}