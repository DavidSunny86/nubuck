#pragma once

#include <LEDA\graph\graph.h>
#include <LEDA\geo\d3_rat_point.h>

namespace M {

struct Vector3;

} // namespace M

class IGeometry;

namespace W {

class IAnimation {
public:
    virtual ~IAnimation() { }

    virtual void Move(float secsPassed) = 0;
};

class Animation : public IAnimation {
public:
    struct AnimatorLink {
        Animation *prev, *next;
    } animatorLink;

    struct SubjectLink {
        Animation* next;
    } subjectLink;

    virtual ~Animation() { }
};

class MoveVertexAnimation : public Animation {
private:
    typedef leda::d3_rat_point point3_t;

    IGeometry* _subject;

    leda::node  _vert;
    point3_t    _p0, _p1;

    float       _time;
    float       _duration;
public:
    MoveVertexAnimation();

    void Init(IGeometry* subject, leda::node vertex, const point3_t& position, float duration);

    void Move(float secsPassed) override;
};

class Animator {
private:
    Animation* _anims;
public:
    Animator();

    void Move(float secsPassed) {
        Animation* anim = _anims;
        while(anim) {
            IAnimation* ianim = anim;
            // anim->Move(secsPassed);
            ianim->Move(secsPassed);
            anim = anim->animatorLink.next;
        }
    }

    template<typename TYPE>
    TYPE* NewAnimation();

    void DeleteAnimation(Animation* anim) {
        if(anim->animatorLink.prev) anim->animatorLink.prev->animatorLink.next = anim->animatorLink.next;
        if(anim->animatorLink.next) anim->animatorLink.next->animatorLink.prev = anim->animatorLink.prev;
        if(_anims == anim) _anims = anim->animatorLink.next;
        delete anim;
    }
};

template<typename TYPE> // TYPE must be a subclass of Animation
TYPE* Animator::NewAnimation() {
    TYPE* anim = new TYPE();

    anim->animatorLink.prev = NULL;
    anim->animatorLink.next = _anims;
    if(_anims) _anims->animatorLink.prev = anim;
    _anims = anim;

    return anim;
}

extern Animator g_animator;

// util

void SetVertexPosition(IGeometry* subject, const leda::node vertex, const leda::d3_rat_point& position, const float duration);

} // namespace W