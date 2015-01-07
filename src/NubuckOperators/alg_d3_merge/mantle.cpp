#include "mantle.h"

void Mantle::Init(const leda::node v0, const leda::node v1, const leda::node v2) {
    _geom = NB::CreateMesh();
    NB::SetMeshName(_geom, "Mantle");
    NB::SetMeshRenderMode(_geom, NB::RM_EDGES | NB::RM_FACES);
    leda::nb::RatPolyMesh& mesh = NB::GetGraph(_geom);
    leda::edge e = mesh.make_triangle(_subj[v0], _subj[v1], _subj[v2]);
    _cand[0] = mesh.face_cycle_pred(e);
    _cand[1] = mesh.face_cycle_succ(e);
    _last = v2;
}

Mantle::Mantle(const mesh_t& subj) : _subj(subj), _geom(NULL) {
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

    leda::nb::RatPolyMesh& mesh = NB::GetGraph(_geom);

    leda::edge u = mesh.split(e);
    mesh.set_position(leda::source(u), _subj[v2]);
    mesh.split(mesh.face_cycle_succ(u), e);

    _cand[0] = u;
    _cand[1] = e;
    _last = v2;
}

void Mantle::Destroy() { NB::DestroyMesh(_geom); }

