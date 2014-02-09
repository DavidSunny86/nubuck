#pragma once

#include "vector3.h"

#ifndef NULL
#define NULL 0
#endif

namespace M {

	struct Ray;
	struct Sphere;
    struct Box;
    struct Plane;

	namespace IS {

		struct Info {
			M::Vector3 where;
			M::Vector3 normal;
			float distance;
		};

		bool Intersects(const Ray& ray, const Sphere& sphere, Info* info = NULL);
        bool Intersects(const Ray& ray, const Box& box, Info* info = NULL);
        bool Intersects(const Ray& ray, const Plane& plane, Info* info = NULL);

	} // namespace IS

} // namespace M