#pragma once

#include "color.h"

namespace R {

	inline Color operator+(const Color& lhp, const Color& rhp) {
		return Color(lhp.r + rhp.r, lhp.g + rhp.g, lhp.b + rhp.b);
	}

	inline Color operator*(float f, const Color& color) {
		return Color(f * color.r, f * color.g, f * color.b, f * color.a);
	}

    inline void CopyRGB(Color& dst, const Color& src) {
        dst.r = src.r;
        dst.g = src.g;
        dst.b = src.b;
    }

} // namespace R