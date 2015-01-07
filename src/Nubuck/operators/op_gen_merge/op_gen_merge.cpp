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

MergePanel::MergePanel() { }

void Merge::Register(Invoker& invoker) {
    QAction* action = NB::SceneMenu()->addAction("Create Merge Scene");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Merge::Invoke() {
    NB::SetOperatorName("Create Merge Scene");

    leda::list_item it;

    leda::list<point3_t> L;
    leda::random_d3_rat_points_on_sphere(80, 4, L);

    const int tl = 8;
    leda::list<point3_t> L0, L1;
    forall_items(it, L) {
        L0.push_back(L[it].translate(-tl, 0, 0));
        L1.push_back(L[it].translate( tl, 0, 0));
    }

    NB::Mesh mesh0 = NB::CreateMesh();
    NB::SetMeshRenderMode(mesh0, NB::RM_ALL);
    leda::CONVEX_HULL(L0, NB::GetGraph(mesh0));
    NB::GetGraph(mesh0).compute_faces();

    NB::Mesh mesh1 = NB::CreateMesh();
    NB::SetMeshRenderMode(mesh1, NB::RM_ALL);
    leda::CONVEX_HULL(L1, NB::GetGraph(mesh1));
    NB::GetGraph(mesh1).compute_faces();

    NB::ClearSelection();
    NB::SelectMesh(NB::SM_ADD, mesh0);
    NB::SelectMesh(NB::SM_ADD, mesh1);

    return true;
}

} // namespace GEN
} // namespace OP