#include <QGridLayout>
#include <QLabel>

#include <Nubuck\polymesh.h>
#include "flipclip.h"

EV::ConcreteEventDef<EV::Arg<float> > ev_distanceChanged;
EV::ConcreteEventDef<EV::Arg<bool> > ev_runConfChanged; // arg = haltBeforeStitching

void FlipClipPanel::OnDistanceChanged(int value) {
    float dist = 10.0f * value / MAX_DISTANCE;
    OP::SendToOperator(ev_distanceChanged.Tag(dist));
}

void FlipClipPanel::OnHaltBeforeStitchingToggled(bool isChecked) {
    bool haltBeforeStitching = isChecked;
    OP::SendToOperator(ev_runConfChanged.Tag(haltBeforeStitching));
}

FlipClipPanel::FlipClipPanel() {
    _dist = new QSlider(Qt::Horizontal);
    _dist->setMaximum(MAX_DISTANCE);
    connect(_dist, SIGNAL(valueChanged(int)), this, SLOT(OnDistanceChanged(int)));

    _haltBeforeStitching = new QCheckBox("halt before stitching");
    _haltBeforeStitching->setChecked(false);
    connect(_haltBeforeStitching, SIGNAL(toggled(bool)), this, SLOT(OnHaltBeforeStitchingToggled(bool)));

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel("hull distance:"), 0, 0);
    gridLayout->addWidget(_dist, 0, 1);
    gridLayout->addWidget(_haltBeforeStitching, 1, 0, 1, 2);

    QWidget* dummy = new QWidget;
    dummy->setLayout(gridLayout);

    GetWidget()->layout()->addWidget(dummy);
}

void FlipClipPanel::Invoke() {
    _dist->setValue(0);
    _haltBeforeStitching->setChecked(false);
}

void FlipClip::Event_DistanceChanged(const EV::Arg<float>& event) {
    float hdist = 0.5f * event.value;

    NB::SetMeshPosition(_g.meshes[Side::FRONT], M::Vector3(0.0f, 0.0f, hdist));
    NB::SetMeshPosition(_g.meshes[Side::BACK], M::Vector3(0.0f, 0.0f, -hdist));
}

void FlipClip::Event_RunConfChanged(const EV::Arg<bool>& event) {
    bool haltBeforeStitching = event.value;
    _g.haltBeforeStitching = haltBeforeStitching;
}

const char* FlipClip::GetName() const { return "Flip & Clip"; }

OP::ALG::Phase* FlipClip::Init() {
    // choose first selected geometry as input
    if(!NB::FirstSelectedMesh()) {
        NB::LogPrintf("ERROR - no input object selected.\n");
        return NULL;
    }
    _g.meshes[Side::FRONT] = NB::FirstSelectedMesh();

    NB::SetMeshRenderMode(_g.meshes[Side::FRONT], NB::RM_ALL);
    NB::SetMeshName(_g.meshes[Side::FRONT], "Front Hull");

    leda::nb::RatPolyMesh& frontGraph = NB::GetGraph(_g.meshes[Side::FRONT]);

    if(0 < frontGraph.number_of_edges()) {
        NB::LogPrintf("deleting edges and faces of input mesh.\n");
        frontGraph.del_all_edges();
        frontGraph.del_all_faces(); // this is necessary!
    }

    _g.L[Side::FRONT] = frontGraph.all_nodes();

    _g.meshes[Side::BACK] = NB::CreateMesh();
    NB::SetMeshRenderMode(_g.meshes[Side::BACK], NB::RM_ALL);
    NB::SetMeshName(_g.meshes[Side::BACK], "Back Hull");

    leda::nb::RatPolyMesh& backGraph = NB::GetGraph(_g.meshes[Side::BACK]);
    backGraph = frontGraph;

    _g.L[Side::BACK] = backGraph.all_nodes();

    _g.side = Side::FRONT;

    _g.hullEdges[Side::FRONT] = _g.hullEdges[Side::BACK] = NULL;

    _g.haltBeforeStitching = false;

    return new Phase_Init(_g);
}

FlipClip::FlipClip() {
    AddEventHandler(ev_distanceChanged, this, &FlipClip::Event_DistanceChanged);
    AddEventHandler(ev_runConfChanged, this, &FlipClip::Event_RunConfChanged);
}