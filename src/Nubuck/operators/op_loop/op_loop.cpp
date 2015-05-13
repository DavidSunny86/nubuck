#include <maxint.h>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\face_vertex_mesh.h>

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

        NB::SelectMesh(NB::SM_NEW, _geom);
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

    OnRequestFinish(EV::Event());
}

Loop::Loop() : _geom(NULL) {
    AddEventHandler(ev_loopStart, this, &Loop::Event_OP_Loop_Start);
    AddEventHandler(ev_buttonClicked, this, &Loop::Event_ButtonClicked);
    AddEventHandler(ev_buttonClicked, this, &Loop::Event_Button0, 0);
    AddEventHandler(ev_buttonClicked, this, &Loop::Event_Button1, 1);
    AddEventHandler(ev_mouse, this, &Loop::Event_Mouse);

    _vertexEditor.SetAxisFlags(3);
}

void Loop::Register(Invoker& invoker) {
    QAction* action = NB::ObjectMenu()->addAction("Loop");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Loop::Invoke() {
    printf("OP::Loop::Invoke\n");
    NB::SetOperatorName("Loop");

	leda::list<leda::d3_rat_point> positions;
	positions.push_back(leda::d3_rat_point(-1, -1, -1));
	positions.push_back(leda::d3_rat_point(-1, -1,  1));
	positions.push_back(leda::d3_rat_point(-1,  1, -1));
	positions.push_back(leda::d3_rat_point(-1,  1,  1));
	positions.push_back(leda::d3_rat_point( 1, -1, -1));
	positions.push_back(leda::d3_rat_point( 1, -1,  1));
	positions.push_back(leda::d3_rat_point( 1,  1, -1));
	positions.push_back(leda::d3_rat_point( 1,  1,  1));

    leda::list<unsigned> indices;
    add_quad(indices, 0, 1, 3, 2);
    add_quad(indices, 1, 5, 7, 3);
    add_quad(indices, 5, 4, 6, 7);
    add_quad(indices, 0, 2, 6, 4);
    add_quad(indices, 0, 4, 5, 1);
    add_quad(indices, 3, 7, 6, 2);

    _geom = (W::ENT_Geometry*)NB::CreateMesh();
    _geom->SetRenderMode(NB::RM_ALL);
	leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
    make_from_indices(positions, indices, mesh);

    NB::SetMeshPattern(_geom, NB::PATTERN_DOTS);
    leda::face f = mesh.first_face();
    mesh.set_pattern(f, R::Color::Red);

    NB::ShowMeshVertexLabels(_geom, true);

    W::ENT_Text* text = W::world.CreateText();

    _vertexEditor.Open(_geom);
    // _entityEditor.Open();

    return true;
}

void Loop::Event_Mouse(const EV::MouseEvent& event) {
    printf("Loop::Event_Mouse\n");

    assert(_geom);
    if(_vertexEditor.HandleMouseEvent(event)) {
        // W::SetColorsFromVertexSelection(*_geom);
    }
    event.Accept();
}

void Loop::OnMouse(const EV::MouseEvent& event) {
    printf("Loop::OnMouse\n");

    assert(_geom);
    if(_entityEditor.HandleMouseEvent(event)) {
        // W::SetColorsFromVertexSelection(*_geom);
    }
    event.Accept();
}

void Loop::OnKey(const EV::KeyEvent& event) {
    if(_vertexEditor.HandleKeyEvent(event)) {
    }
    event.Accept();
}

} // namespace OP