#include <maxint.h>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>

#include <QMenu>
#include <QAction>

#include <Nubuck\operators\operator_invoker.h>

#include "op_loop.h"

namespace OP {

void Loop::Event_OP_Loop_Start(const EV::Event& event) {
    if(_geom) {
        /*
        _geom->SetEdgeRadius(0.25);
        _geom->SetEdgeColor(R::Color::Red);
        */

        _nb.world->GetSelection()->Set(_geom);
	}

    for(unsigned i = 0; i < 50; ++i) {
        printf("OP::LOOP %8d, Doing some action!\n", i);
        Sleep(100);
    }
}

Loop::Loop() : _geom(NULL) {
    AddEventHandler(EV::def_OP_Loop_Start, this, &Loop::Event_OP_Loop_Start);
}

void Loop::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;
    QAction* action = nb.ui->GetObjectMenu()->addAction("Loop");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Loop::Invoke() {
    printf("OP::Loop::Invoke\n");
    _nb.ui->SetOperatorName("Loop");

	leda::list<leda::d3_rat_point> L;
	L.push_back(leda::d3_rat_point(-1, -1, -1));
	L.push_back(leda::d3_rat_point(-1, -1,  1));
	L.push_back(leda::d3_rat_point(-1,  1, -1));
	L.push_back(leda::d3_rat_point(-1,  1,  1));
	L.push_back(leda::d3_rat_point( 1, -1, -1));
	L.push_back(leda::d3_rat_point( 1, -1,  1));
	L.push_back(leda::d3_rat_point( 1,  1, -1));
	L.push_back(leda::d3_rat_point( 1,  1,  1));

	_geom = (W::ENT_Geometry*)_nb.world->CreateGeometry();
	_geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
	leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
	leda::CONVEX_HULL(L, mesh);
	mesh.compute_faces();

    return true;
}

} // namespace OP