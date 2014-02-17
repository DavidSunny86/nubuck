#pragma once

#include "vector3.h"

namespace M {

struct Box {
    Vector3 min; // lower lefthand corner
    Vector3 max; // upper righthand corner

    static Box FromCenterSize(const Vector3& center, const Vector3& size) {
        const Vector3 hsz = 0.5f * size;
        return FromMinMax(center - hsz, center + hsz);
    }

    static Box FromMinMax(const Vector3& min, const Vector3& max) {
        Box box;
        box.min = min;
        box.max = max;
        return box;
    }
};

Vector3 CenterOf(const Box& b);
Vector3 SizeOf(const Box& b);
Box     Scale(const Box& b, float s);

} // namespace M