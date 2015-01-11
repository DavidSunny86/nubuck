#include <renderer\mesh\cone\cone.h>
#include <renderer\mesh\sphere\sphere.h>
#include <nubuck_private.h>
#include "ent_transform_gizmo.h"

// TODO: put this somewhere else
enum Axis {
    AXIS_X = 1,
    AXIS_Y = 2,
    AXIS_Z = 4,

    AXIS_XYZ = AXIS_X | AXIS_Y | AXIS_Z
};

namespace W {

struct AxisMesh {
    std::vector<R::Mesh::Vertex>    vertices;
    std::vector<R::Mesh::Index>     indices;

    AxisMesh(
        int axis,
        float size,
        int subdiv,
        float spacing,
        const R::Color& col)
    {
        COM_assert(0 <= axis && axis < 3);

        unsigned idxCnt = 0;

        unsigned N = (1 << subdiv);
        float f = size / N;
        M::Vector3 d = M::Vector3::Zero;
        d.vec[axis] = 1.0f;

        for(int j = 0; j < N; ++j) {
            R::Mesh::Vertex vert;
            vert.position = f * d * j;
            vert.color = col;
            vertices.push_back(vert);
            indices.push_back(idxCnt++);

            M::Vector3 p = f * d * (j + 1) - spacing * f * d;
            vert.position = p;
            vert.color = col;
            vertices.push_back(vert);
            indices.push_back(idxCnt++);
        }
    }

    R::Mesh::Desc GetDesc() {
        R::Mesh::Desc desc;
        desc.vertices = &vertices[0];
        desc.numVertices = vertices.size();
        desc.indices = &indices[0];
        desc.numIndices = indices.size();
        desc.primType = GL_LINES;
        return desc;
    }
};

void ENT_TransformGizmo::BuildAxis() {
    R::Color colors[] = { R::Color::Red, R::Color::Blue, R::Color::Green };
    for(int i = 0; i < 3; ++i) {
        AxisMesh axisDesc(i, 1.0f, 4, 0.4f, colors[i]);
        _axisMeshes[i] = R::meshMgr.Create(axisDesc.GetDesc());
        _axisTFMeshes[i] = R::meshMgr.Create(_axisMeshes[i]);
    }
}

void ENT_TransformGizmo::BuildArrowHeads() {
    R::Cone arrowHead(0.1f, 0.5f, 20, R::Color::Red);
    R::meshPtr_t meshPtr = _arrowHeadMeshes[0] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Red).GetDesc());
    _arrowHeadTFMeshes[0] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[0] = M::Mat4::Translate(1.0f, 0.0f, 0.0f) * M::Mat4::RotateY( 90.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[0]).SetTransform(_arrowHeadTF[0]);

    meshPtr = _arrowHeadMeshes[1] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Blue).GetDesc());
    _arrowHeadTFMeshes[1] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[1] = M::Mat4::Translate(0.0f, 1.0f, 0.0f) * M::Mat4::RotateX(-90.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[1]).SetTransform(_arrowHeadTF[1]);

    meshPtr = _arrowHeadMeshes[2] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Green).GetDesc());
    _arrowHeadTFMeshes[2] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[2] = M::Mat4::Translate(0.0f, 0.0f, 1.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[2]).SetTransform(_arrowHeadTF[2]);
}

void ENT_TransformGizmo::BuildBoxHeads() {
    const unsigned subdiv = 4;

    R::meshPtr_t meshPtr;

    R::Sphere sphere(subdiv, true);
    sphere.Scale(0.1f);

    sphere.SetColor(R::Color::Red);
    meshPtr = _boxHeadMeshes[0] = R::meshMgr.Create(sphere.GetDesc());
    _boxHeadTFMeshes[0] = R::meshMgr.Create(meshPtr);
    _boxHeadTF[0] = M::Mat4::Translate(1.0f, 0.0f, 0.0f);
    R::meshMgr.GetMesh(_boxHeadTFMeshes[0]).SetTransform(_boxHeadTF[0]);

    sphere.SetColor(R::Color::Blue);
    meshPtr = _boxHeadMeshes[1] = R::meshMgr.Create(sphere.GetDesc());
    _boxHeadTFMeshes[1] = R::meshMgr.Create(meshPtr);
    _boxHeadTF[1] = M::Mat4::Translate(0.0f, 1.0f, 0.0f);
    R::meshMgr.GetMesh(_boxHeadTFMeshes[1]).SetTransform(_boxHeadTF[1]);

    sphere.SetColor(R::Color::Green);
    meshPtr = _boxHeadMeshes[2] = R::meshMgr.Create(sphere.GetDesc());
    _boxHeadTFMeshes[2] = R::meshMgr.Create(meshPtr);
    _boxHeadTF[2] = M::Mat4::Translate(0.0f, 0.0f, 1.0f);
    R::meshMgr.GetMesh(_boxHeadTFMeshes[2]).SetTransform(_boxHeadTF[2]);
}

