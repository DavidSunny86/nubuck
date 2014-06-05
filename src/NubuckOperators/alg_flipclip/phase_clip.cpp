#include <Nubuck\polymesh.h>
#include "phase_flip.h"
#include "phase_clip.h"

Phase_Clip::Phase_Clip(Globals& g) : _g(g) { }

void Phase_Clip::Enter() {
    _g.nb.log->printf("entering phase 'clip'\n");

    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    _L.clear();
    _rdeg.init(mesh, 0);
    leda::edge e;
    forall_edges(e, mesh) {
        if(Color::RED == mesh[e]) {
            leda::node v = leda::source(e);
            if(3 == ++_rdeg[v]) _L.push(v);
        }
    }

    _numClips = 0;
    _stepMode = StepMode::SEARCH;
}

Phase_Clip::StepRet::Enum Phase_Clip::StepSearch() {
    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    while(!_L.empty()) {
        _clipV = _L.pop();

        if(mesh.outdeg(_clipV) != _rdeg[_clipV]) continue;
        if(3 != _rdeg[_clipV]) {
            _g.nb.log->printf("WARNING - unhandled case rdeg[v] != 3!\n");
            continue;
        }


        // highlight neighbourhood of vertex v
        leda::nb::set_color(mesh, R::Color(1.0f, 1.0f, 1.0f, 0.2f));
        leda::edge e;
        forall_out_edges(e, _clipV) {
            mesh.set_color(mesh.face_of(e), R::Color::Red);
        }
        mesh.set_color(_clipV, R::Color::Yellow);

        _stepMode = StepMode::PERFORM_CLIP;
        return StepRet::CONTINUE;
    }

    _g.nb.log->printf("number of clips: %d\n", _numClips);
    return StepRet::DONE;
}

Phase_Clip::StepRet::Enum Phase_Clip::StepPerformClip() {
    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    leda::edge e;
    forall_out_edges(e, _clipV) {
        leda::edge b = mesh.face_cycle_succ(e); // boundary edge
        if(Color::BLACK == mesh[b]) {
            mesh[b] = mesh[mesh.reversal(b)] = Color::RED;
        }
    }

    mesh.del_node(_clipV);
    _numClips++;

    mesh.compute_faces();
    mesh.set_visible(mesh.face_of(_g.hullEdge), false);
    ApplyEdgeColors(mesh);

    _stepMode = StepMode::SEARCH;
    return StepRet::CONTINUE;
}

Phase_Clip::StepRet::Enum Phase_Clip::Step() {
    if(StepMode::SEARCH == _stepMode) {
        return StepSearch();
    } else {
        return StepPerformClip();
    }
}

GEN::Pointer<OP::ALG::Phase> Phase_Clip::NextPhase() {
    if(!_numClips) {
        return OP::ALG::Phase::NextPhase();
    } else {
        return GEN::MakePtr(new Phase_Flip(_g));
    }
}