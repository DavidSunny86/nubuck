#pragma once

#include <vector>

#include <LEDA\graph\graph.h>
#include <LEDA\geo\d3_rat_point.h>

#include "..\mesh.h"

namespace R {

    class PolyhedronMesh : public MeshDesc {
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

        std::vector<Vertex> _vertices;
        std::vector<Index>  _indices;

        std::vector<Face>       _faces;
        leda::edge_array<Edge>  _edges;

        void Set(const Face& face, const Color& color);

        void Rebuild(void);
    public:
        PolyhedronMesh(const graph_t& G);

        unsigned NumFaces(void) const;
        unsigned FaceOf(leda::edge e) const;

        const graph_t&  GetGraph(void) const;
        Desc            GetDesc(void) const override;

        void Set(leda::edge edge, const Color& color);
    };

} // namespace R