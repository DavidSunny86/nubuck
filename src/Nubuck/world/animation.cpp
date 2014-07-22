#include <Nubuck\polymesh.h>
#include <Nubuck\nubuck.h>
#include <Nubuck\math_conv.h>
#include <Nubuck\world\animation.h>
#include <world\entities\ent_geometry\ent_geometry.h>

namespace W {

MoveVertexAnimation::MoveVertexAnimation()
    : _subject(NULL)
    , _time(0.0f)
{ }

void MoveVertexAnimation::Init(IGeometry* subject, leda::node vert, const point3_t& position, float duration) {
    _subject = subject;
    _vert = vert;
    _duration = duration;

    const leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    _p0 = mesh.position_of(_vert);
    _p1 = position;

    W::ENT_Geometry* geom = (W::ENT_Geometry*)_subject;
    geom->AttachAnimation(this);
}

void MoveVertexAnimation::Move(float secsPassed) {
    float l = _time / _duration;

    leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    M::Vector3 p = (1.0f - l) * ToVector(_p0) + l * ToVector(_p1);
    mesh.set_position(_vert, ToRatPoint(p));

    _time += secsPassed;

    W::ENT_Geometry* geom = (W::ENT_Geometry*)_subject;
    geom->Rebuild();
}

Animator::Animator() : _anims(NULL) { }

Animator g_animator;

void SetVertexPosition(IGeometry* subject, const leda::node vertex, const leda::d3_rat_point& position, const float duration) {
    MoveVertexAnimation* anim = g_animator.NewAnimation<MoveVertexAnimation>();
    anim->Init(subject, vertex, position, duration);
}

} // namespace W