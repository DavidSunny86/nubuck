#include <maxint.h>

#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include <world\world.h>
#include "op_chull.h"

namespace OP {

std::string ConvexHull::PreferredShortcut() const {
    return "C";
}

void ConvexHull::Register(Invoker& invoker) {
    NB::AddMenuItem(NB::ObjectMenu(), "Convex Hull", invoker);
}

bool ConvexHull::Invoke() {
    if(W::editMode_t::OBJECTS != W::world.GetEditMode().GetMode()) {
        NB::LogPrintf("cannot invoke operator 'Convex Hull' when not in OBJECT mode\n");
        return false;
    }

    if(!NB::FirstSelectedMesh()) {
        NB::LogPrintf("no mesh selected.\n");
        return false;
    }

    NB::SetOperatorName("Convex Hull");

    NB::Mesh cloud = NB::FirstSelectedMesh();
    assert(cloud);

    leda::nb::RatPolyMesh& cloudGraph = NB::GetGraph(cloud);
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudGraph) L.push_back(cloudGraph.position_of(v));

    NB::Mesh chull = NB::CreateMesh();
    NB::SetMeshRenderMode(chull, NB::RM_ALL);
    NB::SetMeshName(chull, std::string("CH(") + NB::GetMeshName(cloud) + ")");
    leda::nb::RatPolyMesh& chullGraph = NB::GetGraph(chull);
    leda::CONVEX_HULL(L, chullGraph);
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
