#include <common\common.h>
#include <system\winerror.h>
#include <system\locks\spinlock.h>
#include "condvar.h"

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