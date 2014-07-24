#include <Nubuck\animation\animator.h>

namespace A {

// hides allocation details
// URGENT: this should probably be locked
template<typename TYPE>
TYPE* Animator::NewAnimation() {
    TYPE* anim = new TYPE();

    {
        SYS::ScopedLock lock(_animsMtx);
        anim->animatorLink.prev = NULL;
        anim->animatorLink.next = _anims;
        if(_anims) _anims->animatorLink.prev = anim;
        _anims = anim;
    }

    _isIdle = false;

    return anim;
}

} // namespace A