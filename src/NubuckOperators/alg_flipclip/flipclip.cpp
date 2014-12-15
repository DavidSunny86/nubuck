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

    nubuck().set_geometry_position(_g.geom[Side::FRONT], M::Vector3(0.0f, 0.0f, hdist));
    nubuck().set_geometry_position(_g.geom[Side::BACK], M::Vector3(0.0f, 0.0f, -hdist));
}

void FlipClip::Event_RunConfChanged(const EV::Arg<bool>& event) {
    bool haltBeforeStitching = event.value;
    _g.haltBeforeStitching = haltBeforeStitching;
}

const char* FlipClip::GetName() const { return "Flip & Clip"; }

OP::ALG::Phase* FlipClip::Init() {
    // choose first selected geometry as input
    std::vector<nb::geometry> geomSel = nubuck().selected_geometry();
    if(geomSel.empty()) {
        nubuck().log_printf("ERROR - no input object selected.\n");
        return NULL;
    }
    _g.geom[Side::FRONT] = geomSel[0];

    const unsigned renderAll = Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES;
    nubuck().set_geometry_render_mode(_g.geom[Side::FRONT], renderAll);
    nubuck().set_geometry_name(_g.geom[Side::FRONT], "Front Hull");

    leda::nb::RatPolyMesh& frontMesh = nubuck().poly_mesh(_g.geom[Side::FRONT]);

    if(0 < frontMesh.number_of_edges()) {
        nubuck().log_printf("deleting edges and faces of input mesh.\n");
        frontMesh.del_all_edges();
        frontMesh.del_all_faces(); // this is necessary!
    }

    _g.L[Side::FRONT] = frontMesh.all_nodes();

    _g.geom[Side::BACK] = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(_g.geom[Side::BACK], renderAll);
    nubuck().set_geometry_name(_g.geom[Side::BACK], "Back Hull");

    leda::nb::RatPolyMesh& backMesh = nubuck().poly_mesh(_g.geom[Side::BACK]);
    backMesh = frontMesh;

    _g.L[Side::BACK] = backMesh.all_nodes();

    _g.side = Side::FRONT;

    _g.hullEdges[Side::FRONT] = _g.hullEdges[Side::BACK] = NULL;

    _g.haltBeforeStitching = false;

    return new Phase_Init(_g);
}

FlipClip::FlipClip() {
    AddEventHandler(ev_distanceChanged, this, &FlipClip::Event_DistanceChanged);
    AddEventHandler(ev_runConfChanged, this, &FlipClip::Event_RunConfChanged);
}