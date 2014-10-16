#pragma once

#include <QComboBox>
#include <QCheckBox>

#include <UI\outliner\outliner.h>
#include <Nubuck\events\events.h>
#include "ent_geometry_events.h"

class NBW_SpinBox;

namespace W {

class ENT_Geometry;

class ENT_GeometryOutln : public UI::OutlinerView {
    Q_OBJECT
private:
    ENT_Geometry& _subject;

    NBW_SpinBox*        _sbVertexScale;
    NBW_SpinBox*        _sbEdgeScale;
    UI::ColorButton*    _btnEdgeColor;
    NBW_SpinBox*        _sbHullAlpha;

    QPushButton*        _btnRenderVertices;
    QPushButton*        _btnRenderEdges;
    QPushButton*        _btnRenderFaces;

    QComboBox*          _cbEdgeShading;
    QCheckBox*          _cbHiddenLines;

    QCheckBox*          _cbWireframe;
    QCheckBox*          _cbNormals;

    void InitOutline();

    void SendEdgeShading();

    void Event_VertexScaleChanged(const EV::Event& event);
    void Event_EdgeScaleChanged(const EV::Event& event);
    void Event_EdgeColorChanged(const EV::Event& event);
    void Event_EdgeShadingChanged(const EV::Event& event);
    void Event_RenderModeChanged(const EV::Event& event);
private slots:
    void OnVertexScaleChanged(leda::rational value);
    void OnEdgeScaleChanged(leda::rational value);
    void OnEdgeColorChanged(float r, float g, float b);
    void OnTransparencyChanged(leda::rational value);
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