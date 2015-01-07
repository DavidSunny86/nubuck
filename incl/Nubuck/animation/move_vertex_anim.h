#pragma once

#include <LEDA\graph\graph.h>
#include <LEDA\geo\d3_rat_point.h>

#include <Nubuck\nubuck_api.h>
#include <Nubuck\nubuck.h>
#include <Nubuck\animation\animation.h>

namespace A {

class NUBUCK_API MoveVertexAnimation : public Animation {
private:
    typedef leda::d3_rat_point point3_t;

    W::ENT_Geometry* _subject;

    leda::node  _vert;
    point3_t    _p0, _p1;

    float       _time;
    float       _duration;
protected:
    bool Animate() override;
public:
    MoveVertexAnimation();

    void Init(const NB::Mesh subject, leda::node vertex, const point3_t& position, float duration);
};

// supplementary convenience function

NUBUCK_API void SetVertexPosition(const NB::Mesh subject, const leda::node vertex, const leda::d3_rat_point& position, const float duration);

} // namespace A
