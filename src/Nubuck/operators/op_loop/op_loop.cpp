#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>

#include <QMenu>
#include <QAction>

#include <Nubuck\operators\operator_invoker.h>

#include "op_loop.h"

namespace OP {

void Loop::Event_OP_Loop_Start(const EV::Event& event) {
    for(unsigned i = 0; i < 50; ++i) {
        printf("OP::LOOP %8d, Doing some action!\n", i);
        Sleep(100);
    }
}

Loop::Loop() {
    AddEventHandler(EV::def_OP_Loop_Start, this, &Loop::Event_OP_Loop_Start);
}

void Loop::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;
    QAction* action = nb.ui->GetObjectMenu()->addAction("Loop");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void Loop::Invoke() {
    printf("OP::Loop::Invoke\n");

	leda::list<leda::d3_rat_point> L;
	L.push_back(leda::d3_rat_point(-1, -1, -1));
	L.push_back(leda::d3_rat_point(-1, -1,  1));
	L.push_back(leda::d3_rat_point(-1,  1, -1));
	L.push_back(leda::d3_rat_point(-1,  1,  1));
	L.push_back(leda::d3_rat_point( 1, -1, -1));
	L.push_back(leda::d3_rat_point( 1, -1,  1));
	L.push_back(leda::d3_rat_point( 1,  1, -1));
	L.push_back(leda::d3_rat_point( 1,  1,  1));

	IGeometry* geom = _nb.world->CreateGeometry();
	geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES);
	leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
	leda::CONVEX_HULL(L, mesh);
	mesh.compute_faces();
	geom->Update();
}

} // namespace OP