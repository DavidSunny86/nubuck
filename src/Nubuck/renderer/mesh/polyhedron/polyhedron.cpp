#include <common\common.h>
#include "polyhedron.h"

namespace {

    M::Vector3 ToVector(const leda::d3_rat_point& p) {
        const leda::d3_point fp = p.to_float();
        return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
    }

} // unnamed namespace

namespace R {

    PolyhedronMesh::Edge::Edge(void) : faceIndex(INVALID_INDEX) {
    }

    void PolyhedronMesh::Set(const Face& face, const Color& color) {
        for(unsigned i = 0; i < face.size; ++i)
            _vertices[face.base + i].color = color;
    }

    void PolyhedronMesh::Rebuild(void) {
        common.printf("INFO - rebuilding polyhedron mesh with |V| = %d, |E| = %d.\n",
            _G.number_of_nodes(), _G.number_of_edges());

        _vertices.clear();
        _indices.clear();

        _faces.clear();
        _edges.init(_G);

        leda::edge_array<bool> visitedEdge(_G, false);

        unsigned indexCnt = 0;

        leda::edge e;
        forall_edges(e, _G) {
            if(!visitedEdge[e]) {
                leda::edge it = _G.face_cycle_succ(e);

                const M::Vector3 p0(ToVector(_G[source(e)]));
                const M::Vector3 p1(ToVector(_G[source(it)]));
                const M::Vector3 p2(ToVector(_G[target(it)]));

                const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

                Face face;
                face.base = indexCnt; // index of v0
                face.size = 2; // v0 and v1

                _faces.push_back(face);
                unsigned faceIndex = _faces.size() - 1;
                Face& faceRef = _faces.back();

                Vertex vert;
                vert.normal = normal;
                vert.color  = Color::White;

                vert.position = p0;
                _vertices.push_back(vert);
                _indices.push_back(indexCnt);
                indexCnt++;

                vert.position = p1;
                _vertices.push_back(vert);
                _indices.push_back(indexCnt);
                indexCnt++;

                _edges[e].faceIndex = _edges[it].faceIndex = faceIndex;
                visitedEdge[e] = visitedEdge[it] = true;

                while((it = _G.face_cycle_succ(it)) != e) {
                    vert.position = ToVector(_G[source(it)]);
                    _vertices.push_back(vert);
                    _indices.push_back(indexCnt++);

                    visitedEdge[it] = true;
                    _edges[it].faceIndex = faceIndex;
                    faceRef.size += 1;
                }

                _indices.push_back(RESTART_INDEX);
            }
        } // forall_edges

#ifdef PARANOID
        forall_edges(e, _G) {
            assert(visitedEdge[e]);
            assert(Edge::INVALID_INDEX != _edges[e].faceIndex);
        }
#endif
    }

    PolyhedronMesh::PolyhedronMesh(const graph_t& G) : _G(G) {
        Rebuild();
    }

    unsigned PolyhedronMesh::NumFaces(void) const {
        return _faces.size();
    }

    unsigned PolyhedronMesh::FaceOf(leda::edge e) const {
        return _edges[e].faceIndex;
    }

    const PolyhedronMesh::graph_t& PolyhedronMesh::GetGraph(void) const {
        return _G;
    }

    MeshDesc PolyhedronMesh::GetSolidDesc(void) {
        MeshDesc desc;

        desc.vertices = &_vertices[0];
        desc.numVertices = _vertices.size();
        desc.indices = &_indices[0];
        desc.numIndices = _indices.size();
        desc.primType = GL_TRIANGLE_FAN;

        return desc;
    }

    void PolyhedronMesh::Set(leda::edge edge, const Color& color) {
        Set(_faces[_edges[edge].faceIndex], color);
    }

} // namespace R