#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>

#include <UI\colorbutton\colorbutton.h>
#include "ent_geometry_outln.h"
#include "ent_geometry.h"

namespace W {

void ENT_GeometryOutln::OnEdgeRadiusChanged(double value) {
	EV::Params_ENT_Geometry_EdgeRadiusChanged args;
	args.edgeRadius = (float)value;
	_subject.Send(EV::def_ENT_Geometry_EdgeRadiusChanged.Create(args));
}

void ENT_GeometryOutln::OnEdgeColorChanged(float r, float g, float b) {
	EV::Params_ENT_Geometry_EdgeColorChanged args;
	args.edgeColor = R::Color(r, g, b);
	_subject.Send(EV::def_ENT_Geometry_EdgeColorChanged.Create(args));
}

void ENT_GeometryOutln::OnTransparencyChanged(int value) {
    EV::Params_ENT_Geometry_TransparencyChanged args;
    args.transparency = (float)value / _sldHullAlpha->maximum();
    _subject.Send(EV::def_ENT_Geometry_TransparencyChanged.Create(args));
}

void ENT_GeometryOutln::OnRenderModeChanged(bool) {
    EV::Params_ENT_Geometry_RenderModeChanged args;
    int renderMode = 0;
    if(_btnRenderVertices->isChecked()) renderMode |= IGeometry::RenderMode::NODES;
    if(_btnRenderEdges->isChecked()) renderMode |= IGeometry::RenderMode::EDGES;
    if(_btnRenderFaces->isChecked()) renderMode |= IGeometry::RenderMode::FACES;
    args.renderMode = renderMode;
    _subject.Send(EV::def_ENT_Geometry_RenderModeChanged.Create(args));
}

void ENT_GeometryOutln::OnEdgeShadingChanged(int idx) {
    EV::Params_ENT_Geometry_EdgeShadingChanged args;
    assert(0 <= idx && idx < IGeometry::ShadingMode::NUM_MODES);
    args.shadingMode = IGeometry::ShadingMode::Enum(idx);
    _subject.Send(EV::def_ENT_Geometry_EdgeShadingChanged.Create(args));
}

ENT_GeometryOutln::ENT_GeometryOutln(ENT_Geometry& subject) : _subject(subject) {
    InitOutline();

	AddEventHandler(EV::def_ENT_Geometry_EdgeRadiusChanged, this, &ENT_GeometryOutln::Event_EdgeRadiusChanged);
	AddEventHandler(EV::def_ENT_Geometry_EdgeColorChanged, this, &ENT_GeometryOutln::Event_EdgeColorChanged);
}

void ENT_GeometryOutln::InitOutline() {
	QGridLayout* layout = new QGridLayout();

    QLabel* lblEdgeRadius = new QLabel("edge radius:");
    _sbEdgeRadius = new QDoubleSpinBox;
    _sbEdgeRadius->setMinimum(0.05f);
    _sbEdgeRadius->setMaximum(5.00f);
    _sbEdgeRadius->setSingleStep(0.1f);
	_sbEdgeRadius->setValue(_subject.GetEdgeRadius());

    QLabel* lblEdgeColor = new QLabel("edge color:");
    _btnEdgeColor = new UI::ColorButton;
	R::Color edgeColor = _subject.GetEdgeColor();
	_btnEdgeColor->SetColor(edgeColor.r, edgeColor.g, edgeColor.b);

    QLabel* lblEdgeShading = new QLabel("edge shading:");
    _cbEdgeShading = new QComboBox;
    _cbEdgeShading->addItem("nice");
    _cbEdgeShading->addItem("fast");
    _cbEdgeShading->addItem("lines");

    QLabel* lblHullAlpha = new QLabel("hull alpha:");
    _sldHullAlpha = new QSlider(Qt::Horizontal);
    _sldHullAlpha->setTracking(true);
    _sldHullAlpha->setMinimum(0);
    _sldHullAlpha->setMaximum(100);
    _sldHullAlpha->setValue(100);

    QLabel* lblRenderMode = new QLabel("rendermode: ");
    _btnRenderVertices = new QPushButton(QIcon(":/ui/Images/vertices.png"), "");
    _btnRenderEdges = new QPushButton(QIcon(":/ui/Images/edges.png"), "");
    _btnRenderFaces = new QPushButton(QIcon(":/ui/Images/faces.png"), "");
    _btnRenderVertices->setCheckable(true);
    _btnRenderEdges->setCheckable(true);
    _btnRenderFaces->setCheckable(true);
    _btnRenderVertices->setChecked(_subject.GetRenderMode() & IGeometry::RenderMode::NODES);
    _btnRenderEdges->setChecked(_subject.GetRenderMode() & IGeometry::RenderMode::EDGES);
    _btnRenderFaces->setChecked(_subject.GetRenderMode() & IGeometry::RenderMode::FACES);
    QHBoxLayout* hboxLayout =  new QHBoxLayout;
    hboxLayout->addWidget(_btnRenderVertices);
    hboxLayout->addWidget(_btnRenderEdges);
    hboxLayout->addWidget(_btnRenderFaces);

    layout->addWidget(lblEdgeRadius, 0, 0, 1, 1);
    layout->addWidget(_sbEdgeRadius, 0, 1, 1, 1);

    layout->addWidget(lblEdgeColor, 1, 0, 1, 1);
    layout->addWidget(_btnEdgeColor, 1, 1, 1, 1);

    layout->addWidget(lblEdgeShading, 2, 0, 1, 1);
    layout->addWidget(_cbEdgeShading, 2, 1, 1, 1);

    layout->addWidget(lblHullAlpha, 3, 0, 1, 1);
    layout->addWidget(_sldHullAlpha, 3, 1, 1, 1);

    layout->addWidget(lblRenderMode, 4, 0, 1, 1);
    layout->addLayout(hboxLayout, 4, 1, 1, 1);

	setLayout(layout);

    QObject::connect(_sbEdgeRadius, SIGNAL(valueChanged(double)), this, SLOT(OnEdgeRadiusChanged(double)));
    QObject::connect(_btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), this, SLOT(OnEdgeColorChanged(float, float, float)));
    QObject::connect(_cbEdgeShading, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEdgeShadingChanged(int)));
    QObject::connect(_sldHullAlpha, SIGNAL(valueChanged(int)), this, SLOT(OnTransparencyChanged(int)));

    QObject::connect(_btnRenderVertices, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderEdges, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderFaces, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
}

void ENT_GeometryOutln::Event_EdgeRadiusChanged(const EV::Event& event) {
	const EV::Params_ENT_Geometry_EdgeRadiusChanged& args = EV::def_ENT_Geometry_EdgeRadiusChanged.GetArgs(event);
	_sbEdgeRadius->blockSignals(true);
	_sbEdgeRadius->setValue(args.edgeRadius);
	_sbEdgeRadius->blockSignals(false);
}

void ENT_GeometryOutln::Event_EdgeColorChanged(const EV::Event& event) {
	const EV::Params_ENT_Geometry_EdgeColorChanged& args = EV::def_ENT_Geometry_EdgeColorChanged.GetArgs(event);
	_btnEdgeColor->blockSignals(true);
	_btnEdgeColor->SetColor(args.edgeColor.r, args.edgeColor.g, args.edgeColor.b);
	_btnEdgeColor->blockSignals(false);
}

void ENT_GeometryOutln::ExecEvents(const std::vector<EV::Event>& events) {
    for(unsigned i = 0; i < events.size(); ++i) Send(events[i]);
    HandleEvents();
}

} // namespace W