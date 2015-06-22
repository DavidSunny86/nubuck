#include <maxint.h>

#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QTabWidget>

#include <Nubuck\UI\nbw_spinbox.h>
#include <UI\colorbutton\colorbutton.h>
#include <UI\block_signals.h>
#include "ent_geometry_outln.h"
#include "ent_geometry.h"

namespace W {

void ENT_GeometryOutln::OnVertexScaleChanged(leda::rational value) {
    _subject.Send(ev_geom_vertexScaleChanged.Tag(value.to_float()));
}

void ENT_GeometryOutln::OnEdgeScaleChanged(leda::rational value) {
    _subject.Send(ev_geom_edgeScaleChanged.Tag(value.to_float()));
}

void ENT_GeometryOutln::OnEdgeTintChanged(float r, float g, float b) {
	R::Color edgeTint = R::Color(r, g, b);
    _subject.Send(ev_geom_edgeTintChanged.Tag(edgeTint));
}

void ENT_GeometryOutln::OnTransparencyChanged(leda::rational value) {
    _subject.Send(ev_geom_transparencyChanged.Tag(value.to_float()));
}

void ENT_GeometryOutln::OnRenderModeChanged(bool) {
    RenderModeEvent event;
    int renderMode = 0;
    if(_btnRenderVertices->isChecked()) renderMode |= NB::RM_NODES;
    if(_btnRenderEdges->isChecked()) renderMode |= NB::RM_EDGES;
    if(_btnRenderFaces->isChecked()) renderMode |= NB::RM_FACES;
    event.renderMode = renderMode;
    event.showWireframe = _cbWireframe->isChecked();
    event.showNormals = _cbNormals->isChecked();

    _cbWireframe->setEnabled(_btnRenderFaces->isChecked());
    _cbNormals->setEnabled(_btnRenderFaces->isChecked());

    _subject.Send(ev_geom_renderModeChanged.Tag(event));
}

void ENT_GeometryOutln::OnShowVertexLabelsChanged(bool checked) {
    _subject.Send(ev_geom_showVertexLabels.Tag(checked));
}

void ENT_GeometryOutln::OnXrayVertexLabelsChanged(bool checked) {
    _subject.Send(ev_geom_xrayVertexLabels.Tag(checked));
}

void ENT_GeometryOutln::OnVertexLabelSizeChanged(leda::rational value) {
    _subject.Send(ev_geom_setVertexLabelSize.Tag(value.to_float()));
}

void ENT_GeometryOutln::OnPositionChanged() {
    SetEntityVectorEvent event;
    event.m_entityID = _subject.GetID();
    event.m_vector[0] = _sbPosition[0]->value();
    event.m_vector[1] = _sbPosition[1]->value();
    event.m_vector[2] = _sbPosition[2]->value();
    _subject.Send(ev_ent_usr_setPosition.Tag(event));
}

void ENT_GeometryOutln::OnEdgeShadingChanged(int) {
    SendEdgeShading();
}

void ENT_GeometryOutln::OnHiddenLinesChanged(int) {
    SendEdgeShading();
}

ENT_GeometryOutln::ENT_GeometryOutln(ENT_Geometry& subject) : _subject(subject) {
    InitOutline();

    AddEventHandler(ev_geom_vertexScaleChanged, this, &ENT_GeometryOutln::Event_VertexScaleChanged);
	AddEventHandler(ev_geom_edgeScaleChanged, this, &ENT_GeometryOutln::Event_EdgeScaleChanged);
	AddEventHandler(ev_geom_edgeTintChanged, this, &ENT_GeometryOutln::Event_EdgeTintChanged);
	AddEventHandler(ev_geom_edgeShadingChanged, this, &ENT_GeometryOutln::Event_EdgeShadingChanged);
    AddEventHandler(ev_geom_renderModeChanged, this, &ENT_GeometryOutln::Event_RenderModeChanged);
    AddEventHandler(ev_geom_showVertexLabels, this, &ENT_GeometryOutln::Event_ShowVertexLabels);
    AddEventHandler(ev_geom_xrayVertexLabels, this, &ENT_GeometryOutln::Event_XrayVertexLabels);
    AddEventHandler(ev_geom_setVertexLabelSize, this, &ENT_GeometryOutln::Event_SetVertexLabelSize);
}

void ENT_GeometryOutln::InitPropertiesTab() {
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

    QLabel* lblEdgeTint = new QLabel("edge tint:");
    _btnEdgeTint = new UI::ColorButton;
	R::Color edgeTint = _subject.GetEdgeTint();
	_btnEdgeTint->SetColor(edgeTint.r, edgeTint.g, edgeTint.b);

    QLabel* lblEdgeShading = new QLabel("edge shading:");
    _cbEdgeShading = new QComboBox;
    _cbEdgeShading->addItem("nice");
    _cbEdgeShading->addItem("fast");
    _cbEdgeShading->addItem("lines");
    _cbEdgeShading->addItem("billboards (nice)");
    _cbEdgeShading->setCurrentIndex(_subject.GetShadingMode());

    _cbHiddenLines = new QCheckBox("show hidden lines");
    _cbHiddenLines->setChecked(_subject.StylizedHiddenLinesEnabled());

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
    _btnRenderVertices->setChecked(_subject.GetRenderMode() & NB::RM_NODES);
    _btnRenderEdges->setChecked(_subject.GetRenderMode() & NB::RM_EDGES);
    _btnRenderFaces->setChecked(_subject.GetRenderMode() & NB::RM_FACES);
    QHBoxLayout* hboxLayout =  new QHBoxLayout;
    hboxLayout->addWidget(_btnRenderVertices);
    hboxLayout->addWidget(_btnRenderEdges);
    hboxLayout->addWidget(_btnRenderFaces);

    _cbWireframe = new QCheckBox("Wireframe");
    _cbWireframe->setEnabled(_subject.GetRenderMode() & NB::RM_FACES);
    _cbNormals = new QCheckBox("Normals");
    _cbWireframe->setEnabled(_subject.GetRenderMode() & NB::RM_FACES);

    _cbShowVertexLabels = new QCheckBox("vertex labels");
    _cbShowVertexLabels->setChecked(_subject.GetVertexLabelsVisible());

    _sbVertexLabelSize = new NBW_SpinBox;
    _sbVertexLabelSize->setText("vertex label size: ");
    _sbVertexLabelSize->setMinimum(0.05f);
    _sbVertexLabelSize->setMaximum(5.00f);
    _sbVertexLabelSize->setSingleStep(0.1f);
	_sbVertexLabelSize->setValue(_subject.GetVertexLabelSize());

    _cbXrayVertexLabels = new QCheckBox("xray labels");
    // ...

    layout->addWidget(_sbVertexScale, 0, 0, 1, 2);

    layout->addWidget(_sbEdgeScale, 1, 0, 1, 2);

    layout->addWidget(lblEdgeTint, 2, 0, 1, 1);
    layout->addWidget(_btnEdgeTint, 2, 1, 1, 1);

    layout->addWidget(lblEdgeShading, 3, 0, 1, 1);
    layout->addWidget(_cbEdgeShading, 3, 1, 1, 1);

    layout->addWidget(_cbHiddenLines, 4, 0, 1, 2);

    layout->addWidget(_sbHullAlpha, 5, 0, 1, 2);

    layout->addWidget(lblRenderMode, 6, 0, 1, 1);
    layout->addLayout(hboxLayout, 6, 1, 1, 1);

    layout->addWidget(_cbWireframe, 7, 0, 1, 1);
    layout->addWidget(_cbNormals, 7, 1, 1, 1);

    layout->addWidget(_cbShowVertexLabels, 8, 0, 1, 2);
    layout->addWidget(_sbVertexLabelSize, 9, 0, 1, 2);
    layout->addWidget(_cbXrayVertexLabels, 10, 0, 1, 2);

    _tabProperties = new QWidget;
	_tabProperties->setLayout(layout);

    QObject::connect(_sbVertexScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnVertexScaleChanged(leda::rational)));
    QObject::connect(_sbEdgeScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnEdgeScaleChanged(leda::rational)));
    QObject::connect(_btnEdgeTint, SIGNAL(SigColorChanged(float, float, float)), this, SLOT(OnEdgeTintChanged(float, float, float)));
    QObject::connect(_cbEdgeShading, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEdgeShadingChanged(int)));
    QObject::connect(_cbHiddenLines, SIGNAL(stateChanged(int)), this, SLOT(OnHiddenLinesChanged(int)));
    QObject::connect(_sbHullAlpha, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnTransparencyChanged(leda::rational)));

    QObject::connect(_btnRenderVertices, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderEdges, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_btnRenderFaces, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));

    QObject::connect(_cbWireframe, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));
    QObject::connect(_cbNormals, SIGNAL(toggled(bool)), this, SLOT(OnRenderModeChanged(bool)));

    QObject::connect(_cbShowVertexLabels, SIGNAL(toggled(bool)), this, SLOT(OnShowVertexLabelsChanged(bool)));
    QObject::connect(_cbXrayVertexLabels, SIGNAL(toggled(bool)), this, SLOT(OnXrayVertexLabelsChanged(bool)));
    QObject::connect(_sbVertexLabelSize, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnVertexLabelSizeChanged(leda::rational)));
}

