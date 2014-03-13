#include "mantle.h"

void Mantle::Init(const leda::node v0, const leda::node v1, const leda::node v2) {
    _geom = _nb.world->CreateGeometry(); 
    _geom->SetName("Mantle");
    _geom->SetRenderMode(IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
    leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
    leda::edge e = mesh.make_triangle(_subj[v0], _subj[v1], _subj[v2]);
    _geom->Update();
    _cand[0] = mesh.face_cycle_pred(e);
    _cand[1] = mesh.face_cycle_succ(e);
    _last = v2;
}

Mantle::Mantle(const Nubuck& nb, const mesh_t& subj) : _nb(nb), _subj(subj), _geom(NULL) {
    _nmap.init(_subj, NULL);
}

void Mantle::AddTriangle(const leda::node v0, const leda::node v1, const leda::node v2) {
    if(!_geom) {
        Init(v0, v1, v2);
        return;
    }

    leda::edge e = NULL;
    if(v0 == _last) e = _cand[1];
    else e = _cand[0];

    leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();

    leda::edge u = mesh.split(e);
    mesh.set_position(leda::source(u), _subj[v2]);
    mesh.split(mesh.face_cycle_succ(u), e);

    _cand[0] = u;
    _cand[1] = e;
    _last = v2;

    _geom->Update();
}

void Mantle::Destroy() { _geom->Destroy(); }

