#include <QGridLayout>
#include <QLabel>

#include <Nubuck\polymesh.h>
#include "flipclip.h"

void FlipClipPanel::OnDistanceChanged(int value) {
    float dist = 10.0f * value / MAX_DISTANCE;
    EV::Params_FlipClip_DistanceChanged args = { dist };
    OP::SendToOperator(EV::def_FlipClip_DistanceChanged.Create(args));
}

FlipClipPanel::FlipClipPanel() {
    _dist = new QSlider(Qt::Horizontal);
    _dist->setMaximum(MAX_DISTANCE);
    connect(_dist, SIGNAL(valueChanged(int)), this, SLOT(OnDistanceChanged(int)));

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel("hull distance:"), 0, 0);
    gridLayout->addWidget(_dist, 0, 1);

    QWidget* dummy = new QWidget;
    dummy->setLayout(gridLayout);

    layout()->addWidget(dummy);
}

void FlipClipPanel::Invoke() {
    _dist->setValue(0);
}

void FlipClip::Event_DistanceChanged(const EV::Event& event) {
    const EV::Params_FlipClip_DistanceChanged& args = EV::def_FlipClip_DistanceChanged.GetArgs(event);

    float hdist = 0.5f * args.dist;

    _g.geom[Side::FRONT]->SetPosition(M::Vector3(0.0f, 0.0f, hdist));
    _g.geom[Side::BACK]->SetPosition(M::Vector3(0.0f, 0.0f, -hdist));
}

const char* FlipClip::GetName() const { return "Flip & Clip"; }

OP::ALG::Phase* FlipClip::Init(const Nubuck& nb) {
    _g.nb = nb;

    // choose first selected geometry as input
    ISelection* sel = _g.nb.world->GetSelection();
    std::vector<IGeometry*> geomSel = sel->GetList();
    if(geomSel.empty()) {
        _g.nb.log->printf("ERROR - no input object selected.\n");
        return NULL;
    }
    _g.geom[Side::FRONT] = geomSel[0];

    const unsigned renderAll = IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES;
    _g.geom[Side::FRONT]->SetRenderMode(renderAll);
    _g.geom[Side::FRONT]->SetName("Front Hull");

    leda::nb::RatPolyMesh& frontMesh = _g.geom[Side::FRONT]->GetRatPolyMesh();

    if(0 < frontMesh.number_of_edges()) {
        _g.nb.log->printf("deleting edges and faces of input mesh.\n");
        frontMesh.del_all_edges();
        frontMesh.del_all_faces(); // this is necessary!
    }

    _g.L[Side::FRONT] = frontMesh.all_nodes();

    _g.geom[Side::BACK] = _g.nb.world->CreateGeometry();
    _g.geom[Side::BACK]->SetRenderMode(renderAll);
    _g.geom[Side::BACK]->SetName("Back Hull");

    leda::nb::RatPolyMesh& backMesh = _g.geom[Side::BACK]->GetRatPolyMesh();
    backMesh = frontMesh;

    _g.L[Side::BACK] = backMesh.all_nodes();

    _g.side = Side::FRONT;

    _g.hullEdges[Side::FRONT] = _g.hullEdges[Side::BACK] = NULL;

    return new Phase_Init(_g);
}

FlipClip::FlipClip() {
    AddEventHandler(EV::def_FlipClip_DistanceChanged, this, &FlipClip::Event_DistanceChanged);
}