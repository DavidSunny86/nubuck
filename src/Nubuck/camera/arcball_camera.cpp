#include "arcball_camera.h"

namespace {

    const float zoomStep = 2.0f;

} // unnamed namespace

static M::Vector3 VectorOnSphereFromMousePos(float halfWidth, float halfHeight, float radius, int mouseX, int mouseY) {
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

static M::Vector3 VectorInPlaneFromMousePos(float halfWidth, float halfHeight, int mouseX, int mouseY) {
    const float f = 0.5f;
    float x = (mouseX - halfWidth) / (f * halfWidth);
    float y = (-mouseY + halfHeight) / (f * halfHeight);
    return M::Vector3(x, y, 0.0f);
}

/*
shoot a ray originating in camera through projection plane onto plane that 
is parallel to the proj. plane and passes through the origin of the world 
coordinate system. intersection point is used as translation vector.
*/
static M::Vector3 VectorInXYPlaneFromMousePos(
    float halfWidth, float halfHeight, 
    const M::Vector3& translation,
    int mouseX, int mouseY) 
{
    // hardcoded values
    const float fovy = M::Deg2Rad(45.0f); // hardcoded in renderer.cpp
    const float dist = -10.0f; // default camera distance, hardcoded in view.cpp

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
    const float l = -1.0f * (translation.z + dist);

    // intersection point
    return M::Vector3(x * l, y * l, 0.0f);
}

ArcballCamera::ArcballCamera(int width, int height)
    : _dragging(false),
      _panning(false),
      _zooming(false),
      _dragRot(M::Quat::Identity()),
      _panTrans(M::Vector3::Zero),
      _zoomTrans(M::Vector3::Zero)
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
    _zoomTrans  = M::Vector3::Zero;
}

void ArcballCamera::ResetRotation(void) {
    _panning = false;
    _dragging = false;

    _transform.rot		= M::Quat::Identity();

    _dragRot	= M::Quat::Identity();
    _panTrans	= M::Vector3::Zero;
    _zoomTrans  = M::Vector3::Zero;
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
        _v0 = VectorInXYPlaneFromMousePos(_halfWidth, _halfHeight, _transform.trans, mouseX, mouseY);
        _panning = true;
    }
}

bool ArcballCamera::Pan(int mouseX, int mouseY) {
    if(_panning) {
        const M::Vector3 v1 = VectorInXYPlaneFromMousePos(_halfWidth, _halfHeight, _transform.trans, mouseX, mouseY);
        _panTrans = v1 - _v0;
    }
    return _panning;
}

void ArcballCamera::StopPanning(void) {
    _panning = false;
    _transform.trans = _panTrans + _transform.trans;
    _panTrans = M::Vector3::Zero;
}

void ArcballCamera::StartZooming(int mouseX, int mouseY) {
    if(!_zooming) {
        _y0 = mouseY;
        _zooming = true;
    }
}

bool ArcballCamera::Zoom(int mouseX, int mouseY) {
    const float scale = 0.8f;
    if(_zooming) {
        float zoom = scale  * (_y0 - mouseY);
        _zoomTrans = M::Vector3(0.0f, 0.0f, zoom);
    }
    return _zooming;
}

void ArcballCamera::StopZooming(void) {
    _zooming = false;
    _transform.trans = _zoomTrans + _transform.trans;
    _zoomTrans = M::Vector3::Zero;
}

M::Matrix4 ArcballCamera::GetWorldToEyeMatrix(void) const {
    if(_dragging) return M::Mat4::FromTransform(_dragRot * _transform.rot, _transform.trans, _transform.scale);
    if(_panning) return M::Mat4::FromTransform(_transform.rot, _panTrans + _transform.trans, _transform.scale);
    if(_zooming) return M::Mat4::FromTransform(_transform.rot, _zoomTrans + _transform.trans, _transform.scale);
    return M::Mat4::FromTransform(_transform.rot, _transform.trans, _transform.scale);
}
