#pragma once

#include <vector>

#include <Windows.h> // defines MAX_INT
#include <LEDA\graph\node_array.h>

#include <common\config\config.h>
#include <common\types.h>
#include <math\matrix3.h>
#include <math\vector3.h>
#include <renderer\renderer.h>
#include <renderer\mesh\bezier\bezier.h>

namespace W {

enum {
    POLYHEDRON_VALID_FLAG    = (1 << 0),
    POLYHEDRON_HIDE_FLAG     = (1 << 1),

    POLYHEDRON_UPDATE_MESH_VERTICES = (1 << 0),
    POLYHEDRON_UPDATE_MESH_INDICES  = (1 << 1),
    POLYHEDRON_REBUILD_MESH
};

struct PolyhedronNode {
    int flags;

    PolyhedronNode() : flags(0) { }
};

struct PolyhedronNodes {
    std::vector<PolyhedronNode> nodes;
    std::vector<M::Vector3>     positions;
    std::vector<M::Vector3> 	oldPositions;
    std::vector<R::Color>   	colors;
};

enum { POLYHEDRON_INVALID_INDEX = 0xFFFFFFFF };

struct PolyhedronEdge {
    int         flags;
    unsigned    faceIdx;
    unsigned    n0, n1;     // source, target
    unsigned    rev;        // reversal edge
    R::Color    color;
};

struct PolyhedronEdgeColor {
    R::Color    v0, v1, cur;
    float       t;
    bool        ip;
};

struct PolyhedronEdges {
    std::vector<PolyhedronEdge>         edges;
    std::vector<PolyhedronEdgeColor>    colors;

    R::Color                            baseColor;
    float       				        radius;
};

struct PolyhedronFace {
    int flags;
};

struct PolyhedronFaceList {
	unsigned base;
	unsigned size;
};

struct PolyhedronFaceColor {
    R::Color    v0, v1, cur;
    float       t;
    bool        ip;
};

struct PolyhedronFaceTrans {
    // base vertex is local origin
    M::Matrix3  localToWorld;
    M::Vector3  normal;
};

struct PolyhedronFaceCurve {
    unsigned        faceIdx;
    float           time;
    R::Color        color;
    M::Matrix3      localToWorld; // face transformation. face's base vertex is local origin.
    M::Vector3      origin, normal;
    R::PolyBezier2U curve; // curve stores points in face's local space. 
};

struct PolyhedronFaces {
    std::vector<PolyhedronFace>             faces;
    std::vector<PolyhedronFaceList>         lists;
    std::vector<PolyhedronFaceColor>    	colors;
    std::vector<PolyhedronFaceTrans>    	trans;
    std::vector<unsigned>                   freeIndices;
    unsigned                                maxFaceIdx;
    std::vector<PolyhedronFaceCurve>        curves; // TODO: vector of vectors!
    R::Color                                baseColor;
};

struct PolyhedronMesh {
	std::vector<unsigned>                   vnmap; // maps vertices to nodes
	std::vector<R::Mesh::Vertex>            vertices;
	std::vector<R::Mesh::Index>             indices; // hull exists iff indices not empty
    R::MeshMgr::meshPtr_t                   mesh;
};

struct PolyhedronSelection {
    std::vector<bool> nodes;
};

struct ENT_Polyhedron {
    unsigned                entId;
    int                     flags;
    int                     renderFlags;
    bool                    isPickable;
    PolyhedronNodes         nodes;
    PolyhedronEdges         edges;
    PolyhedronFaces         faces;
    PolyhedronMesh          mesh;
    PolyhedronSelection     selection;
    R::RenderList           renderList;
};

struct INF_Polyhedron {
    float           edgeRadius;
    R::Color        edgeColor;
};

void Polyhedron_InitResources(void);
void Polyhedron_Init(ENT_Polyhedron& ph);
void Polyhedron_PrintSize(ENT_Polyhedron& ph);
void Polyhedron_Update(ENT_Polyhedron& ph);
void Polyhedron_Rebuild(ENT_Polyhedron& ph, const graph_t& G, leda::node_map<bool>& cachedNodes, leda::edge_map<bool>& cachedEdges);
void Polyhedron_BuildRenderList(ENT_Polyhedron& ph, const std::string& hullFx);
void Polyhedron_Update(ENT_Polyhedron& ph, const graph_t& G);
void Polyhedron_AddCurve(ENT_Polyhedron& ph, leda::edge edge, const R::Color& color);
void Polyhedron_UpdateCurve(PolyhedronFaceCurve& cv, float secsPassed);
void Polyhedron_UpdateFaceColors(ENT_Polyhedron& ph, float secsPassed);
void Polyhedron_SetFaceColor(ENT_Polyhedron& ph, const leda::edge e, const R::Color& color);
void Polyhedron_SetFaceVisibility(ENT_Polyhedron& ph, const leda::edge e, bool visible);
void Polyhedron_SetHullAlpha(ENT_Polyhedron& ph, float alpha);
bool Polyhedron_RaycastNodes(ENT_Polyhedron& ph, const M::Vector3& rayOrig, const M::Vector3& rayDir, leda::node& hitNode);
bool Polyhedron_RaycastFaces(ENT_Polyhedron& ph, const M::Vector3& rayOrig, const M::Vector3& rayDir, leda::edge& hitFace);
void Polyhedron_GetInfo(const ENT_Polyhedron& ph, INF_Polyhedron& inf);

} // namespace W