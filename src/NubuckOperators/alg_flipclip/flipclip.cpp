#include <Nubuck\polymesh.h>
#include "flipclip.h"

const char* FlipClip::GetName() const { return "Flip & Clip"; }

OP::ALG::Phase* FlipClip::Init(const Nubuck& nb) {
    _g.nb = nb;

    // choose first selected geometry as input
    ISelection* sel = _g.nb.world->GetSelection();
    std::vector<IGeometry*> geomSel = sel->GetList();
    if(geomSel.empty()) {
        _g.nb.log->printf("ERROR - no input object selected.\n");
        return new OP::ALG::Phase;
    }
    _g.geom = geomSel[0];

    const unsigned renderAll = IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES;
    _g.geom->SetRenderMode(renderAll);

    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    if(0 < mesh.number_of_edges()) {
        _g.nb.log->printf("deleting edges and faces of input mesh.\n");
        mesh.del_all_edges();
        mesh.del_all_faces();
    }

    _g.L[Side::FRONT] = mesh.all_nodes();

    _g.L[Side::BACK].clear();
    leda::node v;
    forall(v, _g.L[Side::FRONT]) {
        leda::node w = mesh.new_node();
        mesh.set_position(w, mesh.position_of(v).translate(0, 0, -5));
        _g.L[Side::BACK].push(w);
    }

    forall(v, _g.L[Side::FRONT]) {
        mesh.set_position(v, mesh.position_of(v).translate(0, 0, 5));
    }

    _g.side = Side::FRONT;

    _g.hullEdges[Side::FRONT] = _g.hullEdges[Side::BACK] = NULL;

    return new Phase_Init(_g);
}