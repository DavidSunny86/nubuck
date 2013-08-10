#pragma once

#include <vector>

#include <math\vector2.h>
#include "..\mesh.h"

namespace R {

struct TSample {
    float d, t;
    int l;
};

// quadric, unconstrained poly-bezier curve.
struct PolyBezier2U {
    float                   length;
    /*
    format of input: e c e c e ... e c e, where e are endpoints and c are controlpoints.
    more specific: n = points.size() = 2k + 1 for a k > 0 and
    points[2i] is endpoint, points[2i + 1] is controlpoint, for 0 <= i < n - 2
    */
    std::vector<M::Vector2> points;
    std::vector<TSample>    tSamples;
    std::vector<M::Vector3> decalPos;
};

void ComputeTSamples(PolyBezier2U& pb);
void SampleEquidistantPoints(const PolyBezier2U& pb, float off, float dd, std::vector<M::Vector2>& out);

} // namespace R