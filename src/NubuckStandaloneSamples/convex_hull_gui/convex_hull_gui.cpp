/*
convex_hull_gui.cpp
*/

/*
In the previous sample we created a custom fade-in animation and
implemented the OnKey() method to start the animation when the user
presses the 'S' key.  In this sample we create a graphical user
interface (GUI) for our operator that contains a single button which
starts the animation.

1. CREATING THE GUI

First, we define the class ConvexHullPanel that derives from
OperatorPanel. The panel becomes the content of the operator panel
window when our operator becomes active and contains all our GUI
elements (also called widgets).

In the constructor of the panel we create the button using the
create_button() method, which expects as arguments an integer that
uniquely identifies the button and a label.

Widgets must be organized in layouts. Layouts describe how widgets are
laid out and rearrange widgets automatically when the available amount
of space changes (eg. when the window is resized).
Therefore we create a vertical box layout and add our button to it.

Finally, we connect the panel to the operator by passing the panel as
the final argument to RunNubuck().

2. REACTING TO BUTTON CLICKS

When the user clicks on the button, a special event of type
ev_buttonClicked is sent to the operator.  In order to react to this
event, we have to do two things:
1. write a method that should get called when the button is clicked (the
callback method), and
2. tell the system to call this method when the operator receives an
event of type ev_buttonClicked.
In the constructor of the operator, we connect the callback method with
the event using the method AddEventHandler. It expects the following
arguments:
- the event type. (in this case ev_buttonClicked)
- the instance on which the callback method is called. (usually "this")
- the callback method 
- the identifier of the button
*/

#include <Nubuck\nb_common.h>
#include <Nubuck\events\core_events.h>
#include <LEDA\geo\d3_hull.h>

using namespace ::nb;
using namespace ::A;
using namespace ::M;
using namespace ::OP;
using namespace ::leda;

enum {
    BUTTON_FADE_IN // choose ID for button
};

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

struct ConvexHullPanel : OperatorPanel {
    ConvexHullPanel() {
        Button button = nubuck().create_button(BUTTON_FADE_IN, "Fade In");

        BoxLayout vbox = nubuck().create_vertical_box_layout();
        nubuck().add_widget_to_box(vbox, nubuck().to_widget(button));
        SetLayout(vbox);
    }
};

struct ConvexHull : Operator {

    mesh M;

    void OnFadeIn(const EV::Event&) {
        if(nil != M) {
            FadeIn fadeIn(M);
            fadeIn.PlayUntilIsDone();
            nubuck().wait_for_animations();
        }
    }

    ConvexHull() : M(nil) {
        AddEventHandler(ev_buttonClicked, this, &ConvexHull::OnFadeIn, BUTTON_FADE_IN);
    }

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

};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CREATE_OPERATOR(ConvexHull), CREATE_OPERATOR_PANEL(ConvexHullPanel));
}
