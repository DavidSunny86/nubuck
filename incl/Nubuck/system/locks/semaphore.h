#pragma once

#include <Windows.h>
#include <Nubuck\generic\uncopyable.h>

namespace SYS {

    class Semaphore : private GEN::Uncopyable {
    private:
        HANDLE _hsem;
    public:
        enum { MAX_COUNT = 10 };

        explicit Semaphore(int initCount);
        ~Semaphore(void);

        void Wait(void);
        void Signal(void);
    };

} // namespace SYS
