#include <maxint.h>

#include <Nubuck\polymesh.h>
#include <renderer\mesh\meshmgr.h>
#include "r_point_nodes.h"

namespace R {

void PointNodes::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
    if(_tfmesh) {
        meshMgr.Destroy(_tfmesh);
        _tfmesh = NULL;
    }
}

void PointNodes::Rebuild(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
    _nodes.clear();
    _inMap.clear();
    _inMap.resize(mesh.max_node_index() + 1);

    Node        rv;
    leda::node  pv;
    forall_nodes(pv, mesh) {
        rv.pvert    = pv;
        rv.position = fpos[pv->id()];
        rv.color    = mesh.color_of(pv);

        _inMap[pv->id()] = _nodes.size();
        _nodes.push_back(rv);
    }

    const unsigned numVerts = _nodes.size();

	const float nodeSize = cvar_r_nodeSize;

    _vertices.clear();
    _vertices.resize(numVerts);

    for(unsigned i = 0; i < numVerts; ++i) {
        _vertices[i].position   = _nodes[i].position;
        _vertices[i].normal.z   = nodeSize; // matches billboard nodes layout
        _vertices[i].color      = _nodes[i].color;
    }

    _indices.clear();
    _indices.resize(numVerts);
    for(unsigned i = 0; i < numVerts; ++i) {
        _indices[i] = i;
    }

    _needsRebuild = true;
}

void PointNodes::Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
    typedef leda::nb::RatPolyMesh::State state_t;
    for(unsigned i = 0; i < _nodes.size(); ++i) {
        leda::node pv = _nodes[i].pvert;
        if(state_t::GEOMETRY_CHANGED == mesh.state_of(pv)) {
            _nodes[i].position = fpos[_nodes[i].pvert->id()];
            _nodes[i].color = mesh.color_of(_nodes[i].pvert);
            _vertices[i].position = _nodes[i].position;
            _vertices[i].color = _nodes[i].color;

            const unsigned vertSz = sizeof(Mesh::Vertex);
            const unsigned off = vertSz * (&_vertices[i] - &_vertices[0]);
            if(_mesh) meshMgr.GetMesh(_mesh).Invalidate(&_vertices[0], off, vertSz);
        }
    }
}

void PointNodes::SetColor(leda::node pv, const Color& color) {
    const unsigned ridx = _inMap[pv->id()];
    _vertices[ridx].color = _nodes[ridx].color = color;

    if(!_needsRebuild && _mesh) {
        const unsigned vertSz = sizeof(Mesh::Vertex);
        const unsigned off = vertSz * (&_vertices[ridx] - &_vertices[0]);

        meshMgr.GetMesh(_mesh).Invalidate(&_vertices[0], off, vertSz);
    }
}

void PointNodes::Transform(const M::Matrix4& objToWorld) {
    if(_tfmesh) meshMgr.GetMesh(_tfmesh).SetTransform(objToWorld);
}

void PointNodes::BuildRenderMesh() {
    if(_isInvalid && _mesh) {
        meshMgr.GetMesh(_mesh).Invalidate(&_vertices[0]);
        _isInvalid = false;
    }

    M::Matrix4 lastTransform = M::Mat4::Identity();
    if(_tfmesh) lastTransform = meshMgr.GetMesh(_tfmesh).GetTransform();

    if(_needsRebuild && !_nodes.empty()) {
        if(_mesh) DestroyMesh();

        if(!_nodes.empty()) {
            const unsigned numVerts = _vertices.size();

            Mesh::Desc meshDesc;
            meshDesc.vertices = &_vertices[0];
            meshDesc.numVertices = numVerts;
            meshDesc.indices = &_indices[0];
            meshDesc.numIndices = numVerts;
            meshDesc.primType = GL_POINTS;

            _mesh = meshMgr.Create(meshDesc);
            _tfmesh = meshMgr.Create(_mesh);
            meshMgr.GetMesh(_tfmesh).SetTransform(lastTransform);
        }

        _needsRebuild = false;
    }
}

void PointNodes::DestroyRenderMesh() {
    DestroyMesh();
}

MeshJob PointNodes::GetRenderJob() const {
    assert(!_nodes.empty());

    MeshJob meshJob;
    meshJob.fx = "UnlitThickLines";
    meshJob.tfmesh = _tfmesh;
    meshJob.material = Material::White;
    meshJob.primType = 0;
    return meshJob;
}

} // namespace R