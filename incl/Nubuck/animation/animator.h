#pragma once

#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\condvar.h>

namespace EV { struct Event; }

namespace A {

class Animation;

class Animator {
private:
    SYS::SpinLock   _animsMtx;
    Animation*      _anims;
    bool            _isIdle;

    SYS::ConditionVariable _cvar;
public:
    Animator();

    SYS::ConditionVariable& GetConditionVariable() { return _cvar; }

    bool IsIdle() const { return _isIdle; }

    void Move(float secsPassed);

    void Filter(const EV::Event& event);

    template<typename TYPE> 
    TYPE*   NewAnimation();

    void    DeleteAnimation(Animation* anim);
};

extern Animator g_animator;

} // namespace A

#include "animator_inl.h"