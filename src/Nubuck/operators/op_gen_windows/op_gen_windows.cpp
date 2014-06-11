#include <QAction>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\op_gen_windows\op_gen_windows.h>

namespace OP {
namespace GEN {

void Windows::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;
    QAction* action = _nb.ui->GetSceneMenu()->addAction("Create Windows Scene");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Windows::Invoke() {
    typedef leda::d3_rat_point point3_t;

    _nb.ui->SetOperatorName("Windows Scene");

    const int renderMode = IGeometry::RenderMode::FACES;
    const point3_t p[3] = {
        point3_t(-1, -1, 0),
        point3_t( 1, -1, 0),
        point3_t( 0,  1, 0)
    };
    const R::Color c[3] = {
        R::Color(1.0f, 0.0f, 0.0f),
        R::Color(0.0f, 1.0f, 0.0f),
        R::Color(0.0f, 0.0f, 1.0f)
    };
    char nameBuffer[16];
    IGeometry* geoms[3];
    const float dist = 0.25f;
    for(int i = 0; i < 3; ++i) {
        IGeometry* geom = _nb.world->CreateGeometry();
        sprintf(nameBuffer, "Window %d", i);
        geom->SetName(nameBuffer);
        geom->SetRenderMode(renderMode);
        geom->SetPosition(M::Vector3(0.0f, 0.0f, -dist + dist * i));
        leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        mesh.make_triangle(p[0], p[1], p[2]);
        mesh.set_color(mesh.first_face(), c[i]);
        geoms[i] = geom;
    }

    return true;
}

} // namespace GEN
} // namespace OP