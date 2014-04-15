#pragma once

#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix4.h>
#include <Nubuck\math\quaternion.h>

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

    M::Vector3      _pos;
    M::Quaternion   _orient;
    float           _zoom;

    M::Vector3      _lastPos;
    M::Quaternion   _lastOrient;
    float           _lastZoom;

    M::Vector3 LocalZ();
public:
    ArcballCamera(int width, int height);

    void SetScreenSize(int width, int height);

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
