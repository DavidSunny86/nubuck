#include <assert.h>
#include <algorithm>
#include "bezier.h"

namespace R {

const int GQ_N = 10;
float weights[] = {
    0.2955242247147529,
    0.2955242247147529,
    0.2692667193099963,
    0.2692667193099963,
    0.2190863625159820,
    0.2190863625159820,
    0.1494513491505806,
    0.1494513491505806,
    0.0666713443086881,
    0.0666713443086881
};
float abscissa[] = {
    -0.1488743389816312,
    0.1488743389816312,
    -0.4333953941292472,
    0.4333953941292472,
    -0.6794095682990244,
    0.6794095682990244,
    -0.8650633666889845,
    0.8650633666889845,
    -0.9739065285171717,
    0.9739065285171717
};

// direct evaluation of linear bezier curve
static M::Vector2 Bezier1(const M::Vector2& p0, const M::Vector2& p1, float t) {
    return (1.0f - t) * p0 + t * p1;
}

// direct evaluation of quadric bezier curve
static M::Vector2 Bezier2(const M::Vector2& p0, const M::Vector2& p1, const M::Vector2& p2, float t) {
    const float c = (1.0f - t);
    return c * c * p0 + 2.0f * c * t * p1 + t * t * p2;
}

static M::Vector2 DeCasteljau(const std::vector<M::Vector2>& points, int l, int r, float t) {
    assert(l <= r);
    if(l == r) return points[l];
    return (1.0f - t) * DeCasteljau(points, l, r - 1, t) + t * DeCasteljau(points, l + 1, r, t);
}

static M::Vector2 Eval(const PolyBezier2U& pb, int l, float t) {
    // return DeCasteljau(pb.points, l, l + 2,  t);
    return Bezier2(pb.points[l], pb.points[l + 1], pb.points[l + 2], t);
}

// approximate length of bezier curve segment with gauss quadrature
static float Length(const PolyBezier2U& pb, int l, float t0, float t1) {
    float s = 0.0f;
    // weights of derivative
    const M::Vector2 p0 = 2.0f * (pb.points[l + 1] - pb.points[l]);
    const M::Vector2 p1 = 2.0f * (pb.points[l + 2] - pb.points[l + 1]);
    for(int i = 0; i < GQ_N; ++i)
        s += weights[i] * M::Length(Bezier1(p0, p1, 0.5f * (t1 - t0) * abscissa[i] + 0.5f * (t0 + t1)));
    return 0.5f * (t1 - t0) * s;
}

void ComputeTSamples(PolyBezier2U& pb) {
    assert(0 < pb.points.size() && pb.points.size() / 2 * 2);
    const float dt = 0.01f;
    float dist = 0.0f;
    TSample ts;
    for(int i = 0; i <= pb.points.size() - 2; i += 2) {
        float t0 = 0.0f;
        for(float t = 0.0f; t <= 1.0f; t += dt) {
            // dist += M::Length(B(i, t) - B(i, t0)); 
            dist += Length(pb, i, t0, t);
            t0 = t;
            ts.d = dist;
            ts.t = t;
            ts.l = i;
            pb.tSamples.push_back(ts);
        }
    }
    pb.length = dist;
}

static bool TSample_Cmp(const TSample& lhp, const TSample& rhp) {
    return lhp.d < rhp.d;
}

static M::Vector2 FromDist(const PolyBezier2U& pb, float dist) {
    TSample ts;
    ts.d = dist;
    ts.t = 0.0f;
    ts.l = 0;
    std::vector<TSample>::const_iterator s1 = std::lower_bound(pb.tSamples.begin(), pb.tSamples.end(), ts, TSample_Cmp);
    if(s1 == pb.tSamples.begin()) return Eval(pb, s1->l, s1->t);
    std::vector<TSample>::const_iterator s0 = s1 - 1;
    float t = (dist - s0->d) / (s1->d - s0->d);
    assert(0.0f <= t && t <= 1.0f);
    return (1.0f - t) * Eval(pb, s0->l, s0->t) + t * Eval(pb, s1->l, s1->t);
}

void SampleEquidistantPoints_old(const PolyBezier2U& pb, float off, float dd, std::vector<M::Vector2>& out) {
    out.clear();
    for(float d = 0.0f; d < pb.length; d += dd) {
        float dist = d + off;
        while(dist > pb.length) dist -= pb.length;
        out.push_back(FromDist(pb, dist));
    }
}

void SampleEquidistantPoints(const PolyBezier2U& pb, float off, float dd, std::vector<M::Vector2>& out) {
    unsigned lb = 0;
    const TSample *s0 = NULL, *s1 = NULL;
    for(float d = 0.0f; d < pb.length; d += dd) {
        float dist = fmod(off + d, pb.length);
        if(pb.tSamples[lb].d > dist) lb = 0; // dist wrapped around
        while(pb.tSamples[lb + 1].d <= dist) lb++; // note: off < pb.length = pb.tSamples.end().d
        s0 = &pb.tSamples[lb];
        s1 = &pb.tSamples[lb + 1];
        assert(s0->d <= dist && dist <= s1->d);
        assert(0 <= lb && lb < pb.tSamples.size() - 1);
        float t = (dist - s0->d) / (s1->d - s0->d);
        assert(0.0f <= t && t <= 1.0f);
        out.push_back((1.0f - t) * Eval(pb, s0->l, s0->t) + t * Eval(pb, s1->l, s1->t));
    }
}

} // namespace R