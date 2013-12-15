#pragma once

#include <Windows.h>
#include <generic\uncopyable.h>

namespace SYS {

class SpinLock;
    
class ConditionVariable : private GEN::Uncopyable {
private:
    CONDITION_VARIABLE _cv;
public:
    ConditionVariable();
    ~ConditionVariable() { }

    void Wait(SpinLock& lock);
    void Signal();
};

} // namespace SYS