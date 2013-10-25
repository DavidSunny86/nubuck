#include <vector>
#include <string.h>

#include <renderer\giantbuffer\giant_buffer.h>
#include "mesh.h"

namespace R {

static void GenerateTrianglesFromTriangleStrips(const Mesh::Desc& desc, std::vector<Mesh::TriIndices>& tris) {
    if(desc.numIndices < 3) return;
    unsigned i = 0;
    while(i < desc.numIndices) {
        assert(i + 1 < desc.numIndices);
        Mesh::Index lastEdgeIdx[2] = { desc.indices[i], desc.indices[i + 1] };
        i += 2;
        unsigned j = 0;
        while(i < desc.numIndices) {
            Mesh::Index idx = desc.indices[i];
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

static void GenerateTrianglesFromTriangleFan(const Mesh::Desc& desc, std::vector<Mesh::TriIndices>& tris) {
    if(desc.numIndices < 3) return;
    unsigned i = 0;
    while(i < desc.numIndices) {
        assert(i + 1 < desc.numIndices);
        Mesh::Index base = desc.indices[i++];
        Mesh::Index lastEdgeIdx = desc.indices[i++];
        while(i < desc.numIndices) {
            Mesh::Index idx = desc.indices[i];
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

static void GenerateTriangles(const Mesh::Desc& desc, std::vector<Mesh::TriIndices>& tris) {
    tris.clear();
    switch(desc.primType) {
    case GL_TRIANGLE_STRIP: GenerateTrianglesFromTriangleStrips(desc, tris); break;
    case GL_TRIANGLE_FAN: GenerateTrianglesFromTriangleFan(desc, tris); break;
    default:
        common.printf("ERROR - GenerateTriangles: unkown primitive type desc.primType = %d.\n", desc.primType);
        Crash();
    }
}

template<typename TYPE>
class ArrayPtr {
private:
    TYPE* _mem;
public:
    // deep copy
    ArrayPtr(const TYPE* const mem, unsigned numElements) {
        _mem = new TYPE[numElements];
        memcpy(_mem, mem, sizeof(TYPE) * numElements);
    }
    ~ArrayPtr() {
        if(_mem) delete[] _mem;
    }

    TYPE* Release(void) {
        TYPE* res = _mem;
        _mem = NULL;
        return res;
    }
};

Mesh::Mesh(const Desc& desc) : _gbHandle(GB_INVALID_HANDLE), _compiled(false) {
    ArrayPtr<Vertex> vertexArray(desc.vertices, desc.numVertices);
    ArrayPtr<Index> indexArray(desc.indices, desc.numIndices);

    _desc.vertices = vertexArray.Release();
    _desc.numVertices = desc.numVertices;

    _desc.indices = indexArray.Release();
    _desc.numIndices = desc.numIndices;

    _desc.primType = desc.primType;
}

Mesh::~Mesh(void) {
    if(_desc.vertices) delete[] _desc.vertices;
    if(_desc.indices) delete[] _desc.indices;
    if(GB_INVALID_HANDLE != _gbHandle) GB_FreeMemItem(_gbHandle);
}

void Mesh::AppendTriangles(std::vector<Triangle>& tris, const M::Vector3& eye, const M::Matrix4& worldMat) {
    unsigned idxOff = GB_GetOffset(_gbHandle) / sizeof(Mesh::Vertex);
    for(unsigned i = 0; i < _triangleIndices.size(); ++i) {
        Triangle tri;
        M::Vector3 center = M::Vector3::Zero;
        for(unsigned j = 0; j < 3; ++j) {
            tri.bufIndices.indices[j] = _triangleIndices[i].indices[j] + idxOff; 
            center += M::Transform(worldMat, _desc.vertices[_triangleIndices[i].indices[j]].position);
        }
        center /= 3.0f;
        tri.dist = M::Distance(M::Vector3::Zero, center);
        tris.push_back(tri);
    }
}

void Mesh::Invalidate(Mesh::Vertex* vertices) {
    _mtx.Lock();
    memcpy(_desc.vertices, vertices, sizeof(Mesh::Vertex) * _desc.numVertices);
    _compiled = false;
    if(GB_INVALID_HANDLE != _gbHandle)
        GB_Invalidate(_gbHandle);
    _mtx.Unlock();
}

void Mesh::R_Touch(void) { GB_Touch(_gbHandle); }

void Mesh::R_Compile(void) {
    if(_compiled) return;

    _mtx.Lock();

    GenerateTriangles(_desc, _triangleIndices);

    if(GB_INVALID_HANDLE == _gbHandle) {
        _gbHandle = GB_AllocMemItem(_desc.vertices, _desc.numVertices);
    }
    _compiled = true;

    _mtx.Unlock();
}

} // namespace R
