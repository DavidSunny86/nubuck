#include "arcball_camera.h"

namespace {

const float zoomStep = 2.0f;

M::Vector3 Transform(const M::Quaternion& q, const M::Vector3& v) {
    // TODO: use q*vq instead
    M::Matrix4 m = M::Mat4::FromRigidTransform(q, M::Vector3::Zero);
    return M::Transform(m, v);
}

M::Vector3 VectorOnSphereFromMousePos(float halfWidth, float halfHeight, float radius, int mouseX, int mouseY) {
    float x = mouseX - halfWidth;
    float y = -mouseY + halfHeight;
    const float r2 = radius * radius;
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

M::Vector3 VectorInPlaneFromMousePos(float halfWidth, float halfHeight, int mouseX, int mouseY) {
    const float f = 0.5f;
    float x = (mouseX - halfWidth) / (f * halfWidth);
    float y = (-mouseY + halfHeight) / (f * halfHeight);
    return M::Vector3(x, y, 0.0f);
}

/*
====================
VectorInXYPlaneFromMousePos
    shoot a ray originating in camera through projection plane onto plane that
    is parallel to the proj. plane and passes through the origin of the world
    coordinate system. intersection point is used as translation vector.
====================
*/
M::Vector3 VectorInXYPlaneFromMousePos(
    float halfWidth, float halfHeight, 
    int mouseX, int mouseY, float dist)
{
    // hardcoded values
    const float fovy = M::Deg2Rad(45.0f); // hardcoded in renderer.cpp

    const float aspect = halfWidth / halfHeight;

    // size of projection plane.
    // assuming z = 1.0f, hardcoded in renderer.cpp
    const float h = 2.0f * tanf(0.5f * fovy);
    const float w = h * aspect;

    const float l1 = (float)mouseX / (2.0f * halfWidth);
    const float l2 = -1.0f * mouseY / (2.0f * halfHeight);

    const float x = (l1 - 0.5f) * w;
    const float y = (l2 - 0.5f) * h;

    // ray = (origin, direction) = (0, (x, y, 1))
    // intersects XY plane at l
    const float l = -dist;

    // intersection point
    return M::Vector3(x * l, y * l, 0.0f);
}

} // unnamed namespace

/*
====================
ArcballCamera::Position
    returns the position of the camera in world space
====================
*/
M::Vector3 ArcballCamera::Position() const {
    return _target + Transform(_orient, _zoom * M::Vector3(0.0f, 0.0f, 1.0f));
}

ArcballCamera::ArcballCamera(int width, int height)
    : _dragging(false)
    , _panning(false)
    , _zooming(false)
    , _target(M::Vector3::Zero)
    , _orient(M::Quat::Identity())
    , _zoom(0.0f)
{
    SetScreenSize(width, height);
}

void ArcballCamera::Reset() {
    _panning = false;
    _dragging = false;
    _zooming = false;

    _target = M::Vector3::Zero;
    _orient = M::Quat::Identity();
    _zoom   = 0.0f;
}

void ArcballCamera::ResetRotation() {
    _panning = false;
    _dragging = false;

    _orient = M::Quat::Identity();
}

void ArcballCamera::ZoomIn() {
    _zoom -= zoomStep;
}

void ArcballCamera::ZoomOut() {
    _zoom += zoomStep;
}

void ArcballCamera::SetScreenSize(int width, int height) {
    _halfWidth = width / 2.0f;
    _halfHeight = height / 2.0f;
    _radius = M::Min(_halfWidth, _halfHeight);
}

void ArcballCamera::StartDragging(int mouseX, int mouseY) {
    if(!_dragging) {
        _v0 = VectorOnSphereFromMousePos(_halfWidth, _halfHeight, _radius, mouseX, mouseY);
        _dragging = true;
    }
}

bool ArcballCamera::Drag(int mouseX, int mouseY) {
    if(_dragging) {
        const M::Vector3 v1 = VectorOnSphereFromMousePos(_halfWidth, _halfHeight, _radius, mouseX, mouseY);
        if(M::LinearlyDependent(_v0, v1)) return _dragging;
        const M::Vector3 axis = M::Normalize(M::Cross(_v0, v1));
        float angle = acosf(M::Clamp(-1.0f, M::Dot(_v0, v1), 1.0f));
        const M::Quaternion rot = M::Quat::RotateAxis(axis, M::Rad2Deg(-angle));
        _orient =  rot * _orient;
        _v0 = v1;
    }
    return _dragging;
}

void ArcballCamera::StopDragging() {
    _dragging = false;
}

void ArcballCamera::StartPanning(int mouseX, int mouseY) {
    if(!_panning) {
        _v0 = VectorInXYPlaneFromMousePos(_halfWidth, _halfHeight, mouseX, mouseY, _zoom);
        _lastTarget = _target;
        _panning = true;
    }
}

bool ArcballCamera::Pan(int mouseX, int mouseY) {
    if(_panning) {
        const M::Vector3 v1 = VectorInXYPlaneFromMousePos(_halfWidth, _halfHeight, mouseX, mouseY, _zoom);
        _target = _lastTarget + Transform(_orient, v1 - _v0);
    }
    return _panning;
}

void ArcballCamera::StopPanning() {
    _panning = false;
}

void ArcballCamera::StartZooming(int mouseX, int mouseY) {
    if(!_zooming) {
        _y0 = mouseY;
        _lastZoom = _zoom;
        _zooming = true;
    }
}

bool ArcballCamera::Zoom(int mouseX, int mouseY) {
    const float scale = 0.8f;
    if(_zooming) {
        float zoom = scale  * (_y0 - mouseY);
        _zoom = _lastZoom - zoom;
    }
    return _zooming;
}

void ArcballCamera::StopZooming() {
    _zooming = false;
}

M::Matrix4 ArcballCamera::GetWorldToEyeMatrix() const {
    const M::Quaternion q = M::Quaternion(_orient.w, -_orient.v);
    const M::Vector3    p = Position();
    M::Matrix4 T = M::Mat4::Translate(-p);
    M::Matrix4 R = M::Mat4::FromRigidTransform(q, M::Vector3::Zero);
    return R * T;
}
