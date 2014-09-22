#include <QGridLayout>
#include <QLabel>

#include <Nubuck\polymesh.h>
#include "flipclip.h"

void FlipClipPanel::OnDistanceChanged(int value) {
    float dist = 10.0f * value / MAX_DISTANCE;
    EV::Params_FlipClip_DistanceChanged args = { dist };
    OP::SendToOperator(EV::def_FlipClip_DistanceChanged.Create(args));
}

void FlipClipPanel::OnHaltBeforeStitchingToggled(bool isChecked) {
    EV::Params_FlipClip_RunConfChanged args;
    args.haltBeforeStitching = isChecked;
    OP::SendToOperator(EV::def_FlipClip_RunConfChanged.Create(args));
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

    layout()->addWidget(dummy);
}

void FlipClipPanel::Invoke() {
    _dist->setValue(0);
    _haltBeforeStitching->setChecked(false);
}

void FlipClip::Event_DistanceChanged(const EV::Event& event) {
    const EV::Params_FlipClip_DistanceChanged& args = EV::def_FlipClip_DistanceChanged.GetArgs(event);

    float hdist = 0.5f * args.dist;

    nubuck().set_geometry_position(_g.geom[Side::FRONT], M::Vector3(0.0f, 0.0f, hdist));
    nubuck().set_geometry_position(_g.geom[Side::BACK], M::Vector3(0.0f, 0.0f, -hdist));
}

void FlipClip::Event_RunConfChanged(const EV::Event& event) {
    const EV::Params_FlipClip_RunConfChanged& args = EV::def_FlipClip_RunConfChanged.GetArgs(event);
    _g.haltBeforeStitching = args.haltBeforeStitching;
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
    AddEventHandler(EV::def_FlipClip_DistanceChanged, this, &FlipClip::Event_DistanceChanged);
    AddEventHandler(EV::def_FlipClip_RunConfChanged, this, &FlipClip::Event_RunConfChanged);
}