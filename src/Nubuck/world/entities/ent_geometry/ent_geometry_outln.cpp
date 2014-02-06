#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>

#include <UI\colorbutton\colorbutton.h>
#include "ent_geometry.h"

namespace W {

void ENT_Geometry::OnEdgeRadiusChanged(double value) {
    SetEdgeRadius((float)value);
}

void ENT_Geometry::OnEdgeColorChanged(float r, float g, float b) {
    SetEdgeColor(R::Color(r, g, b));
}

void ENT_Geometry::InitOutline() {
	UI::Outliner::Item& item = UI::Outliner::Instance()->GetItem(_outln.item);

	QGridLayout* layout = new QGridLayout();

    QLabel* lblEdgeRadius = new QLabel("edge radius:");
    _outln.sbEdgeRadius = new QDoubleSpinBox;
    _outln.sbEdgeRadius->setMinimum(0.05f);
    _outln.sbEdgeRadius->setMaximum(5.00f);
    _outln.sbEdgeRadius->setSingleStep(0.1f);
	_outln.sbEdgeRadius->setValue(_edgeRadius);

    QLabel* lblEdgeColor = new QLabel("edge color:");
    _outln.btnEdgeColor = new UI::ColorButton;
	_outln.btnEdgeColor->SetColor(_edgeColor.r, _edgeColor.g, _edgeColor.b);

    QLabel* lblHullAlpha = new QLabel("hull alpha:");
    _outln.sldHullAlpha = new QSlider(Qt::Horizontal);
    _outln.sldHullAlpha->setTracking(true);
    _outln.sldHullAlpha->setMinimum(0);
    _outln.sldHullAlpha->setMaximum(100);
    _outln.sldHullAlpha->setValue(100);

    layout->addWidget(lblEdgeRadius, 0, 0, 1, 1);
    layout->addWidget(_outln.sbEdgeRadius, 0, 1, 1, 1);

    layout->addWidget(lblEdgeColor, 1, 0, 1, 1);
    layout->addWidget(_outln.btnEdgeColor, 1, 1, 1, 1);

    layout->addWidget(lblHullAlpha, 2, 0, 1, 1);
    layout->addWidget(_outln.sldHullAlpha, 2, 1, 1, 1);

	item.content->setLayout(layout);

    connect(_outln.sbEdgeRadius, SIGNAL(valueChanged(double)), this, SLOT(OnEdgeRadiusChanged(double)));
    connect(_outln.btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), this, SLOT(OnEdgeColorChanged(float, float, float)));
    /*
    connect(sldHullAlpha, SIGNAL(valueChanged(int)), outln.polyhedron, SLOT(OnHullAlphaChanged(int)));
    */
}

} // namespace W