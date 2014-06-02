#pragma once

#include <QComboBox>
#include <QCheckBox>

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

    QPushButton*        _btnRenderVertices;
    QPushButton*        _btnRenderEdges;
    QPushButton*        _btnRenderFaces;
    
    QComboBox*          _cbEdgeShading;
    QCheckBox*          _cbHiddenLines;

    void InitOutline();

    void SendEdgeShading();

    void Event_EdgeRadiusChanged(const EV::Event& event);
    void Event_EdgeColorChanged(const EV::Event& event);
    void Event_RenderModeChanged(const EV::Event& event);
private slots:
    void OnEdgeRadiusChanged(double value);
    void OnEdgeColorChanged(float r, float g, float b);
    void OnTransparencyChanged(int value);
    void OnRenderModeChanged(bool checked);
    void OnEdgeShadingChanged(int idx);
    void OnHiddenLinesChanged(int state);
public:
    ENT_GeometryOutln(ENT_Geometry& subject);

    DECL_HANDLE_EVENTS(ENT_GeometryOutln);

	void InitUI() { InitOutline(); }

    void ExecEvents(const std::vector<EV::Event>& events);
};

} // namespace W