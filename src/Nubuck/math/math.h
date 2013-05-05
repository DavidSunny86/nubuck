#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define M_INLINE __forceinline
#else
    #define M_INLINE inline
#endif

namespace M {

	const float PI = 3.14159265f;

	template<typename TYPE> TYPE Abs(TYPE value);
	template<typename TYPE> TYPE Min(TYPE lhp, TYPE rhp);
	template<typename TYPE> TYPE Max(TYPE lhp, TYPE rhp);
	template<typename TYPE> TYPE Clamp(TYPE min, TYPE val, TYPE max);

	float Deg2Rad(float deg);
	float Rad2Deg(float rad);

	bool AlmostEqual(float lhp, float rhp);

} // namespace M

#include "math_inl.h"
