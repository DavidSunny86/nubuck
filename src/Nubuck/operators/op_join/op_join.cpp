#include <maxint.h>

#include <QMenu>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_join.h"

namespace OP {

void Join::Register(Invoker& invoker) {
    QAction* action = NB::ObjectMenu()->addAction("Join");
    action->setShortcut(QKeySequence("J"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Join::Invoke() {
    NB::SetOperatorName("Join");

    NB::Mesh mesh0 = NB::FirstSelectedMesh();
    if(!mesh0) return true;

    NB::ApplyMeshTransformation(mesh0);
    leda::nb::RatPolyMesh& graph0 = NB::GetGraph(mesh0);

    NB::Mesh tmp, mesh1 = NB::NextSelectedMesh(mesh0);
    while(mesh1) {
        NB::ApplyMeshTransformation(mesh1);
        leda::nb::RatPolyMesh& graph1 = NB::GetGraph(mesh1);
        graph0.join(graph1);

        tmp = NB::NextSelectedMesh(mesh1);
        NB::DestroyMesh(mesh1);
        mesh1 = tmp;
    }

    NB::SelectMesh(NB::SM_NEW, mesh0);

    printf(">>>>>>>>>> OP::Join finished\n");

    return true;
}

} // namespace OP