#pragma once

#include "vector3.h"

namespace M {

	struct Line {
		Vector3 p0, p1;

		Line(void) { }
		Line(const Vector3& p0, const Vector3& p1);
	};

	float Distance(const Line& l, const Vector3& p);

} // namespace M

#include "line_mem_inl.h"