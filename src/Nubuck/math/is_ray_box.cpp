#include <algorithm>
#include <limits>
#include <Nubuck\math\ray.h>
#include <Nubuck\math\box.h>
#include <Nubuck\math\intersections.h>

namespace M {
namespace IS {

// "slab method", cnf. http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
bool Intersects(const Ray& ray, const Box& box, Info* info) {
    static const float inf = std::numeric_limits<float>::infinity();
    float maxNear = -inf, minFar = inf;
    for(int i = 0; i < 3; ++i) {
        // ray direction parallel to slab and ray origin not contained in slab
        if(AlmostEqual(0.0f, ray.direction.vec[i]) && (ray.origin.vec[i] < box.min.vec[i] || ray.origin.vec[i] > box.max.vec[i]))
                return false;
        // ray parameters for intersection points with slab
        float near = (box.min.vec[i] - ray.origin.vec[i]) / ray.direction.vec[i];
        float far = (box.max.vec[i] - ray.origin.vec[i]) / ray.direction.vec[i];
        if(far < near) std::swap(near, far); // invariant: near < far
        maxNear = Max(maxNear, near);
        minFar = Min(minFar, far);
    }
    return minFar >= maxNear;
}

} // namespace IS
} // namespace M