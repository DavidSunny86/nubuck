#include <maxint.h>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\face_vertex_mesh.h>

#include <QMenu>
#include <QAction>

#include <Nubuck\operators\operator_invoker.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <world\entities\ent_text\ent_text.h>

#include "op_selected_faces.h"

static EV::ConcreteEventDef<EV::Event> ev_loopStart;

namespace OP {

SelectedFaces::SelectedFaces() { }

void SelectedFaces::Register(Invoker& invoker) {
    QAction* action = NB::ObjectMenu()->addAction("Selected Faces");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool SelectedFaces::Invoke() {
    printf("OP::SelectedFaces::Invoke\n");
    NB::SetOperatorName("SelectedFaces");

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

    W::ENT_Geometry* geom = (W::ENT_Geometry*)NB::CreateMesh();
    geom->SetRenderMode(NB::RM_ALL);
	leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
    make_from_indices(positions, indices, mesh);

    leda::face f = 0;

    // color
    f = mesh.first_face();
    mesh.set_color(f, R::Color::Blue);

    // pattern
    f = mesh.next_face(f);
    NB::SetMeshPattern(geom, NB::PATTERN_DOTS);
    mesh.set_pattern(f, R::Color::Blue);

    // curve
    f = mesh.next_face(f);
    geom->AddCurve(f, R::Color::Blue);

    W::ENT_Text* text = W::world.CreateText();
    text->SetContent("highlights!");
    text->SetPosition(M::Vector3(-12.0f, 2.0f, -10.0f));

    return true;
}

} // namespace OP