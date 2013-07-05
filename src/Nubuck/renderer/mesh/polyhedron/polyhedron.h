#pragma once

#include <vector>

#include <LEDA\graph\graph.h>
#include <LEDA\geo\d3_rat_point.h>

#include "..\mesh.h"

namespace R {

    class PolyhedronMesh {
    public:
        typedef leda::rational              scalar_t;
        typedef leda::d3_rat_point          point_t;
        typedef leda::GRAPH<point_t, int>   graph_t;
    private:
        struct Face {
            unsigned base;
            unsigned size;
        };

        struct Edge {
            enum { INVALID_INDEX = 0xFFFFFFFF };
            unsigned faceIndex;
            
            Edge(void);
        };

        const graph_t& _G;

        // position of _nodes[i] is _vertices[i].
        // _nodes is used for fast updates of _vertices
        std::vector<leda::node>     _nodes;
        std::vector<Mesh::Vertex>   _vertices;
        std::vector<Mesh::Index>    _indices;

        std::vector<Face>       _faces;
        leda::edge_array<Edge>  _edges;

        void Set(const Face& face, const Color& color);

        void Rebuild(void);
    public:
        PolyhedronMesh(const graph_t& G);

        unsigned NumFaces(void) const;
        unsigned FaceOf(leda::edge e) const;

        // recomputes vertex positions from nodes.
        // assumes unchanged graph
        void Update(void);

        const graph_t&  GetGraph(void) const;

        // leaves primitive type undefined!
        // use:
        // GL_TRIANGLE_FAN for solid polyhedron and
        // GL_LINE_LOOP for wireframe polyhedron
        Mesh::Desc      GetDesc(void);

        void Set(leda::edge edge, const Color& color);
    };

} // namespace R