#include <LEDA\geo\d3_hull.h>
#include <Nubuck\nubuck.h>

class MyAlgorithm : public IAlgorithm {
public:
    typedef leda::d3_rat_point          point_t;
    typedef leda::GRAPH<point_t, int>   graph_t;

    IPhase* Init(const Nubuck& nubuck, const leda::GRAPH<leda::d3_rat_point, int>& G) {
        nubuck.log->printf("Hello World!\n");

        IPolyhedron* chull = nubuck.world->CreatePolyhedron();
        chull->SetName("Convex Hull");
        chull->SetRenderFlags(POLYHEDRON_RENDER_HULL | POLYHEDRON_RENDER_NODES | POLYHEDRON_RENDER_EDGES);

        leda::list<point_t> L;
        leda::node n;
        forall_nodes(n, G) L.append(G[n]);

        leda::CONVEX_HULL(L, chull->GetGraph());
        chull->Update();

        leda::edge face = chull->GetGraph().first_edge();
        chull->SetFaceColor(face, 1.0f, 0.0f, 0.0f); // color value in RGB

        return NULL;
    }

    bool Run(void) { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<MyAlgorithm>);
}