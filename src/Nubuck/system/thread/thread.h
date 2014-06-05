#pragma once

#include <Windows.h>
#include <Nubuck\generic\uncopyable.h>

namespace SYS {

	class Thread : public GEN::Uncopyable {
	private:
		HANDLE _handle;
	public:
        enum { INVALID_THREAD_ID = 0 };

        typedef DWORD threadID_t;

        static threadID_t CallerID();

                Thread();
		virtual ~Thread();

		virtual DWORD Thread_Func() = 0;

		void Thread_StartAsync();
		void Thread_Kill();
		void Thread_Join() const;
	};

} // namespace SYS