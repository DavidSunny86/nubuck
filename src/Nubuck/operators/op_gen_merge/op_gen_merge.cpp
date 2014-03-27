#include <QAction>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\op_gen_merge\op_gen_merge.h>

namespace OP {
namespace GEN {

MergePanel::MergePanel(QWidget* parent) : OperatorPanel(parent) { }

void Merge::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetSceneMenu()->addAction("Create Merge Scene");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void Merge::Invoke() {
    _nb.ui->SetOperatorName("Create Merge Scene");

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
        IGeometry::RenderMode::NODES |
        IGeometry::RenderMode::EDGES |
        IGeometry::RenderMode::FACES;

    IGeometry* geom0 = _nb.world->CreateGeometry();
    geom0->SetRenderMode(renderAll);
    leda::CONVEX_HULL(L0, geom0->GetRatPolyMesh());
    geom0->GetRatPolyMesh().compute_faces();

    IGeometry* geom1 = _nb.world->CreateGeometry();
    geom1->SetRenderMode(renderAll);
    leda::CONVEX_HULL(L1, geom1->GetRatPolyMesh());
    geom1->GetRatPolyMesh().compute_faces();
    
    ISelection* sel = _nb.world->GetSelection();
    sel->Clear();
    sel->Add(geom0);
    sel->Add(geom1);
}

} // namespace GEN
} // namespace OP