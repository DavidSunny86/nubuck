#pragma once

#include "math.h"
#include "line.h"

namespace M {

	M_INLINE Line::Line(const Vector3& p0, const Vector3& p1) : p0(p0), p1(p1) { }

} // namespace M