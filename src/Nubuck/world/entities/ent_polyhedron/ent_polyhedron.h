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

struct PolyhedronNodes {
    // valid[i] != 0, if a node n exists with n->id() == i
    // and 0, otherwise
    std::vector<int>        valid;
    std::vector<M::Vector3> positions;
    std::vector<M::Vector3> oldPositions;
    std::vector<R::Color>   colors;
};

struct PolyhedronEdges {
    std::vector<int>    valid;
    std::vector<int>	mask;

    R::Color            color;
    float               radius;
};

struct PolyhedronHullFaceList {
	unsigned base;
	unsigned size;
};

struct PolyhedronHullFaceColor {
    R::Color    v0, v1, cur;
    float       t;
    bool        ip;
};

struct PolyhedronHullFaceTrans {
    // base vertex is local origin
    M::Matrix3  localToWorld;
    M::Vector3  normal;
};

struct PolyhedronHullEdge {
    // edge is valid iff INVALID_FACE_INDEX != faceIdx
	enum { INVALID_FACE_INDEX = 0xFFFFFFFF };
	unsigned    faceIdx;
    unsigned    n0, n1;
    bool        valid;
};

struct PolyhedronFaceCurve {
    unsigned        faceIdx;
    float           time;
    R::Color        color;
    M::Matrix3      localToWorld; // face transformation. face's base vertex is local origin.
    M::Vector3      origin, normal;
    R::PolyBezier2U curve; // curve stores points in face's local space. 
};

struct PolyhedronHull {
	std::vector<PolyhedronHullFaceList>     faceLists;
    std::vector<PolyhedronHullFaceColor>    faceColors;
    std::vector<PolyhedronHullFaceTrans>    faceTrans;
	std::vector<PolyhedronHullEdge>         edges;
	std::vector<unsigned>                   vnmap; // maps vertices to nodes
	std::vector<R::Mesh::Vertex>            vertices;
	std::vector<R::Mesh::Index>             indices; // hull exists iff indices not empty
    std::vector<PolyhedronFaceCurve>        curves;
    R::MeshMgr::meshPtr_t                   mesh;
    float                                   alpha;
};

struct PolyhedronSelection {
    std::vector<bool> nodes;
};

struct ENT_Polyhedron {
    unsigned                entId;
	graph_t*                G;
    int                     renderFlags;
    bool                    isPickable;
    PolyhedronNodes         nodes;
    PolyhedronEdges         edges;
	PolyhedronHull          hull;
    PolyhedronSelection     selection;
    R::RenderList           renderList;
};

struct INF_Polyhedron {
    float           edgeRadius;
    R::Color        edgeColor;
};

void Polyhedron_InitResources(void);
void Polyhedron_Init(ENT_Polyhedron& ph);
void Polyhedron_Rebuild(ENT_Polyhedron& ph);
void Polyhedron_BuildRenderList(ENT_Polyhedron& ph, const std::string& hullFx);
void Polyhedron_Update(ENT_Polyhedron& ph);
void Polyhedron_AddCurve(ENT_Polyhedron& ph, leda::edge edge, const R::Color& color);
void Polyhedron_UpdateCurve(PolyhedronFaceCurve& cv, float secsPassed);
void Polyhedron_UpdateFaceColors(ENT_Polyhedron& ph, float secsPassed);
void Polyhedron_SetFaceColor(ENT_Polyhedron& ph, const leda::edge e, const R::Color& color);
void Polyhedron_HideFace(ENT_Polyhedron& ph, const leda::edge e);
void Polyhedron_SetHullAlpha(ENT_Polyhedron& ph, float alpha);
bool Polyhedron_RaycastNodes(ENT_Polyhedron& ph, const M::Vector3& rayOrig, const M::Vector3& rayDir, leda::node& hitNode);
bool Polyhedron_RaycastFaces(ENT_Polyhedron& ph, const M::Vector3& rayOrig, const M::Vector3& rayDir, leda::edge& hitFace);
void Polyhedron_GetInfo(const ENT_Polyhedron& ph, INF_Polyhedron& inf);

} // namespace W