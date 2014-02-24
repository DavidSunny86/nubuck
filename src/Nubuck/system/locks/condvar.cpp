#include <Nubuck\common\common.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\condvar.h>
#include <system\winerror.h>

namespace SYS {

ConditionVariable::ConditionVariable() {
    InitializeConditionVariable(&_cv);
    CHECK_WIN_ERROR; // despite not any errors documented for CVs
}

void ConditionVariable::Wait(SpinLock& lock) {
    if(!SleepConditionVariableCS(&_cv, lock.GetNativeHandle(), INFINITE)) {
        common.printf("ERROR - ConditionVariable: sleep on critical section failed.\n");
        CHECK_WIN_ERROR;
    }
}

void ConditionVariable::Signal() {
    WakeConditionVariable(&_cv);
}

} // namespace SYS