#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\math\plane.h>
#include <Nubuck\math\intersections.h>
#include <world\world.h>
#include <world\entity.h>
#include <renderer\mesh\meshmgr.h>

namespace W {

class ENT_TransformGizmo : public Entity {
private:
    typedef NB::TransformGizmoMode      Mode;
    typedef NB::TransformGizmoAction    Action;
    typedef NB::Axis                    Axis;
    typedef NB::TransformGizmoMouseInfo MouseInfo;
    typedef EV::MouseEvent              MouseEvent;

    int     _axis;
    Mode    _mode;

    R::meshPtr_t    _axisMeshes[3];
    R::tfmeshPtr_t  _axisTFMeshes[3];

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

    M::Vector3 _oldCursorPos;

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

    Mode GetTransformMode() const { return _mode; }

    void SetAxis(int axisFlags);
    void SetTransformMode(Mode mode);

    void Show();
    void Hide();
    bool IsHidden() const;

    bool HandleMouseEvent(const MouseEvent& event, MouseInfo& info);

    UI::OutlinerView* CreateOutlinerView() override;

    void GetRenderJobs(R::RenderList& renderList);
};

} // namespace W