#include <assert.h>

#include <renderer\glcall.h>
#include <renderer\program\program.h>
#include "staticbuffer.h"
#include "mesh.h"

namespace {

    enum {
        IN_POSITION     = 0,
        IN_NORMAL       = 1,
        IN_COLOR        = 2
    };

    void BindVertices(void) {
        GL_CALL(glVertexAttribPointer(IN_POSITION,
            3, GL_FLOAT, GL_FALSE, sizeof(R::Vertex),
            (void*)offsetof(R::Vertex, position)));
        GL_CALL(glEnableVertexAttribArray(IN_POSITION));

        GL_CALL(glVertexAttribPointer(IN_NORMAL,
            3, GL_FLOAT, GL_FALSE, sizeof(R::Vertex),
            (void*)offsetof(R::Vertex, normal)));
        GL_CALL(glEnableVertexAttribArray(IN_NORMAL));

        GL_CALL(glVertexAttribPointer(IN_COLOR,
            4, GL_FLOAT, GL_FALSE, sizeof(R::Vertex),
            (void*)offsetof(R::Vertex, color)));
        GL_CALL(glEnableVertexAttribArray(IN_COLOR));
    }

    template<typename T> struct ToGLEnum { };
    template<> struct ToGLEnum<unsigned>    { enum { ENUM = GL_UNSIGNED_INT }; };
    template<> struct ToGLEnum<int>         { enum { ENUM = GL_INT }; };

} // unnamed namespace

namespace R {

    void Mesh::Invalidate(void) {
        _compiled = false;
    }

    void Mesh::Compile(void) {
        if(_compiled) return;

        MeshDesc::Desc desc = _meshDesc->GetDesc();

        _vertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(
                    GL_ARRAY_BUFFER,
                    desc.vertices,
                    desc.numVertices * sizeof(Vertex)));
        _indexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(
                    GL_ELEMENT_ARRAY_BUFFER,
                    desc.indices,
                    desc.numIndices * sizeof(Index)));

        _compiled = true;
    }

    Mesh::Mesh(const GEN::Pointer<MeshDesc>& meshDesc) : _meshDesc(meshDesc), _compiled(false) { 
    }

    Mesh::~Mesh(void) { }

    void Mesh::Bind(void) const {
        _vertexBuffer->Bind();
        BindVertices();

        _indexBuffer->Bind();
    }

    void Mesh::Draw(void) const {
#ifdef PARANOID
        assert(GL_UNSIGNED_INT == ToGLEnum<unsigned>::ENUM);
        assert(GL_INT == ToGLEnum<int>::ENUM);
#endif
        MeshDesc::Desc desc = _meshDesc->GetDesc();
        GL_CALL(glDrawElements(desc.primType, desc.numIndices, ToGLEnum<Index>::ENUM, NULL));
    }

} // namespace R
