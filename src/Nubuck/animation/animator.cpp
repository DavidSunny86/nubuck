#include <iostream>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\animation\animation.h>
#include <Nubuck\animation\animator.h>

namespace A {

Animator::Animator()
    : _anims(0)
    , _isIdle(true)
{ }

void Animator::Move(float secsPassed) {
    SYS::ScopedLock lock(_animsMtx);

    Animation *next, *anim = _anims;
    _isIdle = true;
    while(anim) {
        next = anim->animatorLink.next;
        if(!anim->IsDone()) {
            anim->Move(secsPassed);
            _isIdle = false;
        } else {
            UnlinkAnimation(anim);
        }
        anim = next;
    }
}

void Animator::EndFrame() {
    if(_isIdle) _cvar.Signal();
}

void Animator::Filter(const EV::Event& event) {
    SYS::ScopedLock lock(_animsMtx);

    Animation* anim = _anims;
    while(anim) {
        anim->FilterEvent(event);
        anim = anim->animatorLink.next;
    }
}

void Animator::LinkAnimation(Animation* anim) {
    {
        SYS::ScopedLock lock(_animsMtx);
        anim->animatorLink.prev = NULL;
        anim->animatorLink.next = _anims;
        if(_anims) _anims->animatorLink.prev = anim;
        _anims = anim;
    }

    _isIdle = false;
}

void Animator::UnlinkAnimation(Animation* anim) {
    SYS::ScopedLock lock(_animsMtx);

    Animation::AnimatorLink& link = anim->animatorLink;
    if(link.prev) link.prev->animatorLink.next = link.next;
    if(link.next) link.next->animatorLink.prev = link.prev;
    if(_anims == anim) _anims = link.next;

    link.prev = link.next = NULL;
}

Animator g_animator;

} // namespace A