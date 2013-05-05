#include "thread.h"

DWORD WINAPI ThreadFunc(LPVOID param) {
	SYS::Thread* thread = (SYS::Thread*)param;
	return thread->Run();
}

namespace SYS {

	Thread::~Thread(void) {
		Kill();
	}

	void Thread::Start(void) {
		_handle = CreateThread(NULL, 0, ThreadFunc, this, 0, NULL);
	}

	void Thread::Kill(void) {
		TerminateThread(_handle, 0);
		CloseHandle(_handle);
		_handle = NULL;
	}

	void Thread::Join(void) const {
		WaitForSingleObject(_handle, INFINITE);
	}

} // namespace SYS