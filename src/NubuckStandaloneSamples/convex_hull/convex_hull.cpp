/*
convex_hull.cpp
*/

#include <Nubuck\nb_common.h>
#include <LEDA\geo\d3_hull.h>

using namespace ::nb;
using namespace ::OP;
using namespace ::leda;

struct ConvexHull : Operator {

    void Register(Invoker& invoker) override {
        nubuck().add_menu_item(nubuck().object_menu(), "Convex Hull Sample", invoker);
    }

    bool Invoke() override {
        mesh M = nubuck().first_selected_mesh();

        if(nil == M) {
            nubuck().log_printf("no input mesh selected.\n");
            return false;
        }

        NbGraph& G = nubuck().graph_of(M);
        list<d3_rat_point> L;
        node v;
        forall_nodes(v, G) {
            L.push(G.position_of(v));
        }

        NbGraph H;
        CONVEX_HULL(L, H);
        H.compute_faces();

        nubuck().set_graph(M, H);

        nubuck().set_mesh_render_mode(M, Nubuck::RenderMode::ALL);

        return true;
    }

    void Finish() override {
        // nothing to do here
    }

};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CREATE_OPERATOR(ConvexHull), NULL);
}
