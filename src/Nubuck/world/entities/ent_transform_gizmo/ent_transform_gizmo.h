#pragma once

#include <Nubuck\math\plane.h>
#include <Nubuck\math\intersections.h>
#include <Nubuck\operators\operator_events.h>
#include <world\world.h>
#include <world\entity.h>
#include <renderer\mesh\meshmgr.h>

namespace W {

class ENT_TransformGizmo : public Entity {
private:
    typedef Nubuck::TransformGizmoMode          Mode;
    typedef Nubuck::transform_gizmo_action      Action;
    typedef Nubuck::transform_gizmo_axis        Axis;
    typedef Nubuck::transform_gizmo_mouse_info  MouseInfo;
    typedef OP::MouseEvent                      MouseEvent;

    Mode::Enum _mode;

    R::meshPtr_t    _axisMesh;
    R::tfmeshPtr_t  _axisTFMesh;

    enum { X = 0, Y, Z, DIM };
    R::meshPtr_t    _arrowHeadMeshes[3];
    R::tfmeshPtr_t  _arrowHeadTFMeshes[3];
    M::Matrix4      _arrowHeadTF[3];

    R::meshPtr_t    _boxHeadMeshes[3];
    R::tfmeshPtr_t  _boxHeadTFMeshes[3];
    M::Matrix4      _boxHeadTF[3];

    void BuildAxis();
    void BuildArrowHeads();
    void BuildBoxHeads();

    M::Box _bboxes[DIM]; // bounding boxes of cursor

    void BuildBBoxes();

    bool _hidden;

    void UpdateCursor();

    void SetRenderPosition(const M::Vector3& pos);

    M::Vector3                      _cursorPos; // use cursorPos as readonly and SetCursorPosition() for writes
    M::Vector3              		_oldCursorPos;

    bool        _dragging;
    int         _dragAxis;
    M::Vector3  _dragOrig;
    M::Plane    _dragPlane;

    bool TraceCursor(const M::Ray& ray, int& axis, M::IS::Info* inf = NULL);

    bool OnMouseDown(const MouseEvent& event, MouseInfo& info);
    bool OnMouseUp(const MouseEvent& event, MouseInfo& info);
    bool OnMouseMove(const MouseEvent& event, MouseInfo& info);
public:
    ENT_TransformGizmo();

    Mode::Enum GetTransformMode() const { return _mode; }

    void SetTransformMode(Mode::Enum mode);

    void SetCursorPosition(const M::Vector3& pos);

    void ShowCursor();
    void HideCursor();

    bool HandleMouseEvent(const MouseEvent& event, MouseInfo& info);

    UI::OutlinerView* CreateOutlinerView() override;

    void GetRenderJobs(R::RenderList& renderList);
};

} // namespace W