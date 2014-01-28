#include <QMenu>

#include <Nubuck\polymesh.h>
#include "op_loadobj.h"

namespace OP {

void LoadOBJ::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetSceneMenu()->addAction("Load .obj file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void LoadOBJ::Invoke() {
    printf("LoadOBJ::Invoke\n");

    IGeometry* geom = _nb.world->CreateGeometry();
    geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
    mesh.FromObj("bunny2.obj");
    geom->Update();

    _nb.world->SelectGeometry(geom);
}

} // namespace OP