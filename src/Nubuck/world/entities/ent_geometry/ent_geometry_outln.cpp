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
    // SetEdgeRadius((float)value);
}

void ENT_GeometryOutln::OnEdgeColorChanged(float r, float g, float b) {
    // SetEdgeColor(R::Color(r, g, b));
}

ENT_GeometryOutln::ENT_GeometryOutln(ENT_Geometry& subject) : _subject(subject) {
    InitOutline();
}

void ENT_GeometryOutln::InitOutline() {
	QGridLayout* layout = new QGridLayout();

    QLabel* lblEdgeRadius = new QLabel("edge radius:");
    _sbEdgeRadius = new QDoubleSpinBox;
    _sbEdgeRadius->setMinimum(0.05f);
    _sbEdgeRadius->setMaximum(5.00f);
    _sbEdgeRadius->setSingleStep(0.1f);
	// _sbEdgeRadius->setValue(_edgeRadius);

    QLabel* lblEdgeColor = new QLabel("edge color:");
    _btnEdgeColor = new UI::ColorButton;
	// _btnEdgeColor->SetColor(_edgeColor.r, _edgeColor.g, _edgeColor.b);

    QLabel* lblHullAlpha = new QLabel("hull alpha:");
    _sldHullAlpha = new QSlider(Qt::Horizontal);
    _sldHullAlpha->setTracking(true);
    _sldHullAlpha->setMinimum(0);
    _sldHullAlpha->setMaximum(100);
    _sldHullAlpha->setValue(100);

    layout->addWidget(lblEdgeRadius, 0, 0, 1, 1);
    layout->addWidget(_sbEdgeRadius, 0, 1, 1, 1);

    layout->addWidget(lblEdgeColor, 1, 0, 1, 1);
    layout->addWidget(_btnEdgeColor, 1, 1, 1, 1);

    layout->addWidget(lblHullAlpha, 2, 0, 1, 1);
    layout->addWidget(_sldHullAlpha, 2, 1, 1, 1);

	setLayout(layout);

    QObject::connect(_sbEdgeRadius, SIGNAL(valueChanged(double)), this, SLOT(OnEdgeRadiusChanged(double)));
    QObject::connect(_btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), this, SLOT(OnEdgeColorChanged(float, float, float)));
    /*
    connect(sldHullAlpha, SIGNAL(valueChanged(int)), outln.polyhedron, SLOT(OnHullAlphaChanged(int)));
    */
}

} // namespace W