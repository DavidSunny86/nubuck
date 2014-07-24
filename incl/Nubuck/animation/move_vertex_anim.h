#pragma once

#include <LEDA\graph\graph.h>
#include <LEDA\geo\d3_rat_point.h>

#include <Nubuck\animation\animation.h>

struct IGeometry;

namespace A {

class MoveVertexAnimation : public Animation {
private:
    typedef leda::d3_rat_point point3_t;

    IGeometry* _subject;

    leda::node  _vert;
    point3_t    _p0, _p1;

    float       _time;
    float       _duration;
protected:
    void DoMove(float secsPassed) override;
public:
    MoveVertexAnimation();

    void Init(IGeometry* subject, leda::node vertex, const point3_t& position, float duration);
};

// supplementary convenience function

void SetVertexPosition(IGeometry* subject, const leda::node vertex, const leda::d3_rat_point& position, const float duration);

} // namespace A
