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
    action->setShortcut(QKeySequence("J"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Join::Invoke() {
    _nb.ui->SetOperatorName("Join");

	std::vector<IGeometry*> geomList = W::world.GetSelection()->GetList();
    if(geomList.size() < 2) return true; // nothing to join

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

	_nb.world->GetSelection()->Set(geom0);

    printf(">>>>>>>>>> OP::Join finished\n");

    return true;
}

} // namespace OP