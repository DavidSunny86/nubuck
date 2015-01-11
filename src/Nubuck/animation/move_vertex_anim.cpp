#include <maxint.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\nubuck.h>
#include <Nubuck\math_conv.h>
#include <Nubuck\animation\animator.h>
#include <Nubuck\animation\move_vertex_anim.h>
#include <Nubuck\operators\operator.h>
#include <world\entities\ent_geometry\ent_geometry.h>

namespace A {

MoveVertexAnimation::MoveVertexAnimation()
    : _subject(NULL)
    , _time(0.0f)
{ }

void MoveVertexAnimation::Init(const NB::Mesh subject, leda::node vert, const point3_t& position, float duration) {
    _subject = subject;
    _vert = vert;
    _duration = duration;

    const leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    _p0 = mesh.position_of(_vert);
    _p1 = position;

    W::ENT_Geometry* geom = (W::ENT_Geometry*)_subject;
    geom->AttachAnimation(this);
}

bool MoveVertexAnimation::Animate() {
    float l = _time / _duration;

    leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    M::Vector3 p = (1.0f - l) * ToVector(_p0) + l * ToVector(_p1);
    mesh.set_position(_vert, ToRatPoint(p));

    _time += GetSecsPassed();

    W::ENT_Geometry* geom = (W::ENT_Geometry*)_subject;
    geom->Rebuild();

    return _duration <= _time;
}

MoveVerticesAnimation::MoveVerticesAnimation()
    : _subject(NULL)
    , _time(0.0f)
{ }

void MoveVerticesAnimation::Init(NB::Mesh subject, float duration) {
    _subject = subject;
    _duration = duration;

    leda::NbGraph& graph = NB::GetGraph(subject);

    _p0.init(graph);
    _p1.init(graph);

    leda::node v;
    forall_nodes(v, graph) {
        _p0[v] = _p1[v] = graph.position_of(v);
    }
}

void MoveVerticesAnimation::SetStopPosition(leda::node v, const point3_t& p) {
    _p1[v] = p;
}

bool MoveVerticesAnimation::Animate() {
    float l = _time / _duration;

    leda::NbGraph& graph = _subject->GetRatPolyMesh();

    leda::node v;
    forall_nodes(v, graph) {
        const M::Vector3 p = (1.0f - l) * ToVector(_p0[v]) + l * ToVector(_p1[v]);
        graph.set_position(v, ToRatPoint(p));
    }

    _time += GetSecsPassed();

    return _duration <= _time;
}

NUBUCK_API void SetVertexPosition(const NB::Mesh subject, const leda::node vertex, const leda::d3_rat_point& position, const float duration) {
    MoveVertexAnimation anim;
    anim.Init(subject, vertex, position, duration);
    anim.PlayFor(duration);
    OP::WaitForAnimations();
}

} // namespace A