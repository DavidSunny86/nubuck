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
    bool _zooming;
    M::Vector3 _v0;
    float _y0;

    M::TransformTRS	_transform;
    M::Quaternion	_dragRot;
    M::Vector3		_panTrans;
    M::Vector3      _zoomTrans;
public:
    ArcballCamera(int width, int height);

    void Reset(void);
    void ResetRotation(void);

    void SetScreenSize(int width, int height);

    void ZoomIn(void);
    void ZoomOut(void);

    void StartDragging(int mouseX, int mouseY);
    bool Drag(int mouseX, int mouseY);
    void StopDragging(void);

    void StartPanning(int mouseX, int mouseY);
    bool Pan(int mouseX, int mouseY);
    void StopPanning(void);

    void StartZooming(int mouseX, int mouseY);
    bool Zoom(int mouseX, int mouseY);
    void StopZooming(void);

    bool IsDragging(void) const { return _dragging; }

    const M::TransformTRS& GetTransform(void) const;
    M::Quaternion GetRotation(void) const;

    M::Matrix4 GetWorldMatrix(void) const;
};
