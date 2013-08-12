#pragma once

#include <Windows.h>
#include <generic\uncopyable.h>

namespace SYS {

	class Thread : public GEN::Uncopyable {
	private:
		HANDLE _handle;
	public:
		virtual ~Thread(void);

		virtual DWORD Thread_Func(void) = 0;

		void Thread_StartAsync(void);
		void Thread_Kill(void);
		void Thread_Join(void) const;
	};

} // namespace SYS