#include <maxint.h>

#include <QAction>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\op_gen_windows\op_gen_windows.h>

namespace OP {
namespace GEN {

void Windows::Register(Invoker& invoker) {
    QAction* action = NB::SceneMenu()->addAction("Create Windows Scene");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Windows::Invoke() {
    typedef leda::d3_rat_point point3_t;

    NB::SetOperatorName("Windows Scene");

    const int renderMode = NB::RM_FACES;
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
    NB::Mesh meshes[3];
    const float dist = 0.25f;
    for(int i = 0; i < 3; ++i) {
        NB::Mesh mesh = NB::CreateMesh();
        sprintf(nameBuffer, "Window %d", i);
        NB::SetMeshName(mesh, nameBuffer);
        NB::SetMeshRenderMode(mesh, renderMode);
        NB::SetMeshPosition(mesh, M::Vector3(0.0f, 0.0f, -dist + dist * i));
        leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);
        graph.make_triangle(p[0], p[1], p[2]);
        graph.set_color(graph.first_face(), c[i]);
        meshes[i] = mesh;
    }

    return true;
}

} // namespace GEN
} // namespace OP