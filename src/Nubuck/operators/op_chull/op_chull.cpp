#include <maxint.h>

#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include "op_chull.h"

namespace OP {

void ConvexHull::Register(Invoker& invoker) {
    QAction* action = nubuck().object_menu()->addAction("Convex Hull");
    action->setShortcut(QKeySequence("C"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool ConvexHull::Invoke() {
    std::vector<nb::geometry> geomSel = nubuck().selected_geometry();
    if(geomSel.empty()) {
        nubuck().log_printf("no geometry selected.\n");
        return false;
    }

    nubuck().set_operator_name("Convex Hull");

	nb::geometry cloud = geomSel[0];
    assert(cloud);

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(cloud);
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudMesh) L.push_back(cloudMesh.position_of(v));

    nb::geometry chull = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(chull, Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES);
    nubuck().set_geometry_name(chull, std::string("CH(") + nubuck().geometry_name(cloud) + ")");
    leda::nb::RatPolyMesh& chullMesh = nubuck().poly_mesh(chull);
    leda::CONVEX_HULL(L, chullMesh);
    chullMesh.compute_faces();

    nubuck().set_geometry_position(chull, nubuck().geometry_position(cloud));

    nubuck().destroy_geometry(cloud);
    nubuck().select_geometry(Nubuck::SELECT_MODE_NEW, chull);

    nubuck().log_printf("convex hull:\n");
    nubuck().log_printf("... |V| = %d\n", chullMesh.number_of_nodes());
    nubuck().log_printf("... |E| = %d\n", chullMesh.number_of_edges());
    nubuck().log_printf("... |F| = %d\n", chullMesh.number_of_faces());

    return true;
}

} // namespace OP
