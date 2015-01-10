#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\nubuck_api.h>

namespace W {
    class ENT_Geometry;
    class ENT_TransformGizmo;
};

namespace NB {

class NUBUCK_API VertexEditor {
private:
    int                                 _axis;

    leda::node_map<M::Vector3>          _oldVertPosF;
    leda::node_map<leda::d3_rat_point>  _oldVertPosR;
    M::Vector3                          _center; // of selection

    W::ENT_TransformGizmo*              _gizmo;

    bool VertexEditor::DoPicking(const EV::MouseEvent& event, W::ENT_Geometry& geom);
    void OnBeginDragging(W::ENT_Geometry& geom);
    void OnDragging(const NB::TransformGizmoMouseInfo& info, W::ENT_Geometry& geom);
public:
    VertexEditor();

    void SetAxisFlags(int axisFlags);

    bool HandleMouseEvent(const EV::MouseEvent& event, W::ENT_Geometry* geom);
};

} // namespace NB