#include <stdlib.h>

#include <math\math.h>
#include "color.h"

static float RandomIn01(void) {
    const unsigned range = 1000;
    return (float)(rand() % range) / range;
}

namespace R {

    Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) { }

    Color Color::RandomSolidColor(void) {
        return Color(RandomIn01(), RandomIn01(), RandomIn01());
    }

    Color Color::FromBytes(char r, char g, char b) {
        const float f = 1.0f / 255.0f;
        return Color(f * r, f * g, f * b);
    }

    Color Color::White	= Color(1.0f, 1.0f, 1.0f);
    Color Color::Black	= Color(0.0f, 0.0f, 0.0f);
    Color Color::Red	= Color(1.0f, 0.0f, 0.0f);
    Color Color::Green	= Color(0.0f, 1.0f, 0.0f);
    Color Color::Blue	= Color(0.0f, 0.0f, 1.0f);

    Color BlendAddRGB(const Color& lhp, const Color& rhp) {
        return Color(
            M::Min(1.0f, lhp.r + rhp.r), 
            M::Min(1.0f, lhp.g + rhp.g), 
            M::Min(1.0f, lhp.b + rhp.b));
    }

    Color BlendMulRGB(const Color& lhp, const Color& rhp) {
        return Color(lhp.r * rhp.r, lhp.g * rhp.g, lhp.b * rhp.b);
    }

    Color BlendMulRGBA(const Color& lhp, const Color& rhp) {
        return Color(lhp.r * rhp.r, lhp.g * rhp.g, lhp.b * rhp.b, lhp.a * rhp.a);
    }

    Color Lerp(const Color& source, const Color& target, float l) {
        return R::Color(
            (1.0f - l) * source.r + l * target.r,
            (1.0f - l) * source.g + l * target.g,
            (1.0f - l) * source.b + l * target.b,
            (1.0f - l) * source.a + l * target.a);
    }

    Color4ub ColorTo4ub(const Color& col4f) {
        const unsigned char min = 0;
        const unsigned char max = 255;
        Color4ub col4ub;
        col4ub.r = M::Clamp(min, (unsigned char)(255 * col4f.r), max);
        col4ub.g = M::Clamp(min, (unsigned char)(255 * col4f.g), max);
        col4ub.b = M::Clamp(min, (unsigned char)(255 * col4f.b), max);
        col4ub.a = M::Clamp(min, (unsigned char)(255 * col4f.a), max);
        return col4ub;
    }

    Color3ub ColorTo3ub(const Color& col4f) {
        const unsigned char min = 0;
        const unsigned char max = 255;
        Color3ub col3ub;
        col3ub.r = M::Clamp(min, (unsigned char)(255 * col4f.r), max);
        col3ub.g = M::Clamp(min, (unsigned char)(255 * col4f.g), max);
        col3ub.b = M::Clamp(min, (unsigned char)(255 * col4f.b), max);
        return col3ub;
    }

} // namespace R
