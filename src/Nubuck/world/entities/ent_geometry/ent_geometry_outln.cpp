#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>

#include <UI\colorbutton\colorbutton.h>
#include <UI\block_signals.h>
#include "ent_geometry_outln.h"
#include "ent_geometry.h"

namespace W {

void ENT_GeometryOutln::OnVertexScaleChanged(double value) {
    EV::Params_ENT_Geometry_VertexScaleChanged args;
    args.vertexScale = (float)value;
    _subject.Send(EV::def_ENT_Geometry_VertexScaleChanged.Create(args));
}

void ENT_GeometryOutln::OnEdgeScaleChanged(double value) {
	EV::Params_ENT_Geometry_EdgeScaleChanged args;
	args.edgeScale = (float)value;
	_subject.Send(EV::def_ENT_Geometry_EdgeScaleChanged.Create(args));
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

void ENT_GeometryOutln::OnEdgeShadingChanged(int) {
    SendEdgeShading();
}

void ENT_GeometryOutln::OnHiddenLinesChanged(int) {
    SendEdgeShading();
}

ENT_GeometryOutln::ENT_GeometryOutln(ENT_Geometry& subject) : _subject(subject) {
    InitOutline();

    AddEventHandler(EV::def_ENT_Geometry_VertexScaleChanged, this, &ENT_GeometryOutln::Event_VertexScaleChanged);
	AddEventHandler(EV::def_ENT_Geometry_EdgeScaleChanged, this, &ENT_GeometryOutln::Event_EdgeScaleChanged);
	AddEventHandler(EV::def_ENT_Geometry_EdgeColorChanged, this, &ENT_GeometryOutln::Event_EdgeColorChanged);
    AddEventHandler(EV::def_ENT_Geometry_RenderModeChanged, this, &ENT_GeometryOutln::Event_RenderModeChanged);
}

void ENT_GeometryOutln::InitOutline() {
	QGridLayout* layout = new QGridLayout();

    QLabel* lblVertexScale = new QLabel("vertex scale:");
    _sbVertexScale = new QDoubleSpinBox;
    _sbVertexScale->setMinimum(0.05f);
    _sbVertexScale->setMaximum(5.00f);
    _sbVertexScale->setSingleStep(0.1f);
	_sbVertexScale->setValue(_subject.GetVertexScale());

    QLabel* lblEdgeRadius = new QLabel("edge scale:");
    _sbEdgeScale = new QDoubleSpinBox;
    _sbEdgeScale->setMinimum(0.05f);
    _sbEdgeScale->setMaximum(5.00f);
    _sbEdgeScale->setSingleStep(0.1f);
	_sbEdgeScale->setValue(_subject.GetEdgeScale());

    QLabel* lblEdgeColor = new QLabel("edge color:");
    _btnEdgeColor = new UI::ColorButton;
	R::Color edgeColor = _subject.GetEdgeColor();
	_btnEdgeColor->SetColor(edgeColor.r, edgeColor.g, edgeColor.b);

    QLabel* lblEdgeShading = new QLabel("edge shading:");
    _cbEdgeShading = new QComboBox;
    _cbEdgeShading->addItem("nice");
    _cbEdgeShading->addItem("fast");
    _cbEdgeShading->addItem("lines");
    _cbEdgeShading->addItem("billboards (nice)");

    _cbHiddenLines = new QCheckBox("show hidden lines");

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

    layout->addWidget(lblVertexScale, 0, 0, 1, 1);
    layout->addWidget(_sbVertexScale, 0, 1, 1, 1);

    layout->addWidget(lblEdgeRadius, 1, 0, 1, 1);
    layout->addWidget(_sbEdgeScale, 1, 1, 1, 1);

    layout->addWidget(lblEdgeColor, 2, 0, 1, 1);
    layout->addWidget(_btnEdgeColor, 2, 1, 1, 1);

    layout->addWidget(lblEdgeShading, 3, 0, 1, 1);
    layout->addWidget(_cbEdgeShading, 3, 1, 1, 1);

    layout->addWidget(_cbHiddenLines, 4, 0, 1, 2);

    layout->addWidget(lblHullAlpha, 5, 0, 1, 1);
    layout->addWidget(_sldHullAlpha, 5, 1, 1, 1);

    layout->addWidget(lblRenderMode, 6, 0, 1, 1);
    layout->addLayout(hboxLayout, 6, 1, 1, 1);

	setLayout(layout);

    QObject::connect(_sbVertexScale, SIGNAL(valueChanged(double)), this, SLOT(OnVertexScaleChanged(double)));
    QObject::connect(_sbEdgeScale, SIGNAL(valueChanged(double)), this, SLOT(OnEdgeScaleChanged(double)));
    QObject::connect(_btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), this, SLOT(OnEdgeColorChanged(float, float, float)));
    QObject::connect(_cbEdgeShading, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEdgeShadingChanged(int)));
    QObject::connect(_cbHiddenLines, SIGNAL(stateChanged(int)), this, SLOT(OnHiddenLinesChanged(int)));
    QObject::connect(_sldHullAlpha, SIGNAL(valueChanged(int)), this, SLOT(OnTransparencyChanged(int)));

