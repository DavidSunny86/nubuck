#include <maxint.h>

#include <QMenu>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_join.h"

namespace OP {

void Join::Register(Invoker& invoker) {
    QAction* action = nubuck().object_menu()->addAction("Join");
    action->setShortcut(QKeySequence("J"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Join::Invoke() {
    nubuck().set_operator_name("Join");

    std::vector<nb::geometry> geomList = nubuck().selected_geometry();
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

    nubuck().select_geometry(Nubuck::SELECT_MODE_NEW, geom0);

    printf(">>>>>>>>>> OP::Join finished\n");

    return true;
}

} // namespace OP