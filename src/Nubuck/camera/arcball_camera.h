#pragma once

#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix4.h>
#include <Nubuck\math\quaternion.h>

class ArcballCamera {
public:
    struct Projection {
        enum Enum {
            PERSPECTIVE = 0,
            ORTHOGRAPHIC,
            NUM_PROJECTIONS
        };
    };
private:
    float _halfWidth;
    float _halfHeight;
    float _radius;

    bool _dragging;
    bool _panning;
    bool _zooming;
    M::Vector3 _v0;
    float _y0;

    M::Vector3          _target;
    M::Quaternion       _orient;
    float               _zoom; // distance to target
    float               _projWeight;
    Projection::Enum    _proj;

    M::Vector3      _lastTarget;
    M::Quaternion   _lastOrient;
    float           _lastZoom;

    // animation state of orientation
    struct OrientAnim {
        M::Quaternion   v0, v1;
        float           dur, t;
        bool            active;

        OrientAnim() : active(false) { }
    } _orientAnim;

    // animation state of translation
    struct TransAnim {
        M::Vector3  v0, v1;
        float       dur, t;
        bool        active;

        TransAnim() : active(false) { }
    } _transAnim;

    // animation state of projection weight
    struct ProjWeightAnim {
        float   v0, v1;
        float   dur, t;
        bool    active;

        ProjWeightAnim() : active(false) { }
    } _projWeightAnim;

    M::Vector3 Position() const;
public:
    ArcballCamera(int width, int height);

    float GetZoom() const;
    float GetProjectionWeight() const;

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

    void RotateTo(const M::Quaternion& orient, float dur);
    void TranslateTo(const M::Vector3& pos, float dur);
    void SetProjection(Projection::Enum proj, float dur);
    void ToggleProjection(float dur);

    bool FrameUpdate(float secsPassed); // returns true when camera changed

    M::Matrix4 GetWorldToEyeMatrix() const;
};
