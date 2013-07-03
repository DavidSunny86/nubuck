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
    M::Vector2 Bezier1(const M::Vector2& p0, const M::Vector2& p1, float t) {
        return (1.0f - t) * p0 + t * p1;
    }

    // direct evaluation of quadric bezier curve
    M::Vector2 Bezier2(const M::Vector2& p0, const M::Vector2& p1, const M::Vector2& p2, float t) {
        const float c = (1.0f - t);
        return c * c * p0 + 2.0f * c * t * p1 + t * t * p2;
    }

    M::Vector2 DeCasteljau(const std::vector<M::Vector2>& points, int l, int r, float t) {
        assert(l <= r);
        if(l == r) return points[l];
        return (1.0f - t) * DeCasteljau(points, l, r - 1, t) + t * DeCasteljau(points, l + 1, r, t);
    }

    M::Vector2 PolyBezier2U::B(int l, float t) {
        // return DeCasteljau(_points, l, l + 2,  t);
        return Bezier2(_points[l], _points[l + 1], _points[l + 2], t);
    }

    // approximate length of bezier curve segment with gauss quadrature
    float PolyBezier2U::Length(int l, float t0, float t1) {
        float s = 0.0f;
        // weights of derivative
        const M::Vector2 p0 = 2.0f * (_points[l + 1] - _points[l]);
        const M::Vector2 p1 = 2.0f * (_points[l + 2] - _points[l + 1]);
        for(int i = 0; i < GQ_N; ++i)
            s += weights[i] * M::Length(Bezier1(p0, p1, 0.5f * (t1 - t0) * abscissa[i] + 0.5f * (t0 + t1)));
        return 0.5f * (t1 - t0) * s;
    }

    void PolyBezier2U::Rebuild(void) {
        _vertices.clear();
        _indices.clear();
        BuildStroke();
    }

    void PolyBezier2U::Build(void) {
        const float dt = 0.01f;
        Mesh::Vertex vert;
        vert.color = Color::Black;
        vert.normal = M::Vector3(0.0f, 0.0f, 1.0f);
        Mesh::Index idxCnt = 0;
        for(int i = 0; i <= _points.size() - 2; i += 2) {
            for(float t = 0.0f; t <= 1.0f; t += dt) {
                M::Vector2 p = B(i, t);
                vert.position = M::Vector3(p.x, p.y, 0.0f);
                _vertices.push_back(vert);
                _indices.push_back(idxCnt++);
            }
        }
    }

    void PolyBezier2U::BuildStroke(void) {
        const float r = _length / 30.0f;
        float accu = 0.0;
        bool down = true;
        const float dd = 0.01f;
        Mesh::Vertex vert;
        vert.color = Color::Red;
        vert.normal = M::Vector3(0.0f, 0.0f, 1.0f);
        Mesh::Index idxCnt = 0;
        for(float d = 0.0f; d < _length; d += dd) {
            accu += dd;
            if(r <= accu) {
                down = !down;
                accu = 0.0f;
                if(down) _indices.push_back(Mesh::RESTART_INDEX);
            }
            if(down) {
                float dist = d + _time;
                while(dist > _length) dist -= _length;
                M::Vector2 p = FromDist(dist);
                vert.position = M::Vector3(p.x, p.y, 0.0f);
                _vertices.push_back(vert);
                _indices.push_back(idxCnt++);
            }
        }
    }

    void PolyBezier2U::ComputeTSamples(void) {
        const float dt = 0.01f;
        float dist = 0.0f;
        for(int i = 0; i <= _points.size() - 2; i += 2) {
            float t0 = 0.0f;
            for(float t = 0.0f; t <= 1.0f; t += dt) {
                // dist += M::Length(B(i, t) - B(i, t0)); 
                dist += Length(i, t0, t);
                t0 = t;
                _tSamples.push_back(TSample(dist, i, t));
            }
        }
        _length = dist;
    }
    
    M::Vector2 PolyBezier2U::FromDist(float dist) {
        std::vector<TSample>::iterator s1 = std::lower_bound(_tSamples.begin(), _tSamples.end(), TSample(dist, 0, 0.0f));
        if(s1 == _tSamples.begin()) return B(s1->l, s1->t);
        std::vector<TSample>::iterator s0 = s1 - 1;
        float t = (dist - s0->d) / (s1->d - s0->d);
        assert(0.0f <= t && t <= 1.0f);
        return (1.0f - t) * B(s0->l, s0->t) + t * B(s1->l, s1->t);
    }

    PolyBezier2U::PolyBezier2U(const std::vector<M::Vector2>& points) : _points(points), _time(0.0f) {
        assert(0 < _points.size() && _points.size() / 2 * 2);
        ComputeTSamples();
        BuildStroke();
    }

    Mesh::Desc PolyBezier2U::GetDesc(void) {
        Mesh::Desc desc;
        desc.vertices = &_vertices[0];
        desc.numVertices = _vertices.size();
        desc.indices = &_indices[0];
        desc.numIndices = _indices.size();
        desc.primType = GL_LINE_STRIP;
        return desc;
    }

    void PolyBezier2U::SampleEquidistantPoints(float dd, std::vector<M::Vector2>& out) {
        out.clear();
        for(float d = 0.0f; d < _length; d += dd) {
            float dist = d + _time;
            while(dist > _length) dist -= _length;
            out.push_back(FromDist(dist));
        }
    }

    void PolyBezier2U::Update(float secsPassed) {
        _time += secsPassed * 0.1;
        Rebuild();
    }

} // namespace R