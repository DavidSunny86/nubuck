#include <stdlib.h>
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

    Color Color::White	= Color(1.0f, 1.0f, 1.0f);
	Color Color::Black	= Color(0.0f, 0.0f, 0.0f);
	Color Color::Red	= Color(1.0f, 0.0f, 0.0f);
	Color Color::Green	= Color(0.0f, 1.0f, 0.0f);
	Color Color::Blue	= Color(0.0f, 0.0f, 1.0f);

} // namespace R
