#pragma once

#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>

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

    QTabWidget*         _tabWidget;
    QWidget*            _tabProperties;
    QWidget*            _tabTransformation;

    // properties tab

    NBW_SpinBox*        _sbVertexScale;
    NBW_SpinBox*        _sbEdgeScale;
    UI::ColorButton*    _btnEdgeTint;
    NBW_SpinBox*        _sbHullAlpha;

    QPushButton*        _btnRenderVertices;
    QPushButton*        _btnRenderEdges;
    QPushButton*        _btnRenderFaces;

    QComboBox*          _cbEdgeShading;
    QCheckBox*          _cbHiddenLines;

    QCheckBox*          _cbWireframe;
    QCheckBox*          _cbNormals;

    QCheckBox*          _cbShowVertexLabels;
    NBW_SpinBox*        _sbVertexLabelSize;
    QCheckBox*          _cbXrayVertexLabels;

    // transformation tab

    QWidget*        _vectors;       // contains _grpPosition, _grpScale
    QGroupBox*      _grpPosition;
    QGroupBox*      _grpScale;
    NBW_SpinBox*    _sbPosition[3]; // xyz
    NBW_SpinBox* 	_sbScale[3];    // xyz

    void InitPropertiesTab();
    void InitTransformationTab();
    void InitOutline();

    void SendEdgeShading();

    void Event_VertexScaleChanged(const EV::Arg<float>& event);
    void Event_EdgeScaleChanged(const EV::Arg<float>& event);
    void Event_EdgeTintChanged(const EV::Arg<R::Color>& event);
    void Event_EdgeShadingChanged(const EdgeShadingEvent& event);
    void Event_RenderModeChanged(const RenderModeEvent& event);
    void Event_ShowVertexLabels(const EV::Arg<bool>& event);
    void Event_XrayVertexLabels(const EV::Arg<bool>& event);
    void Event_SetVertexLabelSize(const EV::Arg<float>& event);

    void Event_SetPosition(const SetEntityVectorEvent& event);
private slots:
    void OnVertexScaleChanged(leda::rational value);
    void OnEdgeScaleChanged(leda::rational value);
    void OnEdgeTintChanged(float r, float g, float b);
    void OnTransparencyChanged(leda::rational value);
    void OnRenderModeChanged(bool checked);
    void OnEdgeShadingChanged(int idx);
    void OnHiddenLinesChanged(int state);
    void OnShowVertexLabelsChanged(bool checked);
    void OnXrayVertexLabelsChanged(bool checked);
    void OnVertexLabelSizeChanged(leda::rational value);

    void OnPositionChanged(leda::rational);
    void OnScaleChanged(leda::rational);
public:
    ENT_GeometryOutln(ENT_Geometry& subject);

    DECL_HANDLE_EVENTS(ENT_GeometryOutln);

	void InitUI() { InitOutline(); }

    void ExecEvents(const std::vector<EV::Event>& events);
};

} // namespace W