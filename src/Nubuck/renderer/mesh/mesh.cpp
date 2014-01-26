#include <vector>
#include <string.h>

#include <system\locks\scoped_lock.h>
#include <renderer\giantbuffer\giant_buffer.h>
#include "mesh.h"

namespace R {

static void GenerateTrianglesFromTriangles(const std::vector<Mesh::Index>& indices, std::vector<Mesh::TriIndices>& tris) {
    unsigned i = 0;
    while(i < indices.size()) {
        assert(i + 2 < indices.size());
        Mesh::TriIndices tri;
        tri.indices[0] = indices[i + 0];
        tri.indices[1] = indices[i + 1];
        tri.indices[2] = indices[i + 2];
        tris.push_back(tri);
        i += 3;
    }
}

static void GenerateTrianglesFromTriangleStrips(const std::vector<Mesh::Index>& indices, std::vector<Mesh::TriIndices>& tris) {
    if(indices.size() < 3) return;
    unsigned i = 0;
    while(i < indices.size()) {
        assert(i + 1 < indices.size());
        Mesh::Index lastEdgeIdx[2] = { indices[i], indices[i + 1] };
        i += 2;
        unsigned j = 0;
        while(i < indices.size()) {
            Mesh::Index idx = indices[i];
            if(Mesh::RESTART_INDEX == idx) break;
            Mesh::TriIndices tri;
            if(j % 2) {
                tri.indices[0] = lastEdgeIdx[1];
                tri.indices[1] = lastEdgeIdx[0];
            } else {
                tri.indices[0] = lastEdgeIdx[0];
                tri.indices[1] = lastEdgeIdx[1];
            }
            tri.indices[2] = idx;
            tris.push_back(tri);
            lastEdgeIdx[0] = lastEdgeIdx[1];
            lastEdgeIdx[1] = idx;
            j++;
            i++;
        }
        i++;
    }
}

static void GenerateTrianglesFromTriangleFan(const std::vector<Mesh::Index>& indices, std::vector<Mesh::TriIndices>& tris) {
    if(indices.size() < 3) return;
    unsigned i = 0;
    while(i < indices.size()) {
        assert(i + 1 < indices.size());
        Mesh::Index base = indices[i++];
        Mesh::Index lastEdgeIdx = indices[i++];
        while(i < indices.size()) {
            Mesh::Index idx = indices[i];
            if(Mesh::RESTART_INDEX == idx) break;
            Mesh::TriIndices tri;
            tri.indices[0] = base;
            tri.indices[1] = lastEdgeIdx;
            tri.indices[2] = idx;
            tris.push_back(tri);
            lastEdgeIdx = idx;
            i++;
        }
        i++;
    }
}

static void GenerateTriangles(const std::vector<Mesh::Index>& indices, const GLenum primType, std::vector<Mesh::TriIndices>& tris) {
    tris.clear();
    switch(primType) {
    case GL_TRIANGLES: GenerateTrianglesFromTriangles(indices, tris); break;
    case GL_TRIANGLE_STRIP: GenerateTrianglesFromTriangleStrips(indices, tris); break;
    case GL_TRIANGLE_FAN: GenerateTrianglesFromTriangleFan(indices, tris); break;
    default:
        common.printf("ERROR - GenerateTriangles: unkown primitive type desc.primType = %d.\n", primType);
        Crash();
    }
}

static void Subdiv(unsigned subdiv, std::vector<Mesh::Vertex>& vertices, std::vector<Mesh::TriIndices>& triIndices) {
    typedef std::vector<Mesh::TriIndices>::iterator triIt_t;
    std::vector<Mesh::TriIndices> T(triIndices);
    triIndices.clear();
    for(triIt_t triIt(T.begin()); T.end() != triIt; ++triIt) {
        M::Vector3 center = M::Vector3::Zero;
        for(unsigned i = 0; i < 3; ++i) center += vertices[triIt->indices[i]].position;
        center /= 3.0f;
        unsigned idx = vertices.size();
        Mesh::Vertex vert;
        vert.position = center;
        vert.color = vertices[triIt->indices[0]].color;
        vert.normal = vertices[triIt->indices[0]].normal;
        vertices.push_back(vert);

        Mesh::Index id[] = {
            triIt->indices[0],
            triIt->indices[1],
            triIt->indices[2],
            idx
        };

        Mesh::TriIndices t[3];
        t[0].indices[0] = id[0];
        t[0].indices[1] = id[1];
        t[0].indices[2] = id[3];
        t[1].indices[0] = id[1];
        t[1].indices[1] = id[2];
        t[1].indices[2] = id[3];
        t[2].indices[0] = id[2];
        t[2].indices[1] = id[0];
        t[2].indices[2] = id[3];
        triIndices.push_back(t[0]);
        triIndices.push_back(t[1]);
        triIndices.push_back(t[2]);
    }
}

Mesh::Mesh(const Desc& desc) : _invalidate(false), _gbHandle(GB_INVALID_HANDLE) {
    _vertices.resize(desc.numVertices);
    _indices.resize(desc.numIndices);

    memcpy(&_vertices[0], desc.vertices, sizeof(Vertex) * desc.numVertices);
    memcpy(&_indices[0], desc.indices, sizeof(Index) * desc.numIndices);

    _primType = desc.primType;
}

Mesh::~Mesh() {
    if(GB_INVALID_HANDLE != _gbHandle) GB_FreeMemItem(_gbHandle);
}

bool Mesh::IsCached() const {
    return GB_IsCached(_gbHandle);
}

static inline bool IsSolid(GLenum primType) {
    switch(primType) {
    case GL_POINTS:         return false;

    case GL_LINES:          return false;
    case GL_LINE_STRIP:     return false;
    case GL_LINE_LOOP:      return false;

    case GL_TRIANGLES:      return true;
    case GL_TRIANGLE_STRIP: return true;
    case GL_TRIANGLE_FAN:   return true;
    }
    // omitted: gl_{lines, triangles}_adjacency, gl_patches
    assert(false && "IsSolid(primType): unknown primitive type");
    return false;
}

bool Mesh::IsSolid() const {
    return R::IsSolid(_primType);
}

GLenum Mesh::PrimitiveType() const { return _primType; }

unsigned Mesh::NumIndices() const { return _indices.size(); }

const Mesh::Index* Mesh::Indices() const { return &_indices[0]; }

void Mesh::Invalidate(Mesh::Vertex* const vertices) {
    SYS::ScopedLock lock(_mtx);
    memcpy(&_vertices[0], vertices, sizeof(Mesh::Vertex) * _vertices.size());
    _invalidate = true;
}

void Mesh::Invalidate(Mesh::Index* const indices, unsigned numIndices) {
    SYS::ScopedLock lock(_mtx);
    _indices.resize(numIndices);
    memcpy(&_indices[0], indices, sizeof(Index) * numIndices);
    _triangleIndices.clear();
}

void Mesh::R_AppendTriangles(std::vector<Triangle>& tris, const M::Vector3& eye) {
    assert(IsSolid());
    SYS::ScopedLock lock(_mtx);
    if(_triangleIndices.empty()) GenerateTriangles(_indices, _primType, _triangleIndices);
    unsigned idxOff = GB_GetOffset(_gbHandle) / sizeof(Mesh::Vertex);
    for(unsigned i = 0; i < _triangleIndices.size(); ++i) {
        Triangle tri;
        M::Vector3 center = M::Vector3::Zero;
        M::Vector3 p[3];
        for(unsigned j = 0; j < 3; ++j) {
            tri.bufIndices.indices[j] = _triangleIndices[i].indices[j] + idxOff; 
            p[j] = _vertices[_triangleIndices[i].indices[j]].position;
            center += p[j];
        }
        const M::Vector3 normal = M::Normalize(M::Cross(p[1] - p[0], p[2] - p[0]));
        const M::Vector3 view   = M::Normalize(eye - p[0]);
        tri.viewAngle = M::Dot(view, normal);
        center /= 3.0f;
        tri.dist = M::Distance(eye, center);
        tris.push_back(tri);
    }
}

void Mesh::R_AllocBuffer() {
    SYS::ScopedLock lock(_mtx);
    if(GB_INVALID_HANDLE == _gbHandle) {
        _gbHandle = GB_AllocMemItem(&_vertices[0], _vertices.size());
    }
    GB_Touch(_gbHandle);
}

void Mesh::R_TouchBuffer() {
    SYS::ScopedLock lock(_mtx);
    assert(GB_INVALID_HANDLE != _gbHandle);
    if(_invalidate) {
        GB_Invalidate(_gbHandle);
        _invalidate = false;
    }
    GB_Cache(_gbHandle);
}

} // namespace R
