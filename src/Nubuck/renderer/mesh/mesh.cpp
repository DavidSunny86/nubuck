#include <string.h>

#include "mesh.h"

namespace R {

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

    Mesh::Mesh(const Desc& desc) : _compiled(false), _radius(0.0f) {
        ArrayPtr<Vertex> vertices(desc.vertices, desc.numVertices);
        ArrayPtr<Index> indices(desc.indices, desc.numIndices);

        _desc.vertices = vertices.Release();
        _desc.numVertices = desc.numVertices;

        _desc.indices = indices.Release();
        _desc.numIndices = desc.numIndices;

        _desc.primType = desc.primType;
    }

    Mesh::~Mesh(void) {
        if(_desc.vertices) delete[] _desc.vertices;
        if(_desc.indices) delete[] _desc.indices;
    }

    void Mesh::R_Compile(void) {
        if(_compiled) return;

        // compute radius of bounding sphere
        float rs = 0.0f;
        for(unsigned i = 0; i < _desc.numVertices; ++i)
            rs = M::Max(rs, M::Dot(_desc.vertices[i].position, _desc.vertices[i].position));
        _radius = sqrtf(rs);

        _vertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, _desc.vertices, sizeof(Vertex) * _desc.numVertices));
        _indexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, _desc.indices, sizeof(Index) * _desc.numIndices));
        _compiled = true;
    }

    void Mesh::R_Bind(void) {
        _vertexBuffer->Bind();
        _indexBuffer->Bind();
    }

} // namespace R
