#include <Nubuck\editors\editor.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>
#include <world\world.h>

namespace NB {

static M::Vector3 AxisVector(int i) {
    if(0 == i) return M::Vector3(1.0f, 0.0f, 0.0f);
    if(1 == i) return M::Vector3(0.0f, 1.0f, 0.0f);
    if(2 == i) return M::Vector3(0.0f, 0.0f, 1.0f);
    assert(false && "AxisVector(int i): parameter out of range");
    return M::Vector3::Zero;
}

// returns z axis of eye space in world space
static M::Vector3 EyeZ(const M::Matrix4& modelView) {
    M::Matrix3 M = M::RotationOf(modelView);
    float det = M::Det(M);
    if(M::AlmostEqual(0.0f, det)) common.printf("WARNING - modelview matrix is singular\n");
    M::Matrix3 invM = M::Inverse(M, det);
    return M::Transform(invM, M::Vector3(0.0f, 0.0f, 1.0f));
}

bool Editor::OnMouseDown(const EV::MouseEvent& event) {
    COM_assert(_gizmo);

    if(EV::MouseEvent::BUTTON_RIGHT != event.button) return false;

    const M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));

    int         axis;
    M::IS::Info inf;

    if(_gizmo->Trace(ray, axis, &inf)) {
        M::Vector3 eyeZ = EyeZ(W::world.GetModelView());
        M::Vector3 vAxis = AxisVector(axis);
        _dragAxis = axis;
        _dragPlane = M::Plane::FromPointSpan(_gizmo->GetPosition(), M::Cross(eyeZ, vAxis), vAxis);
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        _dragOrig = inf.where;

        unsigned selectionSize = 0;
        W::Entity* ent = W::world.FirstSelectedEntity();
        while(ent) {
            selectionSize++;
            ent = W::world.NextSelectedEntity(ent);
        }

        _oldCursorPos = _gizmo->GetPosition();
        _dragging = true;

        OnBeginDragging();

        return true;
    }

    return false;
}

bool Editor::OnMouseUp(const EV::MouseEvent& event) {
    if(_dragging && EV::MouseEvent::BUTTON_RIGHT == event.button) {
        _dragging = false;
        OnEndDragging();
    }
    return false;
}

bool Editor::OnMouseMove(const EV::MouseEvent& event) {
    if(_dragging) {
        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        M::IS::Info inf;
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        _dragPlanePick = inf.where;
        OnDragging();

        return true;
    }

    return false;
}

void Editor::SetGizmoPosition(const M::Vector3& pos) {
    _gizmo->SetPosition(pos);
}

void Editor::SetGizmoTransformMode(const NB::TransformGizmoMode mode) {
    _gizmo->SetTransformMode(mode);
}

void Editor::SetGizmoVisibility(bool show) {
    if(show) _gizmo->Show();
    else _gizmo->Hide();
}

Editor::Editor()
    : _axisFlags(NB::AF_XYZ)
    , _dragging(false)
    , _gizmo(NULL)
{
    _gizmo = W::world.CreateTransformGizmo();
    _gizmo->Hide();
}

Editor::~Editor() {
    _gizmo->Destroy();
}

void Editor::SetAxisFlags(int axisFlags) {
    _axisFlags = axisFlags;
    _gizmo->SetAxis(_axisFlags);
}

bool Editor::HandleMouseEvent(const EV::MouseEvent& event) {
    bool retVal = false;

    switch(event.type) {
    case EV::MouseEvent::MOUSE_DOWN: retVal = OnMouseDown(event); break;
    case EV::MouseEvent::MOUSE_UP: retVal = OnMouseUp(event); break;
    case EV::MouseEvent::MOUSE_MOVE: retVal = OnMouseMove(event); break;
    }

    // cool application of short-circuit evaluation. I'm such a good programmer...
    return retVal || OnMouseEvent(event);
}

bool Editor::HandleKeyEvent(const EV::KeyEvent& event) {
    return OnKeyEvent(event);
}

bool Editor::SimulateMouseEvent(const EV::MouseEvent& event) {
    if(EV::MouseEvent::BUTTON_RIGHT != event.button) return false;
    const M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
    int axis;
    return _gizmo->Trace(ray, axis) || OnMouseEvent(event, true);
}

bool Editor::SimulateKeyEvent(const EV::KeyEvent& event) {
    return OnKeyEvent(event, true);
}

} // namespace NB