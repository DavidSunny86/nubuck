#include "shared.h"
#include "mantle.h"
#include "phase3.h"

static void DeleteYellowEdges() {
    mesh_t& G = GetG();
    leda::edge e;
    forall_edges(e, G) {
        if(YELLOW == g.edgeColors[e])
            G.del_edge(e);
    }
}

static void DeleteRedVertices() {
    mesh_t& G = GetG();
    leda::node v;
    forall_nodes(v, G) {
        if(RED == g.nodeColors[v])
            G.del_node(v);
    }
}

static void DeleteSelfEdges(Conf& conf) {
    mesh_t& G = GetG();
    // this is important!
    if(conf.self) {
        G.del_edge(G.reversal(conf.self));
        G.del_edge(conf.self);
    }
}

void Phase3::Enter() {
    std::cout << "entering Phase3" << std::endl;
}

Phase3::StepRet::Enum Phase3::Step() {
    DeleteYellowEdges();
    DeleteRedVertices();
    DeleteSelfEdges(g.P0);
    DeleteSelfEdges(g.P1);

    mesh_t& G = GetG();
    leda::edge e;
    forall_edges(e, G) G.set_unmasked(e);
    G.compute_faces();

    NB::DestroyMesh(g.geom_activeEdge);
    NB::DestroyMesh(g.geom_activeEdge0);
    NB::DestroyMesh(g.geom_activeEdge1);
    NB::DestroyMesh(g.geom_suppEdge);
    g.mantle->Destroy();

    return StepRet::DONE;
}