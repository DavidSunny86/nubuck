#pragma once

#include <Nubuck\nubuck_api.h>

namespace R {

    struct Color4ub { unsigned char r, g, b, a; };
    struct Color3ub { unsigned char r, g, b; };

    struct NUBUCK_API Color {
        float r, g, b, a;

        Color(void) { }
        Color(float r, float g, float b, float a = 1.0f);

		static Color RandomSolidColor(void);
        static Color FromBytes(unsigned char r, unsigned char g, unsigned char b);

        static Color White;
		static Color Black;
		static Color Red;
		static Color Green;
		static Color Blue;
        static Color Yellow;
    };

	Color operator+(const Color& lhp, const Color& rhp);
	Color operator*(float f, const Color& color);

    void  CopyRGB(Color& dst, const Color& src);

    Color BlendAddRGB(const Color& lhp, const Color& rhp);
    Color BlendMulRGB(const Color& lhp, const Color& rhp);
    Color BlendMulRGBA(const Color& lhp, const Color& rhp);

    NUBUCK_API Color Lerp(const Color& source, const Color& target, float l);

    Color4ub ColorTo4ub(const Color& col4f);
    Color3ub ColorTo3ub(const Color& col4f);

} // namespace R

#include "color_inl.h"
