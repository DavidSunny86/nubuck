#pragma once

#include "vector3.h"

namespace M {

	struct Line3 {
		Vector3 p0, p1;

		Line3(void) { }
		Line3(const Vector3& p0, const Vector3& p1);
	};

	float Distance(const Line3& l, const Vector3& p);

} // namespace M

#include "line_mem_inl.h"