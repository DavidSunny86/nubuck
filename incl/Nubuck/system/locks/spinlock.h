#pragma once

#include <Windows.h>
#include <Nubuck\nubuck_api.h>
#include <Nubuck\generic\uncopyable.h>

namespace SYS {

    class NUBUCK_API SpinLock : private GEN::Uncopyable {
    private:
        enum { SPIN_COUNT = 1000 };
        CRITICAL_SECTION _cs;
    public:
        SpinLock(void);
        ~SpinLock(void);

        /*
        the Lock() method is reeentrant which means a thread can call
        Lock() again without blocking, if it already owns the resource.
        There must be a matching call to Unlock() for each call to Lock().
        see http://msdn.microsoft.com/en-us/library/ms682608.aspx
        */
        void Lock(void);
        void Unlock(void);

        LPCRITICAL_SECTION GetNativeHandle(void) { return &_cs; }
    };

} // namespace SYS