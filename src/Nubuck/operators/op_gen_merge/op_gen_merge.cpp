#include <maxint.h>

#include <QAction>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\op_gen_merge\op_gen_merge.h>

namespace OP {
namespace GEN {

MergePanel::MergePanel(QWidget* parent) : OperatorPanel(parent) { }

void Merge::Register(Invoker& invoker) {
    QAction* action = nubuck().scene_menu()->addAction("Create Merge Scene");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Merge::Invoke() {
    nubuck().set_operator_name("Create Merge Scene");

    leda::list_item it;

    leda::list<point3_t> L;
    leda::random_d3_rat_points_on_sphere(80, 4, L);

    const int tl = 8;
    leda::list<point3_t> L0, L1;
    forall_items(it, L) {
        L0.push_back(L[it].translate(-tl, 0, 0));
        L1.push_back(L[it].translate( tl, 0, 0));
    }

    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;

    nb::geometry geom0 = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(geom0, renderAll);
    leda::CONVEX_HULL(L0, nubuck().poly_mesh(geom0));
    nubuck().poly_mesh(geom0).compute_faces();

    nb::geometry geom1 = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(geom1, renderAll);
    leda::CONVEX_HULL(L1, nubuck().poly_mesh(geom1));
    nubuck().poly_mesh(geom1).compute_faces();

    nubuck().clear_selection();
    nubuck().select_geometry(Nubuck::SELECT_MODE_ADD, geom0);
    nubuck().select_geometry(Nubuck::SELECT_MODE_ADD, geom1);

    return true;
}

} // namespace GEN
} // namespace OP