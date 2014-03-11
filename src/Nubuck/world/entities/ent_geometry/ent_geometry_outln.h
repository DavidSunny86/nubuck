#pragma once

#include <UI\outliner\outliner.h>
#include <Nubuck\events\events.h>
#include "ent_geometry_events.h"

namespace W {

class ENT_Geometry;

class ENT_GeometryOutln : public UI::OutlinerView {
    Q_OBJECT
private:
    ENT_Geometry& _subject;

    QDoubleSpinBox*     _sbEdgeRadius;
    UI::ColorButton*    _btnEdgeColor;
    QSlider*            _sldHullAlpha;

    void InitOutline();

    void Event_EdgeRadiusChanged(const EV::Event& event);
    void Event_EdgeColorChanged(const EV::Event& event);
private slots:
    void OnEdgeRadiusChanged(double value);
    void OnEdgeColorChanged(float r, float g, float b);
    void OnTransparencyChanged(int value);
public:
    ENT_GeometryOutln(ENT_Geometry& subject);

    DECL_HANDLE_EVENTS(ENT_GeometryOutln);

	void InitUI() { InitOutline(); }

    void ExecEvents(const std::vector<EV::Event>& events);
};

} // namespace W