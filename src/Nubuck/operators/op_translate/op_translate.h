#pragma once

#include <vector>

#include <QMenu>

#include <Nubuck\nubuck.h>
#include <Nubuck\math\box.h>
#include <Nubuck\math\plane.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>

#include <LEDA\geo\d3_hull.h>

namespace OP {

class Translate : public Operator {
private:
    typedef leda::rational scalar_t;
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;

    R::meshPtr_t    _axisMesh;
    R::tfmeshPtr_t  _axisTFMesh;

    enum { X = 0, Y, Z, DIM };
    R::meshPtr_t    _arrowHeadMeshes[3];
    R::tfmeshPtr_t  _arrowHeadTFMeshes[3];
    M::Matrix4      _arrowHeadTF[3];

    M::Box _bboxes[DIM];

    void BuildBBoxes();
    void BuildCursor();
    void ShowCursor();
    void HideCursor();

    void SetCenterPosition(M::Box& box, const M::Vector3& center) {
        const M::Vector3 oldCenter = 0.5f * (box.max - box.min) + box.min;
        const M::Vector3 d = (center - oldCenter);
        box.min += d;
        box.max += d;
    }

    void SetPosition(const M::Vector3& pos) {
        M::Matrix4 T = M::Mat4::Translate(pos);
        R::meshMgr.GetMesh(_axisTFMesh).SetTransform(T);
        for(int i = 0; i < DIM; ++i) {
            R::meshMgr.GetMesh(_arrowHeadTFMeshes[i]).SetTransform(T * _arrowHeadTF[i]);
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

    M::Vector3  _oldCursorPos;
    M::Vector3  _cursorPos;
    bool        _hidden;

    std::vector<M::Vector3> _oldPos;

    bool        _dragging;
    int         _dragAxis;
    M::Vector3  _dragOrig;
    M::Plane    _dragPlane;

    void AlignWithCamera();
public:
    Translate();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs) override;
    void OnGeometrySelected() override;
    void OnCameraChanged() override;
    bool OnMouseDown(const M::Vector2& mouseCoords, bool shiftKey) override;
    bool OnMouseUp(const M::Vector2& mouseCoords) override;
    bool OnMouseMove(const M::Vector2& mouseCoords) override;
};

} // namespace OP