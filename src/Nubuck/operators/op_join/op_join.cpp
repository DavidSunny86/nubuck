#include <QMenu>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_join.h"

namespace OP {

void Join::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("Join");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void Join::Invoke() {
    _nb.ui->SetOperatorName("Join");

    const std::vector<IGeometry*>& geomList = W::world.GetSelection().GetGeometryList();
    if(geomList.size() < 2) return; // nothing to join

    W::ENT_Geometry* geom0 = (W::ENT_Geometry*)geomList[0];
    geom0->ApplyTransformation();
    leda::nb::RatPolyMesh& mesh0 = geom0->GetRatPolyMesh();
    for(unsigned i = 1; i < geomList.size(); ++i) {
        W::ENT_Geometry* geom = (W::ENT_Geometry*)geomList[i];
        geom->ApplyTransformation();
        leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        mesh0.join(mesh);
        geom->Destroy();
    }

    geom0->Update();
    _nb.world->SelectGeometry(geom0);
}

} // namespace OP