void ENT_GeometryOutln::InitTransformationTab() {
    QVBoxLayout* vbox = NULL;

    _grpPosition = new QGroupBox("position");
    _grpPosition->setObjectName("vectorGroup");

    vbox = new QVBoxLayout;
    for(int i = 0; i < 3; ++i) {
        _sbPosition[i] = new NBW_SpinBox;
        _sbPosition[i]->setText(QString("%1: ").arg(static_cast<char>('x' + i)));
        // TODO: actually, it's not good that these values are bounded
        _sbPosition[i]->setMinimum(-100);
        _sbPosition[i]->setMaximum( 100);
        _sbPosition[i]->setSingleStep(leda::rational(1, 10));
        connect(_sbPosition[i], SIGNAL(SigValueChanged()), this, SLOT(OnPositionChanged()));
        vbox->addWidget(_sbPosition[i]);
    }
    _grpPosition->setLayout(vbox);

    _grpScale = new QGroupBox("scale");
    _grpScale->setObjectName("vectorGroup");

    vbox = new QVBoxLayout;
    for(int i = 0; i < 3; ++i) {
        _sbScale[i] = new NBW_SpinBox;
        _sbScale[i]->setText(QString("%1: ").arg(static_cast<char>('x' + i)));
        // TODO: actually, it's not good that these values are bounded
        _sbScale[i]->setMinimum(-100);
        _sbScale[i]->setMaximum( 100);
        _sbScale[i]->setSingleStep(leda::rational(1, 10));
        vbox->addWidget(_sbScale[i]);
    }
    _grpScale->setLayout(vbox);

    vbox = new QVBoxLayout;
    vbox->addWidget(_grpPosition);
    vbox->addWidget(_grpScale);
    vbox->addStretch();

    _vectors = new QWidget;
    _vectors->setLayout(vbox);

    vbox = new QVBoxLayout;
    vbox->addWidget(_vectors);

    _tabTransformation = new QWidget;
    _tabTransformation->setLayout(vbox);
}

