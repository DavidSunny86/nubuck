#pragma once

#include <vector>

#include <QMenu>

#include <Nubuck\nubuck.h>
#include <math\box.h>
#include <math\plane.h>
#include <operators\operator.h>
#include <operators\operators.h>
#include <renderer\mesh\mesh.h>

#include <LEDA\geo\d3_hull.h>

namespace OP {

class Translate : public Operator {
private:
    typedef leda::rational scalar_t;
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;

    IGeometry* _geom_axis;

    enum { X = 0, Y, Z, DIM };
    IGeometry*  _geom_arrowHeads[DIM];
    scalar_t    _arrowHeadSize;
    M::Vector3  _arrowHeadsOffsets[DIM];

    M::Box _bboxes[DIM];

    void BuildAxis();
    void BuildArrowHead();
    void BuildBBoxes();

    void SetPosition(const M::Vector3& pos) {
        _geom_axis->SetPosition(pos.x, pos.y, pos.z);
        for(unsigned i = 0; i < DIM; ++i) {
            const M::Vector3& off = _arrowHeadsOffsets[i];
            _geom_arrowHeads[i]->SetPosition(off.x + pos.x, off.y + pos.y, off.z + pos.z);
        }
    }

    M::Vector3  _position;

    bool        _dragging;
    int         _dragAxis;
    M::Vector3  _dragOrig;
    M::Plane    _dragPlane;
public:
    Translate();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override { }
    void Finish() override { }

    void OnMouseDown(const M::Vector2& mouseCoords) override;
    void OnMouseMove(const M::Vector2& mouseCoords) override;
};

} // namespace OP