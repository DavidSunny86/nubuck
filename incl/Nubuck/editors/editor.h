#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\math\plane.h>
#include <Nubuck\events\core_events.h>

namespace W { class ENT_TransformGizmo; }

namespace NB {

// maybe put this somewhere else
enum TransformModeFlags {
    TMF_TRANSLATE   = 1,
    TMF_SCALE       = 2,
    TMF_ALL         = (TMF_TRANSLATE | TMF_SCALE)
};

class Editor {
private:
    int         _axisFlags;

    M::Vector3  _oldCursorPos;

    bool        _dragging;
    int         _dragAxis;
    M::Vector3  _dragOrig;
    M::Plane    _dragPlane;
    M::Vector3  _dragPlanePick;

    W::ENT_TransformGizmo* _gizmo;

    bool OnMouseDown(const EV::MouseEvent& event);
    bool OnMouseUp(const EV::MouseEvent& event);
    bool OnMouseMove(const EV::MouseEvent& event);
protected:
    // queries that subclasses use in On*Dragging() methods
    int                 GetDragAxis() const { return _dragAxis; }
    const M::Vector3&   GetDragOrigin() const { return _dragOrig; }

    // returns point of intersection of drag plane and picking ray
    const M::Vector3&   GetDragPlanePick() const { return _dragPlanePick; }

    float GetTranslation() const { return (_dragPlanePick - _dragOrig).vec[_dragAxis]; }

    float GetScale() const { return M::Length(_dragPlanePick - _oldCursorPos) / M::Length(_dragOrig - _oldCursorPos); }              

    virtual void OnBeginDragging() { }
    virtual void OnDragging() { }
    virtual void OnEndDragging() { }
    virtual bool OnMouseEvent(const EV::MouseEvent& event, bool simulate = false) { return false; }

    virtual bool OnKeyEvent(const EV::KeyEvent& event, bool simulate = false) { return false; }

    const W::ENT_TransformGizmo* GetGizmo() const { return _gizmo; }

    void SetGizmoPosition(const M::Vector3& pos);
    void SetGizmoTransformMode(const NB::TransformGizmoMode mode);
    void SetGizmoVisibility(bool show);
public:
    Editor();

    virtual ~Editor() { }

    void SetAxisFlags(int axisFlags);

    bool HandleMouseEvent(const EV::MouseEvent& event);
    bool HandleKeyEvent(const EV::KeyEvent& event);

    bool SimulateMouseEvent(const EV::MouseEvent& event);
    bool SimulateKeyEvent(const EV::KeyEvent& event);
};

} // namespace NB