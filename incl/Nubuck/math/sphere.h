#pragma once

#include "vector3.h"

namespace M {

	struct Sphere {
		M::Vector3 center;
		float radius;

		Sphere(const M::Vector3& center, float radius)
			: center(center), radius(radius) { }
	};

} // namespace M