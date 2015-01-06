#include <maxint.h>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>

#include <QMenu>
#include <QAction>

#include <Nubuck\operators\operator_invoker.h>
#include <world\entities\ent_text\ent_text.h>

#include "op_loop.h"

static EV::ConcreteEventDef<EV::Event> ev_loopStart;

namespace OP {

void LoopPanel::OnButtonClicked() {
    OP::SendToOperator(ev_loopStart.Tag());
}

void Loop::Event_OP_Loop_Start(const EV::Event& event) {
    if(_geom) {
        /*
        _geom->SetEdgeRadius(0.25);
        _geom->SetEdgeColor(R::Color::Red);
        */

        nubuck().select_geometry(Nubuck::SELECT_MODE_NEW, _geom);
	}

    for(unsigned i = 0; i < 50; ++i) {
        printf("OP::LOOP %8d, Doing some action!\n", i);
        Sleep(100);
    }
}

void Loop::Event_ButtonClicked(const EV::Event& event) {
    printf("Ola, button clicked!\n");
}

void Loop::Event_Button0(const EV::Event& event) {
    printf("specialized handler for id = 0\n");
}

void Loop::Event_Button1(const EV::Event& event) {
    printf("specialized handler for id = 1\n");
}

Loop::Loop() : _geom(NULL) {
    AddEventHandler(ev_loopStart, this, &Loop::Event_OP_Loop_Start);
    AddEventHandler(ev_buttonClicked, this, &Loop::Event_ButtonClicked);
    AddEventHandler(ev_buttonClicked, this, &Loop::Event_Button0, 0);
    AddEventHandler(ev_buttonClicked, this, &Loop::Event_Button1, 1);

    _vertexEditor.SetAxis(3);
}

void Loop::Register(Invoker& invoker) {
    QAction* action = nubuck().object_menu()->addAction("Loop");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Loop::Invoke() {
    printf("OP::Loop::Invoke\n");
    nubuck().set_operator_name("Loop");

	leda::list<leda::d3_rat_point> L;
	L.push_back(leda::d3_rat_point(-1, -1, -1));
	L.push_back(leda::d3_rat_point(-1, -1,  1));
	L.push_back(leda::d3_rat_point(-1,  1, -1));
	L.push_back(leda::d3_rat_point(-1,  1,  1));
	L.push_back(leda::d3_rat_point( 1, -1, -1));
	L.push_back(leda::d3_rat_point( 1, -1,  1));
	L.push_back(leda::d3_rat_point( 1,  1, -1));
	L.push_back(leda::d3_rat_point( 1,  1,  1));

    _geom = (W::ENT_Geometry*)nubuck().create_geometry();
	_geom->SetRenderMode(Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES);
	leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
	leda::CONVEX_HULL(L, mesh);
	mesh.compute_faces();

    nubuck().set_geometry_pattern(_geom, Nubuck::Pattern::DOTS);
    leda::face f = mesh.first_face();
    mesh.set_pattern(f, R::Color::Red);

    W::ENT_Text* text = W::world.CreateText();

    return true;
}

bool Loop::OnMouse(const MouseEvent& event) {
    assert(_geom);
    _vertexEditor.HandleMouseEvent(event, *_geom);
    return true;
}

} // namespace OP