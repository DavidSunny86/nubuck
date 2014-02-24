#include <Nubuck\common\common.h>
#include <Nubuck\system\locks\spinlock.h>
#include <system\winerror.h>

namespace SYS {

    SpinLock::SpinLock(void) {
        if(!InitializeCriticalSectionAndSpinCount(&_cs, SPIN_COUNT))
            common.printf("ERROR - SpinLock: initializing critical section failed.\n");
        CHECK_WIN_ERROR;
    }

    SpinLock::~SpinLock(void) {
        DeleteCriticalSection(&_cs);
    }

    void SpinLock::Lock(void) {
        EnterCriticalSection(&_cs);
    }

    void SpinLock::Unlock(void) {
        LeaveCriticalSection(&_cs);
    }

} // namespace SYS