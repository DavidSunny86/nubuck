#pragma once

#include "math.h"
#include "line3.h"

namespace M {

	M_INLINE Line3::Line3(const Vector3& p0, const Vector3& p1) : p0(p0), p1(p1) { }

} // namespace M