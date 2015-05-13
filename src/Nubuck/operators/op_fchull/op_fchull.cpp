#include <maxint.h>

#include <QMenu>
#include <QAction>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include <polymesh\flipclip.h>
#include "op_fchull.h"

namespace OP {

std::string FlipClip::PreferredShortcut() const {
    return "Shift+C";
}

void FlipClip::Register(Invoker& invoker) {
    NB::AddMenuItem(NB::ObjectMenu(), "FlipClip Hull", invoker);
}

bool FlipClip::Invoke() {
    if(!NB::FirstSelectedMesh()) {
        NB::LogPrintf("no mesh selected.\n");
        return false;
    }

    NB::SetOperatorName("FlipClip Hull");

    NB::LogPrintf("FlipClip Hull:\n");

    NB::Mesh cloud = NB::FirstSelectedMesh();
    assert(cloud);

    leda::nb::RatPolyMesh& cloudGraph = NB::GetGraph(cloud);
    leda::list<leda::d3_rat_point> L0, L1, L2;
    leda::node v;
    forall_nodes(v, cloudGraph) L0.push_back(cloudGraph.position_of(v));
    L1 = L0;
    L2 = L0;

    NB::Mesh chull = NB::CreateMesh();
    NB::SetMeshRenderMode(chull, NB::RM_ALL);
    NB::SetMeshName(chull, std::string("CH(") + NB::GetMeshName(cloud) + ")");
    leda::nb::RatPolyMesh& chullGraph = NB::GetGraph(chull);

    SYS::Timer  timer;
    float       secsPassed;

    leda::GRAPH<leda::d3_rat_point, int> H;

    timer.Start();
    leda::CONVEX_HULL(L0, H);
    secsPassed = timer.Stop();
    NB::LogPrintf("... CONVEX_HULL: %fs\n", secsPassed);

    H.clear();

    timer.Start();
    FlipClipHull(L1, H);
    secsPassed = timer.Stop();
    NB::LogPrintf("... FlipClip: %fs\n", secsPassed);

    FlipClipHull_WriteProfilerReport();
    NB::LogPrintf("... wrote profiler report to file\n");

    NB::LogPrintf("... CHECK_HULL: ");
    bool isConvex = leda::CHECK_HULL(H);
    NB::LogPrintf(isConvex ? "true" : "false");
    NB::LogPrintf("\n");

    leda::CONVEX_HULL(L2, chullGraph);
    chullGraph.compute_faces();

    NB::SetMeshPosition(chull, NB::GetMeshPosition(cloud));

    NB::DestroyMesh(cloud);
    NB::SelectMesh(NB::SM_NEW, chull);

    NB::LogPrintf("convex hull:\n");
    NB::LogPrintf("... |V| = %d\n", chullGraph.number_of_nodes());
    NB::LogPrintf("... |E| = %d\n", chullGraph.number_of_edges());
    NB::LogPrintf("... |F| = %d\n", chullGraph.number_of_faces());

    return true;
}

} // namespace OP