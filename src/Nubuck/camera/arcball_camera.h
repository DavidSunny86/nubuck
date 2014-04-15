#pragma once

#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix4.h>
#include <Nubuck\math\quaternion.h>
#include <Nubuck\math\transform.h>

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

    void SetScreenSize(int width, int height);

    void SetTransform(const M::TransformTRS& tf) { _transform = tf; }

    void Reset();
    void ResetRotation();

    void StartDragging(int mouseX, int mouseY);
    bool Drag(int mouseX, int mouseY);
    void StopDragging();

    void StartPanning(int mouseX, int mouseY);
    bool Pan(int mouseX, int mouseY);
    void StopPanning();

    void StartZooming(int mouseX, int mouseY);
    bool Zoom(int mouseX, int mouseY);
    void StopZooming();

    void ZoomIn();
    void ZoomOut();

    M::Matrix4 GetWorldToEyeMatrix() const;
};
