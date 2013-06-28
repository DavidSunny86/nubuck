#include "arcball_camera.h"

namespace {

    const float zoomStep = 2.0f;

} // unnamed namespace

M::Vector3 ArcballCamera::VectorOnSphereFromMousePos(int mouseX, int mouseY) {
    float x = mouseX - _halfWidth;
    float y = -mouseY + _halfHeight;
    const float r2 = _radius * _radius;
    const float len2 = x * x + y * y;
    M::Vector3 ret;
    if(len2 > r2) {
        ret = M::Normalize(M::Vector3(x, y, 0.0f));
    } else {
        const float z = sqrt(r2 - len2);
        ret = M::Normalize(M::Vector3(x, y, z));
    }
    return ret;
}

M::Vector3 ArcballCamera::VectorInPlaneFromMousePos(int mouseX, int mouseY) {
    const float f = 0.5f;

    float x = (mouseX - _halfWidth) / (f * _halfWidth);
    float y = (-mouseY + _halfHeight) / (f * _halfHeight);
    return M::Vector3(x, y, 0.0f);
}

/*
shoot a ray originating in camera through projection plane onto plane that 
is parallel to the proj. plane and passes through the origin of the world 
coordinate system. intersection point is used as translation vector.
*/
M::Vector3 ArcballCamera::VectorInXYPlaneFromMousePos(int mouseX, int mouseY) {
    // hardcoded values
    const float fovy = M::Deg2Rad(45.0f); // hardcoded in renderer.cpp
    const float dist = -10.0f; // default camera distance, hardcoded in view.cpp

    const float aspect = _halfWidth / _halfHeight;

    // size of projection plane.
    // assuming z = 1.0f, hardcoded in renderer.cpp
    const float h = 2.0f * tanf(0.5f * fovy);
    const float w = h * aspect;

    const float l1 = (float)mouseX / (2.0f * _halfWidth);
    const float l2 = -1.0f * mouseY / (2.0f * _halfHeight);

    const float x = (l1 - 0.5f) * w;
    const float y = (l2 - 0.5f) * h;

    // ray = (origin, direction) = (0, (x, y, 1))
    // intersects XY plane at l
    const float l = -1.0f * (_transform.trans.z + dist);

    // intersection point
    return M::Vector3(x * l, y * l, 0.0f);
}

ArcballCamera::ArcballCamera(int width, int height)
    : _dragging(false),
      _panning(false),
      _dragRot(M::Quat::Identity()),
      _panTrans(M::Vector3::Zero)
{
    _transform.trans	= M::Vector3::Zero;
    _transform.rot		= M::Quat::Identity();
    _transform.scale	= 1.0f;

    SetScreenSize(width, height);
}

void ArcballCamera::Reset(void) {
    _panning = false;
    _dragging = false;

    _transform.trans	= M::Vector3::Zero;
    _transform.rot		= M::Quat::Identity();
    _transform.scale	= 1.0f;

    _dragRot	= M::Quat::Identity();
    _panTrans	= M::Vector3::Zero;
}

void ArcballCamera::ZoomIn(void) {
    _transform.trans.z += zoomStep;
}

void ArcballCamera::ZoomOut(void) {
    _transform.trans.z -= zoomStep;
}

void ArcballCamera::SetScreenSize(int width, int height) {
    _halfWidth = width / 2.0f;
    _halfHeight = height / 2.0f;
    _radius = M::Min(_halfWidth, _halfHeight);
}

void ArcballCamera::StartDragging(int mouseX, int mouseY) {
    if(!_dragging) {
        _v0 = VectorOnSphereFromMousePos(mouseX, mouseY);
        _dragging = true;
    }
}

bool ArcballCamera::Drag(int mouseX, int mouseY) {
    if(_dragging) {
        const M::Vector3 v1 = VectorOnSphereFromMousePos(mouseX, mouseY);
        if(M::LinearlyDependent(_v0, v1)) return _dragging;
        const M::Vector3 axis = M::Normalize(M::Cross(_v0, v1));
        float angle = acosf(M::Clamp(-1.0f, M::Dot(_v0, v1), 1.0f));
        _dragRot = _dragRot * M::Quat::RotateAxis(-axis, M::Rad2Deg(angle));
        _v0 = v1;
    }
    return _dragging;
}

void ArcballCamera::StopDragging(void) {
    _dragging = false;
    _transform.rot = _dragRot * _transform.rot;
    _dragRot = M::Quat::Identity();
}

void ArcballCamera::StartPanning(int mouseX, int mouseY) {
    if(!_panning) {
        _v0 = VectorInXYPlaneFromMousePos(mouseX, mouseY);
        _panning = true;
    }
}

bool ArcballCamera::Pan(int mouseX, int mouseY) {
    if(_panning) {
        const M::Vector3 v1 = VectorInXYPlaneFromMousePos(mouseX, mouseY);
        _panTrans = v1 - _v0;
    }
    return _panning;
}

void ArcballCamera::StopPanning(void) {
    _panning = false;
    _transform.trans = _panTrans + _transform.trans;
    _panTrans = M::Vector3::Zero;
}

const M::TransformTRS& ArcballCamera::GetTransform(void) const {
    return _transform;
}

M::Quaternion ArcballCamera::GetRotation(void) const {
    if(_dragging) return _dragRot * _transform.rot;
    return _transform.rot;
}

M::Matrix4 ArcballCamera::GetWorldMatrix(void) const {
    if(_dragging) return M::Mat4::FromTransform(_dragRot * _transform.rot, _transform.trans, _transform.scale);
    if(_panning) return M::Mat4::FromTransform(_transform.rot, _panTrans + _transform.trans, _transform.scale);
    return M::Mat4::FromTransform(_transform.rot, _transform.trans, _transform.scale);
}

void ArcballCamera::Render(void) {
}
