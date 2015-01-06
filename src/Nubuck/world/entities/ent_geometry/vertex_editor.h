#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator_events.h>

namespace W {
    class ENT_Geometry;
    class ENT_TransformGizmo;
};

// TODO: put this somewhere else
enum {
    AXIS_X = 1,
    AXIS_Y = 2,
    AXIS_Z = 4,

    AXIS_XYZ = AXIS_X | AXIS_Y | AXIS_Z
};

class VertexEditor {
private:
    int                                 _axis;

    leda::node_map<M::Vector3>          _oldVertPosF;
    leda::node_map<leda::d3_rat_point>  _oldVertPosR;
    M::Vector3                          _center; // of selection

    W::ENT_TransformGizmo*              _gizmo;

    bool VertexEditor::DoPicking(const OP::MouseEvent& event, W::ENT_Geometry& geom);
    void OnBeginDragging(W::ENT_Geometry& geom);
    void OnDragging(const Nubuck::transform_gizmo_mouse_info& info, W::ENT_Geometry& geom);
public:
    VertexEditor();

    void SetAxis(int axisFlags);

    bool HandleMouseEvent(const OP::MouseEvent& event, W::ENT_Geometry& geom);
};