    QObject::connect(_btnRenderVertices, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderEdges, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderFaces, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
}

void ENT_GeometryOutln::SendEdgeShading() {
    EV::Params_ENT_Geometry_EdgeShadingChanged args;
    int idx = _cbEdgeShading->currentIndex();
    assert(0 <= idx && idx < IGeometry::ShadingMode::NUM_MODES);
    args.shadingMode = IGeometry::ShadingMode::Enum(idx);
    args.showHiddenLines = _cbHiddenLines->isChecked();
    _subject.Send(EV::def_ENT_Geometry_EdgeShadingChanged.Create(args));
}

void ENT_GeometryOutln::Event_VertexScaleChanged(const EV::Event& event) {
    const EV::Params_ENT_Geometry_VertexScaleChanged& args = EV::def_ENT_Geometry_VertexScaleChanged.GetArgs(event);
    _sbVertexScale->blockSignals(true);
    _sbVertexScale->setValue(args.vertexScale);
    _sbVertexScale->blockSignals(false);
}

void ENT_GeometryOutln::Event_EdgeScaleChanged(const EV::Event& event) {
	const EV::Params_ENT_Geometry_EdgeScaleChanged& args = EV::def_ENT_Geometry_EdgeScaleChanged.GetArgs(event);
	_sbEdgeScale->blockSignals(true);
	_sbEdgeScale->setValue(args.edgeScale);
	_sbEdgeScale->blockSignals(false);
}

void ENT_GeometryOutln::Event_EdgeColorChanged(const EV::Event& event) {
	const EV::Params_ENT_Geometry_EdgeColorChanged& args = EV::def_ENT_Geometry_EdgeColorChanged.GetArgs(event);
	_btnEdgeColor->blockSignals(true);
	_btnEdgeColor->SetColor(args.edgeColor.r, args.edgeColor.g, args.edgeColor.b);
	_btnEdgeColor->blockSignals(false);
}

void ENT_GeometryOutln::Event_RenderModeChanged(const EV::Event& event) {
    const EV::Params_ENT_Geometry_RenderModeChanged& args = EV::def_ENT_Geometry_RenderModeChanged.GetArgs(event);
    UI::BlockSignals blockSignals(_btnRenderVertices, _btnRenderEdges, _btnRenderFaces);
    _btnRenderVertices->setChecked(IGeometry::RenderMode::NODES & args.renderMode);
    _btnRenderEdges->setChecked(IGeometry::RenderMode::EDGES & args.renderMode);
    _btnRenderFaces->setChecked(IGeometry::RenderMode::FACES & args.renderMode);
}

void ENT_GeometryOutln::ExecEvents(const std::vector<EV::Event>& events) {
    for(unsigned i = 0; i < events.size(); ++i) Send(events[i]);
    HandleEvents();
}

} // namespace W