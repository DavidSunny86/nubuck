#pragma once

#include <Windows.h>
#include <generic\uncopyable.h>

namespace SYS {

	class Thread : public GEN::Uncopyable {
	private:
		HANDLE _handle;
	public:
		virtual ~Thread(void);

		virtual DWORD Run(void) = 0;

		void Start(void);
		void Kill(void);
		void Join(void) const;
	};

} // namespace SYS