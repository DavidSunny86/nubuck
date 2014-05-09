#include <Nubuck\common\common.h>
#include <Nubuck\system\locks\semaphore.h>
#include <system\winerror.h>

namespace SYS {

    Semaphore::Semaphore(long initCount, long maxCount) : _hsem(NULL) {
        if(!(_hsem = CreateSemaphore(NULL, initCount, maxCount, NULL))) {
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