ENT_TransformGizmo::ENT_TransformGizmo()
    : _axis(AXIS_XYZ)
    , _mode(NB::TGM_TRANSLATE)
    , _dragging(false)
{
    SetType(EntityType::ENT_TRANSFORM_GIZMO);
    SetSolid(false);

    _cursorPos = M::Vector3::Zero;
    // _hidden = true;
    _hidden = false;

    BuildAxis();
    BuildArrowHeads();
    BuildBoxHeads();
}

void ENT_TransformGizmo::BuildBBoxes() {
    const float l = 1.2f;
    const float w = 0.4f;
    _bboxes[X] = M::Box::FromCenterSize(M::Vector3(0.5f, 0.0f, 0.0f), M::Vector3(l, w, w));
    _bboxes[Y] = M::Box::FromCenterSize(M::Vector3(0.0f, 0.5f, 0.0f), M::Vector3(w, l, w));
    _bboxes[Z] = M::Box::FromCenterSize(M::Vector3(0.0f, 0.0f, 0.5f), M::Vector3(w, w, l));
}

void ENT_TransformGizmo::HideCursor() {
    _hidden = true;
}

bool ENT_TransformGizmo::IsHidden() const {
    return _hidden || !g_showRenderViewControls;
}

void ENT_TransformGizmo::ShowCursor() {
    _hidden = false;
}

static void SetCenterPosition(M::Box& box, const M::Vector3& center) {
    const M::Vector3 oldCenter = 0.5f * (box.max - box.min) + box.min;
    const M::Vector3 d = (center - oldCenter);
    box.min += d;
    box.max += d;
}

void ENT_TransformGizmo::SetRenderPosition(const M::Vector3& pos) {
    M::Matrix4 T = M::Mat4::Translate(pos);
    for(int i = 0; i < DIM; ++i) {
        R::meshMgr.GetMesh(_axisTFMeshes[i]).SetTransform(T);
        R::meshMgr.GetMesh(_arrowHeadTFMeshes[i]).SetTransform(T * _arrowHeadTF[i]);
        R::meshMgr.GetMesh(_boxHeadTFMeshes[i]).SetTransform(T * _boxHeadTF[i]);
    }
    for(unsigned i = 0; i < DIM; ++i) {
        SetCenterPosition(_bboxes[i], pos);
        const float l = 1.2f;
        const float w = 0.2f;
        _bboxes[X] = M::Box::FromCenterSize(pos + M::Vector3(0.5f, 0.0f, 0.0f), M::Vector3(l, w, w));
        _bboxes[Y] = M::Box::FromCenterSize(pos + M::Vector3(0.0f, 0.5f, 0.0f), M::Vector3(w, l, w));
        _bboxes[Z] = M::Box::FromCenterSize(pos + M::Vector3(0.0f, 0.0f, 0.5f), M::Vector3(w, w, l));
    }
}

/*
====================
AlignWithCamera
    moves vector v at constant distance to camera.
    note: worldToEye = modelview matrix
====================
*/
static M::Vector3 AlignWithCamera(const M::Matrix4& worldToEye, const M::Vector3& v) {
    M::Matrix4 eyeToWorld;
    bool r = true;
    r = M::TryInvert(worldToEye, eyeToWorld);
    COM_assert(r);
    M::Vector3 eye = M::Transform(eyeToWorld, M::Vector3::Zero); // eye pos in world space
    const float c = 10.0f;
    M::Vector3 d = v - eye;
    M::Matrix4 M = M::Mat4::Translate(-(d.Length() - c) * M::Normalize(d));
    return M::Transform(M, v);
}

void ENT_TransformGizmo::SetAxis(int axisFlags) {
    _axis = axisFlags;
    _dragging = false;
}

void ENT_TransformGizmo::SetTransformMode(Mode mode) {
    _mode = mode;
    _dragging = false;
}

void ENT_TransformGizmo::SetCursorPosition(const M::Vector3& pos) {
    _cursorPos = pos;
    M::Vector3 renderPos = AlignWithCamera(W::world.GetModelView(), _cursorPos);
    SetRenderPosition(renderPos);
}

static M::Vector3 AxisVector(int i) {
    if(0 == i) return M::Vector3(1.0f, 0.0f, 0.0f);
    if(1 == i) return M::Vector3(0.0f, 1.0f, 0.0f);
    if(2 == i) return M::Vector3(0.0f, 0.0f, 1.0f);
    assert(false && "AxisVector(int i): parameter out of range");
    return M::Vector3::Zero;
}

bool ENT_TransformGizmo::TraceCursor(const M::Ray& ray, int& axis, M::IS::Info* inf) {
    for(int i = 0; i < DIM; ++i) {
        if(M::IS::Intersects(ray, _bboxes[i], inf)) {
            axis = i;
            return true;
        }
    }
    return false;
}

