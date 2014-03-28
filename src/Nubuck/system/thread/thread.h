#pragma once

#include <Windows.h>
#include <Nubuck\generic\uncopyable.h>

namespace SYS {

	class Thread : public GEN::Uncopyable {
	private:
		HANDLE _handle;
	public:
                Thread();
		virtual ~Thread();

		virtual DWORD Thread_Func() = 0;

		void Thread_StartAsync();
		void Thread_Kill();
		void Thread_Join() const;
	};

} // namespace SYS