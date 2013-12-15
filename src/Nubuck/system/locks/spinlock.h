#pragma once

#include <Windows.h>
#include <generic\uncopyable.h>

namespace SYS {

    class SpinLock : private GEN::Uncopyable {
    private:
        enum { SPIN_COUNT = 1000 };
        CRITICAL_SECTION _cs;
    public:
        SpinLock(void);
        ~SpinLock(void);

        void Lock(void);
        void Unlock(void);

        LPCRITICAL_SECTION GetNativeHandle(void) { return &_cs; }
    };

} // namespace SYS