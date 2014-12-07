#include <maxint.h>

#include <QMenu>
#include <QAction>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include <polymesh\flipclip.h>
#include "op_fchull.h"

namespace OP {

void FlipClip::Register(Invoker& invoker) {
    QAction* action = nubuck().object_menu()->addAction("FlipClip Hull");
    action->setShortcut(QKeySequence("Shift+C"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool FlipClip::Invoke() {
    std::vector<nb::geometry> geomSel = nubuck().selected_geometry();
    if(geomSel.empty()) {
        nubuck().log_printf("no geometry selected.\n");
        return false;
    }

    nubuck().set_operator_name("FlipClip Hull");

    nubuck().log_printf("FlipClip Hull:\n");

	nb::geometry cloud = geomSel[0];
    assert(cloud);

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(cloud);
    leda::list<leda::d3_rat_point> L0, L1, L2;
    leda::node v;
    forall_nodes(v, cloudMesh) L0.push_back(cloudMesh.position_of(v));
    L1 = L0;
    L2 = L0;

    nb::geometry chull = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(chull, Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES);
    nubuck().set_geometry_name(chull, std::string("CH(") + nubuck().geometry_name(cloud) + ")");
    leda::nb::RatPolyMesh& chullMesh = nubuck().poly_mesh(chull);

    SYS::Timer  timer;
    float       secsPassed;

    leda::GRAPH<leda::d3_rat_point, int> H;

    timer.Start();
    leda::CONVEX_HULL(L0, H);
    secsPassed = timer.Stop();
    nubuck().log_printf("... CONVEX_HULL: %fs\n", secsPassed);

    H.clear();

    timer.Start();
    FlipClipHull(L1, H);
    secsPassed = timer.Stop();
    nubuck().log_printf("... FlipClip: %fs\n", secsPassed);

    FlipClipHull_WriteProfilerReport();
    nubuck().log_printf("... wrote profiler report to file\n");

    nubuck().log_printf("... CHECK_HULL: ");
    bool isConvex = leda::CHECK_HULL(H);
    nubuck().log_printf(isConvex ? "true" : "false");
    nubuck().log_printf("\n");

    leda::CONVEX_HULL(L2, chullMesh);
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