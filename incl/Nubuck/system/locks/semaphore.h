#pragma once

#include <Windows.h>
#include <Nubuck\generic\uncopyable.h>

namespace SYS {

    class Semaphore : private GEN::Uncopyable {
    private:
        HANDLE _hsem;
    public:
        enum { MAX_COUNT = 10000 };

        explicit Semaphore(long initCount, long maxCount = MAX_COUNT);
        ~Semaphore(void);

        void Wait(void);
        void Signal(void);
    };

} // namespace SYS
