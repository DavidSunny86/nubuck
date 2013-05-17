#include <common\common.h>
#include "..\winerror.h"
#include "semaphore.h"

namespace SYS {

    Semaphore::Semaphore(int initCount) : _hsem(NULL) {
        if(!(_hsem = CreateSemaphore(NULL, initCount, MAX_COUNT, NULL))) {
            common.printf("Semaphore: CreateSemaphore failed.\n");
            CHECK_WIN_ERROR;
            Crash();
        }
    }

    Semaphore::~Semaphore(void) {
        /* handle is closed automatically when
         * process terminates */
    }

    void Semaphore::Wait(void) {
        WaitForSingleObject(_hsem, INFINITE);
    }

    void Semaphore::Signal(void) {
        ReleaseSemaphore(_hsem, 1, NULL);
    }

} // namespace SYS
