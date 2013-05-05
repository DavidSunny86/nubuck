#pragma once

namespace R {

    struct Color {
        float r, g, b, a;

        Color(void) { }
        Color(float r, float g, float b, float a = 1.0f);

		static Color RandomSolidColor(void);

        static Color White;
		static Color Black;
		static Color Red;
		static Color Green;
		static Color Blue;
    };

	Color operator+(const Color& lhp, const Color& rhp);
	Color operator*(float f, const Color& color);

} // namespace R

#include "color_inl.h"
