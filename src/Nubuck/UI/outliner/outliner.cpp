#include <events\event_defs.h>
#include <UI\logwidget\logwidget.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "outliner.h"

namespace UI {

void Outliner::EntityOutline_Init(EntityOutline& outln) {
    outln.widget = new QWidget();
    outln.layout = new QGridLayout();
    outln.widget->setLayout(outln.layout);
}

void PolyhedronOutline::OnEdgeColorChanged(float r, float g, float b) {
    EV::Params_SetEdgeBaseColor args;
    args.entId  = entId;
    args.color  = R::Color(r, g, b);
    W::world.Send(EV::def_SetEdgeBaseColor.Create(args));
}

void PolyhedronOutline::OnEdgeRadiusChanged(double value) {
    EV::Params_SetEdgeRadius args;
    args.entId  = entId;
    args.radius = value;
    W::world.Send(EV::def_SetEdgeRadius.Create(args));
}

void PolyhedronOutline::OnHullAlphaChanged(int value) {
    float alpha = value / 100.0f;
    EV::Params_SetHullAlpha args;
    args.entId  = entId;
    args.alpha  = alpha;
    W::world.Send(EV::def_SetHullAlpha.Create(args));
}

void Outliner::PolyhedronOutline_Init(EntityOutline& outln) {
    outln.polyhedron = new PolyhedronOutline;

    QLabel* lblEdgeRadius = new QLabel("edge radius:");
    QDoubleSpinBox* sbEdgeRadius = new QDoubleSpinBox;
    sbEdgeRadius->setMinimum(0.05f);
    sbEdgeRadius->setMaximum(5.00f);
    sbEdgeRadius->setSingleStep(0.1f);

    QLabel* lblEdgeColor = new QLabel("edge color:");
    ColorButton* btnEdgeColor = new ColorButton;

    QLabel* lblHullAlpha = new QLabel("hull alpha:");
    QSlider* sldHullAlpha = new QSlider(Qt::Horizontal);
    sldHullAlpha->setTracking(true);
    sldHullAlpha->setMinimum(0);
    sldHullAlpha->setMaximum(100);
    sldHullAlpha->setValue(100);

    outln.layout->addWidget(lblEdgeRadius, 0, 0, 1, 1);
    outln.layout->addWidget(sbEdgeRadius, 0, 1, 1, 1);

    outln.layout->addWidget(lblEdgeColor, 1, 0, 1, 1);
    outln.layout->addWidget(btnEdgeColor, 1, 1, 1, 1);

    outln.layout->addWidget(lblHullAlpha, 2, 0, 1, 1);
    outln.layout->addWidget(sldHullAlpha, 2, 1, 1, 1);

    outln.polyhedron->entId = outln.entId;
    outln.polyhedron->sbEdgeRadius = sbEdgeRadius;
    outln.polyhedron->btnEdgeColor = btnEdgeColor;

    connect(btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), outln.polyhedron, SLOT(OnEdgeColorChanged(float, float, float)));
    connect(sbEdgeRadius, SIGNAL(valueChanged(double)), outln.polyhedron, SLOT(OnEdgeRadiusChanged(double)));
    connect(sldHullAlpha, SIGNAL(valueChanged(int)), outln.polyhedron, SLOT(OnHullAlphaChanged(int)));
}

Outliner::EntityOutline* Outliner::FindByEntityID(unsigned entId) {
    for(unsigned i = 0; i < _outlines.size(); ++i) {
        if(entId == _outlines[i].entId) return &_outlines[i];
    }
    return NULL;
}

BEGIN_EVENT_HANDLER(Outliner)
    EVENT_HANDLER(EV::def_SpawnPolyhedron,  &Outliner::Event_SpawnPolyhedron)
    EVENT_HANDLER(EV::def_SetName,          &Outliner::Event_SetName)
    EVENT_HANDLER(EV::def_EntityInfo,       &Outliner::Event_EntityInfo)
END_EVENT_HANDLER

void Outliner::Event_SpawnPolyhedron(const EV::Event& event) {
    const EV::Params_SpawnPolyhedron& args =  EV::def_SpawnPolyhedron.GetArgs(event);
    EntityOutline outln;
    outln.entId = args.entId;
    EntityOutline_Init(outln);
    outln.type = W::EntityType::ENT_POLYHEDRON;
    PolyhedronOutline_Init(outln);

    QTreeWidgetItem* headerIt = new QTreeWidgetItem;
    _treeWidget->insertTopLevelItem(0, headerIt);
    QPushButton* button = new QPushButton(QString("Polyhedron %1").arg(args.entId));
    button->setStyleSheet("text-align: left");
    _treeWidget->setItemWidget(headerIt, 0, button);
    outln.headerWidget = button;

    QTreeWidgetItem* contentIt = new QTreeWidgetItem;
    headerIt->addChild(contentIt);
    _treeWidget->setItemWidget(contentIt, 0, outln.widget);

    _outlines.push_back(outln);
}

void Outliner::Event_SetName(const EV::Event& event) {
    const EV::Params_SetName& args = EV::def_SetName.GetArgs(event);
    EntityOutline* outln = FindByEntityID(args.entId);
    assert(outln);
    outln->headerWidget->setText(QString("Polyhedron %1 \"%2\"").arg(args.entId).arg(args.name));
}

void Outliner::Event_EntityInfo(const EV::Event& event) {
    const EV::Params_EntityInfo& args = EV::def_EntityInfo.GetArgs(event);

    W::EntityInf* inf = args.inf;
    EntityOutline* outln = FindByEntityID(inf->entId);
    assert(outln);
    assert(outln->type == args.entType);

    if(W::EntityType::ENT_POLYHEDRON == args.entType) {
        W::INF_Polyhedron* phInf = (W::INF_Polyhedron*)inf->inf;
        outln->polyhedron->sbEdgeRadius->setValue(phInf->edgeRadius);
        outln->polyhedron->btnEdgeColor->SetColor(phInf->edgeColor.r, phInf->edgeColor.g, phInf->edgeColor.b);
    }
}

Outliner::Outliner(QWidget* parent) : QWidget(parent) {
    _treeWidget = new QTreeWidget(this);
    _treeWidget->setHeaderHidden(true);
    
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_treeWidget);
    setLayout(layout);
}

Outliner* Outliner::Instance(void) {
    static Outliner* instance = NULL;
    if(!instance) instance = new Outliner();
    return instance;
}

} // namespace UI