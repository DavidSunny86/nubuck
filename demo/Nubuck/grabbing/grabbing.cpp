#include <Nubuck\nubuck.h>
#include <LEDA\geo\geo_alg.h>
#include "globals.h"
#include "phase0.h"

class Algorithm : public IAlgorithm {
private:
    typedef leda::rat_point point2_t;
    typedef leda::GRAPH<point2_t, int> graph2_t;

    Globals _globals;

    leda::list<point2_t> ToPointList2(const graph_t& G) {
        leda::list<point2_t> L;
        leda::node n;
        forall_nodes(n, G) L.push_back(point2_t(G[n].xcoord(), G[n].ycoord()));
        return L;
    }

    void FromProjection(const graph2_t& G2, graph_t& G3) {
        leda::node_array<leda::node> nmap(G2, NULL);

        leda::node n2;
        forall_nodes(n2, G2) {
            leda::node n3 = nmap[n2] = G3.new_node();
            G3[n3] = point_t(G2[n2].xcoord(), G2[n2].ycoord(), 0);
        }

        leda::edge e2;
        forall_edges(e2, G2) {
            G3.new_edge(nmap[leda::source(e2)], nmap[leda::target(e2)]); 
        }
    }

    void Prepare(graph_t& G) {
        leda::node n;

        // project all nodes on xy-plane. scale, too
        double s = 0.01;
        _globals.nb.log->printf("Scaling points by factor %f.\n", s);
        forall_nodes(n, G)
            G[n] = point_t(s * G[n].xcoord(), s * G[n].ycoord(), 0);
    }
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t& G) override {
        // we are expected to copy these parameters
        _globals.nb = nubuck;
        _globals.G  = G;

        Prepare(_globals.G);

        leda::list<point2_t> L(ToPointList2(_globals.G));
        graph2_t G2;
        leda::DELAUNAY_DIAGRAM(L, G2);
        _globals.G.clear();
        FromProjection(G2, _globals.G);

        _globals._delaunay = _globals.nb.world->CreatePolyhedron(_globals.G);

        return new Phase0(_globals);
    }

    bool Run(void) override { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}