#include <Nubuck\polymesh.h>
#include <Nubuck\math_conv.h>
#include <Nubuck\face_vertex_mesh.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\cylinder\cylinder.h>
#include <renderer\mesh\cone\cone.h>
#include "op_gen_parametric.h"

namespace OP {

namespace ParametricImpl {

EV::ConcreteEventDef<Params> ev_paramsChanged;

} // namespace ParametricImpl

using namespace ParametricImpl;

struct PMeshType {
    enum Enum {
        SPHERE = 0,
        NUM_TYPES
    };
};

QWidget* ParametricPanel::CreateSphereParamWidget() {
    _sphereSubdiv = new NBW_SpinBox;
    _sphereSubdiv->setText("subdiv: ");
    _sphereSubdiv->setTypeMask(NBW_SpinBox::TypeFlags::INTEGER);
    _sphereSubdiv->setMinimum(0);
    _sphereSubdiv->setMaximum(5);
    _sphereSubdiv->setValue(DEFAULT_SPHERE_SUBDIV);

    connect(_sphereSubdiv, SIGNAL(SigValueChanged(leda::rational)),
        this, SLOT(OnSphereSubdivChanged(leda::rational)));

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(_sphereSubdiv);
    l->addStretch();

    QWidget* w = new QWidget;
    w->setLayout(l);
    return w;
}

void ParametricPanel::SendUpdatedParams() {
    Params params;
    params.sphereSubdiv = (int)_sphereSubdiv->value().numerator().to_long();
    SendToOperator(ev_paramsChanged.Tag(params));
}

void ParametricPanel::OnSphereSubdivChanged(leda::rational) {
    SendUpdatedParams();
}

ParametricPanel::ParametricPanel() {
    const QString nameOfType[] = {
        "sphere"
    };

    QComboBox* names = new QComboBox;
    for(int i = 0; i < PMeshType::NUM_TYPES; ++i) {
        names->addItem(nameOfType[i]);
    }

    _paramStack = new QStackedWidget;
    _paramStack->addWidget(CreateSphereParamWidget());

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(names);
    layout->addWidget(_paramStack);
    layout->addStretch();
    GetWidget()->setLayout(layout);
}

static void GraphFromMeshDesc(const R::Mesh::Desc& desc, NB::Graph& G) {
    leda::list<leda::d3_rat_point> positions;
    for(unsigned i = 0; i < desc.numVertices; ++i) {
        positions.push_back(ToRatPoint(desc.vertices[i].position));
    }

    std::vector<R::Mesh::TriIndices> tris;
    R::GenerateTriangles(std::vector<R::Mesh::Index>(desc.indices, desc.indices + desc.numIndices),
        desc.primType, tris);

    leda::list<unsigned> indices;
    for(unsigned i = 0; i < tris.size(); ++i) {
        indices.push_back(tris[i].indices[0]);
        indices.push_back(tris[i].indices[1]);
        indices.push_back(tris[i].indices[2]);
        indices.push_back(tris[i].indices[0]);
    }

    G.clear();
    make_from_indices(positions, indices, G);
}

void Parametric::RebuildMesh() {
    R::Sphere sphere(_params.sphereSubdiv, false);
    GraphFromMeshDesc(sphere.GetDesc(), NB::GetGraph(_mesh));
}

void Parametric::Event_ParamsChanged(const ParametricImpl::Params& event) {
    _params = event;
    RebuildMesh();
}

Parametric::Parametric()
    : _meshType(0)
    , _mesh(NULL)
{
    // set default parameters
    _params.sphereSubdiv = DEFAULT_SPHERE_SUBDIV;

    AddEventHandler(ev_paramsChanged, this, &Parametric::Event_ParamsChanged);
}

void Parametric::Register(Invoker& invoker) {
    NB::AddMenuItem(NB::SceneMenu(), "Parametric Mesh", invoker);
}

bool Parametric::Invoke() {
    _mesh = NB::CreateMesh();
    NB::SetMeshName(_mesh, "Paramteric Mesh");
    NB::SetMeshRenderMode(_mesh, NB::RM_ALL);

    RebuildMesh();

    return true;
}

void Parametric::Finish() {
    _mesh = NULL;
}

} // namespace OP