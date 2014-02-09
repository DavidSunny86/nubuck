#include <Nubuck\math\ray.h>
#include <Nubuck\math\plane.h>
#include <Nubuck\math\intersections.h>

namespace M {
namespace IS {

bool Intersects(const Ray& ray, const Plane& plane, Info* info) {
    float nv = Dot(plane.n, ray.direction);
    if(AlmostEqual(0.0f, nv)) return 0.0f;
    if(info) {
        float t = -(Dot(plane.n, ray.origin) + plane.d) / nv;
        info->normal = plane.n;
        info->distance = t;
        info->where = ray.origin + t * ray.direction;
    }
    return true;
}

} // namespace IS
} // namespace M