// returns z axis of eye space in world space
static M::Vector3 EyeZ(const M::Matrix4& modelView) {
    M::Matrix3 M = M::RotationOf(modelView);
    float det = M::Det(M);
    if(M::AlmostEqual(0.0f, det)) common.printf("WARNING - modelview matrix is singular\n");
    M::Matrix3 invM = M::Inverse(M, det);
    return M::Transform(invM, M::Vector3(0.0f, 0.0f, 1.0f));
}

bool ENT_TransformGizmo::OnMouseDown(const MouseEvent& event, MouseInfo& info) {
    // while dragging, a mousedown event is 'handled' by being ignored
    if(_dragging) return true;

	if(MouseEvent::BUTTON_RIGHT != event.button) return false;

    M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));

    int         axis;
    M::IS::Info inf;

    if(TraceCursor(ray, axis, &inf)) {
        M::Vector3 eyeZ = EyeZ(W::world.GetModelView());
        M::Vector3 vAxis = AxisVector(axis);
        _dragAxis = axis;
        _dragPlane = M::Plane::FromPointSpan(_cursorPos, M::Cross(eyeZ, vAxis), vAxis);
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        _dragOrig = inf.where;

        unsigned selectionSize = 0;
        W::Entity* ent = W::world.FirstSelectedEntity();
        while(ent) {
            selectionSize++;
            ent = W::world.NextSelectedEntity(ent);
        }

        _oldCursorPos = _cursorPos;
        _dragging = true;

        info.action = static_cast<Action>(NB::TGA_BEGIN_DRAGGING);
        info.axis = static_cast<Axis>(axis);

        return true;
    }

    return false;
}

bool ENT_TransformGizmo::OnMouseUp(const MouseEvent& event, MouseInfo& info) {
    if(_dragging) {
        _dragging = false;
        info.action = NB::TGA_END_DRAGGING;
        return true;
    }
    return false;
}

static bool lastDrag = false;

bool ENT_TransformGizmo::OnMouseMove(const MouseEvent& event, MouseInfo& info) {
    lastDrag = _dragging;

    if(_dragging && NB::TGM_TRANSLATE == _mode) {
        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        M::IS::Info inf;
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        M::Vector3 p = inf.where;
        M::Vector3 pos = _cursorPos;
        const float translation = (p - _dragOrig).vec[_dragAxis];
        pos.vec[_dragAxis] = _oldCursorPos.vec[_dragAxis] + translation;
        SetCursorPosition(pos);

        info.action = NB::TGA_DRAGGING;
        info.axis   = Axis(_dragAxis);
        info.value  = translation;

        return true;
    }
    if(_dragging && NB::TGM_SCALE == _mode) {
        M::Vector2 mousePos = M::Vector2(event.x, event.y);
        float base = M::Length(_dragOrig - _oldCursorPos);

        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        M::IS::Info inf;
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        M::Vector3 p = inf.where;

        const float scale = M::Length(p - _oldCursorPos) / base;

        info.action = NB::TGA_DRAGGING;
        info.axis   = Axis(_dragAxis);
        info.value  = scale;

        return true;
    }
    return false;
}

bool ENT_TransformGizmo::HandleMouseEvent(const MouseEvent& event, MouseInfo& info) {
    info.action = NB::TGA_NO_ACTION;
    info.axis   = NB::INVALID_AXIS;
    info.value  = 0.0f;

    if(IsHidden()) return false;

	switch(event.type) {
	case MouseEvent::MOUSE_DOWN:  return OnMouseDown(event, info);
	case MouseEvent::MOUSE_UP:    return OnMouseUp(event, info);
	case MouseEvent::MOUSE_MOVE:  return OnMouseMove(event, info);
	}

    return false;
}

UI::OutlinerView* ENT_TransformGizmo::CreateOutlinerView() {
    assert(0 && "never called");
    return 0;
}

void ENT_TransformGizmo::GetRenderJobs(R::RenderList& renderList) {
    if(IsHidden()) return;

    SetCursorPosition(_cursorPos); // updates renderpos

    R::MeshJob meshJob;

    meshJob.fx = "UnlitThickLines";
    meshJob.layer = R::Renderer::Layers::GEOMETRY_1;
    meshJob.material = R::Material::White;
    meshJob.primType = 0;
    for(int i = 0; i < 3; ++i) {
        if(_axis & (1 << i)) {
            meshJob.tfmesh = _axisTFMeshes[i];
            renderList.meshJobs.push_back(meshJob);
        }
    }

    meshJob.fx = "LitDirectional";
    meshJob.layer = R::Renderer::Layers::GEOMETRY_1;
    meshJob.material = R::Material::White;
    meshJob.primType = 0;
    for(int i = 0; i < 3; ++i) {
        if(_axis & (1 << i)) {
            switch(_mode) {
            case NB::TGM_TRANSLATE:
                meshJob.tfmesh = _arrowHeadTFMeshes[i];
                break;
            case NB::TGM_SCALE:
                meshJob.tfmesh = _boxHeadTFMeshes[i];
                break;
            }
            renderList.meshJobs.push_back(meshJob);
        }
    }
}

} // namespace W