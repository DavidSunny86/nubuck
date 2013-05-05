#pragma once

namespace GEN {

	template<typename TYPE>
	class Singleton {
	public:
		static TYPE& Instance(void) {
			static TYPE instance;
			return instance;
		}
	};

} // namespace GEN