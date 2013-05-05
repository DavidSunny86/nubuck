#pragma once

#include <math\vector3.h>
#include <math\matrix4.h>
#include <math\quaternion.h>
#include <math\transform.h>

class ArcballCamera {
private:
    float _halfWidth;
    float _halfHeight;
    float _radius;

    bool _dragging;
    bool _panning;
    M::Vector3 _v0;

    M::TransformTRS	_transform;
    M::Quaternion	_dragRot;
    M::Vector3		_panTrans;

    M::Vector3 VectorOnSphereFromMousePos(int mouseX, int mouseY);
    M::Vector3 VectorInPlaneFromMousePos(int mouseX, int mouseY);
    M::Vector3 VectorInXYPlaneFromMousePos(int mouseX, int mouseY);

    M::Matrix4 _projection;
public:
    ArcballCamera(int width, int height);

    void Reset(void);

    void SetScreenSize(int width, int height);

    void ZoomIn(void);
    void ZoomOut(void);

    void StartDragging(int mouseX, int mouseY);
    bool Drag(int mouseX, int mouseY);
    void StopDragging(void);

    void StartPanning(int mouseX, int mouseY);
    bool Pan(int mouseX, int mouseY);
    void StopPanning(void);

    bool IsDragging(void) const { return _dragging; }

    const M::TransformTRS& GetTransform(void) const;
    M::Quaternion GetRotation(void) const;

    M::Matrix4 GetProjectionMatrix(void) const;
    M::Matrix4 GetWorldMatrix(void) const;

    void Render(void);
};
