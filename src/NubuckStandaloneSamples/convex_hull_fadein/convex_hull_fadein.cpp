/*
convex_hull_fadein.cpp
*/

#include <Nubuck\nb_common.h>
#include <LEDA\geo\d3_hull.h>

using namespace ::nb;
using namespace ::A;
using namespace ::M;
using namespace ::OP;
using namespace ::leda;

static float f(float x) {
    return -cosf(4.0f * x) / (2.0f * x) + 2.0f;
}

class FadeIn : public Animation {
private:
    mesh    M;
    float   duration;
    float   time;
protected:
    bool Animate() override {
        float s = Max(0.0f, f(time));
        nubuck().set_mesh_scale(M, Vector3(s, s, s));
        time += GetSecsPassed();
        return duration <= time;
    }
public:
    FadeIn(mesh M) : M(M), duration(4.0f), time(0.0f) { }
};

struct ConvexHull : Operator {

    mesh M;

    ConvexHull() : M(nil) { }

    void Register(Invoker& invoker) override {
        nubuck().add_menu_item(nubuck().object_menu(), "Convex Hull Sample", invoker);
    }

    bool Invoke() override {
        M = nubuck().first_selected_mesh();

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

    bool OnKey(const KeyEvent& event) override {
        if(nil != M && Qt::Key_S == event.keyCode) {
            FadeIn fadeIn(M);
            fadeIn.PlayUntilIsDone();
            nubuck().wait_for_animations();
            return true;
        }
        return false;
    }

};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CREATE_OPERATOR(ConvexHull), NULL);
}
