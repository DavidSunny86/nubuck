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

    Animation* anim = _anims;
    _isIdle = true;
    while(anim) {
        if(!anim->IsDone()) {
            anim->Move(secsPassed);
            _isIdle = false;
        }
        anim = anim->animatorLink.next;
    }

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

void Animator::DeleteAnimation(Animation* anim) {
    Animation::AnimatorLink& link = anim->animatorLink;

    // unlink
    if(link.prev) link.prev->animatorLink.next = link.next;
    if(link.next) link.next->animatorLink.prev = link.prev;
    if(_anims == anim) _anims = link.next;

    delete anim;
}

Animator g_animator;

} // namespace A