#include "thread.h"

DWORD WINAPI ThreadFunc(LPVOID param) {
	SYS::Thread* thread = (SYS::Thread*)param;
	return thread->Thread_Func();
}

namespace SYS {

	Thread::~Thread(void) {
		Thread_Kill();
	}

	void Thread::Thread_StartAsync(void) {
		_handle = CreateThread(NULL, 0, ThreadFunc, this, 0, NULL);
	}

	void Thread::Thread_Kill(void) {
        if(_handle) {
            TerminateThread(_handle, 0);
            CloseHandle(_handle);
            _handle = NULL;
        }
	}

	void Thread::Thread_Join(void) const {
		WaitForSingleObject(_handle, INFINITE);
	}

} // namespace SYS