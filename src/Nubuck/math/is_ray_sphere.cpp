#include <Nubuck\math\ray.h>
#include <Nubuck\math\sphere.h>
#include <Nubuck\math\intersections.h>

namespace M {
namespace IS {

	bool Intersects(const Ray& ray, const Sphere& sphere, Info* info) {
		const float B = 2.0f *
			(ray.direction.x * (ray.origin.x - sphere.center.x) +
			 ray.direction.y * (ray.origin.y - sphere.center.y) +
			 ray.direction.z * (ray.origin.z - sphere.center.z));
		const float C = (ray.origin.x - sphere.center.x) * (ray.origin.x - sphere.center.x)
			+ (ray.origin.y - sphere.center.y) * (ray.origin.y - sphere.center.y)
			+ (ray.origin.z - sphere.center.z) * (ray.origin.z - sphere.center.z)
			- sphere.radius * sphere.radius;

		const float M = B * B - 4.0f * C;
		if(0.0f > M) return false;
		const float D = sqrt(M);

		const float t0 = 0.5f * (-B - D);
		if(t0 > 0) {
			if(info) {
				info->where = ray.origin + t0 * ray.direction;
				info->normal = M::Normalize(info->where - sphere.center);
				info->distance = t0;
			}
			return true;
		}

		const float t1 = 0.5f * (-B + D);
		if(t1 > 0) {
			if(info) {
				info->where = ray.origin + t1 * ray.direction;
				info->normal = M::Normalize(info->where - sphere.center);
				info->distance = t1;
			}
			return true;
		}

		return false;
	}

} // namespace IS
} // namespace M