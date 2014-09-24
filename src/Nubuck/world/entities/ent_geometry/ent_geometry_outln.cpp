#include <maxint.h>

#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>

#include <UI\nbw_spinbox\nbw_spinbox.h>
#include <UI\colorbutton\colorbutton.h>
#include <UI\block_signals.h>
#include "ent_geometry_outln.h"
#include "ent_geometry.h"

namespace W {

void ENT_GeometryOutln::OnVertexScaleChanged(leda::rational value) {
    EV::Params_ENT_Geometry_VertexScaleChanged args;
    args.vertexScale = value.to_float();
    _subject.Send(EV::def_ENT_Geometry_VertexScaleChanged.Create(args));
}

void ENT_GeometryOutln::OnEdgeScaleChanged(leda::rational value) {
	EV::Params_ENT_Geometry_EdgeScaleChanged args;
    args.edgeScale = value.to_float();
	_subject.Send(EV::def_ENT_Geometry_EdgeScaleChanged.Create(args));
}

void ENT_GeometryOutln::OnEdgeColorChanged(float r, float g, float b) {
	EV::Params_ENT_Geometry_EdgeColorChanged args;
	args.edgeColor = R::Color(r, g, b);
	_subject.Send(EV::def_ENT_Geometry_EdgeColorChanged.Create(args));
}

void ENT_GeometryOutln::OnTransparencyChanged(leda::rational value) {
    EV::Params_ENT_Geometry_TransparencyChanged args;
    args.transparency = value.to_float();
    _subject.Send(EV::def_ENT_Geometry_TransparencyChanged.Create(args));
}

void ENT_GeometryOutln::OnRenderModeChanged(bool) {
    EV::Params_ENT_Geometry_RenderModeChanged args;
    int renderMode = 0;
    if(_btnRenderVertices->isChecked()) renderMode |= Nubuck::RenderMode::NODES;
    if(_btnRenderEdges->isChecked()) renderMode |= Nubuck::RenderMode::EDGES;
    if(_btnRenderFaces->isChecked()) renderMode |= Nubuck::RenderMode::FACES;
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

    _sbVertexScale = new NBW_SpinBox;
    _sbVertexScale->setText("vertex scale: ");
    _sbVertexScale->setMinimum(0.05f);
    _sbVertexScale->setMaximum(5.00f);
    _sbVertexScale->setSingleStep(0.1f);
	_sbVertexScale->setValue(_subject.GetVertexScale());

    _sbEdgeScale = new NBW_SpinBox;
    _sbEdgeScale->setText("edge scale: ");
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

    _sbHullAlpha = new NBW_SpinBox;
    _sbHullAlpha->showProgressBar(true);
    _sbHullAlpha->setText("hull alpha: ");
    _sbHullAlpha->setMinimum(0.0);
    _sbHullAlpha->setMaximum(1.0);
    _sbHullAlpha->setSingleStep(0.025);
    _sbHullAlpha->setValue(1.0);

    QLabel* lblRenderMode = new QLabel("rendermode: ");
    _btnRenderVertices = new QPushButton(QIcon(":/ui/Images/vertices.png"), "");
    _btnRenderEdges = new QPushButton(QIcon(":/ui/Images/edges.png"), "");
    _btnRenderFaces = new QPushButton(QIcon(":/ui/Images/faces.png"), "");
    _btnRenderVertices->setCheckable(true);
    _btnRenderEdges->setCheckable(true);
    _btnRenderFaces->setCheckable(true);
    _btnRenderVertices->setChecked(_subject.GetRenderMode() & Nubuck::RenderMode::NODES);
    _btnRenderEdges->setChecked(_subject.GetRenderMode() & Nubuck::RenderMode::EDGES);
    _btnRenderFaces->setChecked(_subject.GetRenderMode() & Nubuck::RenderMode::FACES);
    QHBoxLayout* hboxLayout =  new QHBoxLayout;
    hboxLayout->addWidget(_btnRenderVertices);
    hboxLayout->addWidget(_btnRenderEdges);
    hboxLayout->addWidget(_btnRenderFaces);

    layout->addWidget(_sbVertexScale, 0, 0, 1, 2);

    layout->addWidget(_sbEdgeScale, 1, 0, 1, 2);

    layout->addWidget(lblEdgeColor, 2, 0, 1, 1);
    layout->addWidget(_btnEdgeColor, 2, 1, 1, 1);

    layout->addWidget(lblEdgeShading, 3, 0, 1, 1);
    layout->addWidget(_cbEdgeShading, 3, 1, 1, 1);

    layout->addWidget(_cbHiddenLines, 4, 0, 1, 2);

    layout->addWidget(_sbHullAlpha, 5, 0, 1, 2);

    layout->addWidget(lblRenderMode, 6, 0, 1, 1);
    layout->addLayout(hboxLayout, 6, 1, 1, 1);

	setLayout(layout);

    QObject::connect(_sbVertexScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnVertexScaleChanged(leda::rational)));
    QObject::connect(_sbEdgeScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnEdgeScaleChanged(leda::rational)));
    QObject::connect(_btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), this, SLOT(OnEdgeColorChanged(float, float, float)));
    QObject::connect(_cbEdgeShading, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEdgeShadingChanged(int)));
    QObject::connect(_cbHiddenLines, SIGNAL(stateChanged(int)), this, SLOT(OnHiddenLinesChanged(int)));
    QObject::connect(_sbHullAlpha, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnTransparencyChanged(leda::rational)));

    QObject::connect(_btnRenderVertices, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderEdges, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderFaces, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
}

void ENT_GeometryOutln::SendEdgeShading() {
    EV::Params_ENT_Geometry_EdgeShadingChanged args;
    int idx = _cbEdgeShading->currentIndex();
    assert(0 <= idx && idx < Nubuck::ShadingMode::NUM_MODES);
    args.shadingMode = Nubuck::ShadingMode::Enum(idx);
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
    _btnRenderVertices->setChecked(Nubuck::RenderMode::NODES & args.renderMode);
    _btnRenderEdges->setChecked(Nubuck::RenderMode::EDGES & args.renderMode);
    _btnRenderFaces->setChecked(Nubuck::RenderMode::FACES & args.renderMode);
}

void ENT_GeometryOutln::ExecEvents(const std::vector<EV::Event>& events) {
    for(unsigned i = 0; i < events.size(); ++i) Send(events[i]);
    HandleEvents();
}

} // namespace W