void ENT_GeometryOutln::InitOutline() {
    InitPropertiesTab();
    InitTransformationTab();

    // create tabwidget
    _tabWidget = new QTabWidget;
    _tabWidget->setObjectName("outlinerTabs");

    QTabBar* tabBar = _tabWidget->findChild<QTabBar *>(QLatin1String("qt_tabwidget_tabbar"));
    tabBar->setObjectName("outlinerTabBar");

    _tabWidget->setTabPosition(QTabWidget::North);
    _tabWidget->addTab(_tabProperties, "Visual");
    _tabWidget->addTab(_tabTransformation, "Transform");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_tabWidget);
    setLayout(layout);
}

void ENT_GeometryOutln::SendEdgeShading() {
    EdgeShadingEvent event;
    int idx = _cbEdgeShading->currentIndex();
    assert(0 <= idx && idx < NB::NUM_SHADING_MODES);
    event.shadingMode = NB::ShadingMode(idx);
    event.showHiddenLines = _cbHiddenLines->isChecked();
    _subject.Send(ev_geom_edgeShadingChanged.Tag(event));
}

void ENT_GeometryOutln::Event_VertexScaleChanged(const EV::Arg<float>& event) {
    _sbVertexScale->blockSignals(true);
    _sbVertexScale->setValue(event.value);
    _sbVertexScale->blockSignals(false);
}

void ENT_GeometryOutln::Event_EdgeScaleChanged(const EV::Arg<float>& event) {
	_sbEdgeScale->blockSignals(true);
	_sbEdgeScale->setValue(event.value);
	_sbEdgeScale->blockSignals(false);
}

void ENT_GeometryOutln::Event_EdgeTintChanged(const EV::Arg<R::Color>& event) {
    const R::Color& edgeTint = event.value;
	_btnEdgeTint->blockSignals(true);
	_btnEdgeTint->SetColor(edgeTint.r, edgeTint.g, edgeTint.b);
	_btnEdgeTint->blockSignals(false);
}

void ENT_GeometryOutln::Event_EdgeShadingChanged(const EdgeShadingEvent& event) {
    UI::BlockSignals blockSignals(_btnRenderVertices, _btnRenderEdges, _btnRenderFaces);
    _cbEdgeShading->setCurrentIndex(event.shadingMode);
    _cbHiddenLines->setChecked(event.showHiddenLines);
}

void ENT_GeometryOutln::Event_RenderModeChanged(const RenderModeEvent& event) {
    UI::BlockSignals blockSignals(_btnRenderVertices, _btnRenderEdges, _btnRenderFaces);
    _btnRenderVertices->setChecked(NB::RM_NODES & event.renderMode);
    _btnRenderEdges->setChecked(NB::RM_EDGES & event.renderMode);
    _btnRenderFaces->setChecked(NB::RM_FACES & event.renderMode);
}

void ENT_GeometryOutln::Event_ShowVertexLabels(const EV::Arg<bool>& event) {
    UI::BlockSignals blockSignals(_cbShowVertexLabels);
    _cbShowVertexLabels->setChecked(event.value);
}

void ENT_GeometryOutln::Event_XrayVertexLabels(const EV::Arg<bool>& event) {
    UI::BlockSignals blockSignals(_cbXrayVertexLabels);
    _cbXrayVertexLabels->setChecked(event.value);
}

void ENT_GeometryOutln::Event_SetVertexLabelSize(const EV::Arg<float>& event) {
    UI::BlockSignals blockSignals(_sbVertexLabelSize);
    _sbVertexLabelSize->setValue(event.value);
}

void ENT_GeometryOutln::ExecEvents(const std::vector<EV::Event>& events) {
    for(unsigned i = 0; i < events.size(); ++i) Send(events[i]);
    HandleEvents();
}

} // namespace W