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

extern COM::Config::Variable<float> cvar_nodeSize;
extern COM::Config::Variable<int>	cvar_nodeSubdiv;

namespace W {

struct PolyhedronNodes {
    // valid[i] != 0, if a node n exists with n->id() == i
    // and 0, otherwise
    std::vector<int>        valid;
    std::vector<M::Vector3> positions;
    std::vector<R::Color>   colors;
};

struct PolyhedronHullFaceList {
	unsigned base;
	unsigned size;
};

struct PolyhedronHullFaceTrans {
    // base vertex is local origin
    M::Matrix3  localToWorld;
    M::Vector3  normal;
};

struct PolyhedronHullEdge {
	enum { INVALID_FACE_INDEX = 0xFFFFFFFF };
	unsigned faceIdx;
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
    std::vector<PolyhedronHullFaceTrans>    faceTrans;
	std::vector<PolyhedronHullEdge>         edges;
	std::vector<unsigned>                   vnmap; // maps vertices to nodes
	std::vector<R::Mesh::Vertex>            vertices;
	std::vector<R::Mesh::Index>             indices; // hull exists iff indices not empty
    std::vector<PolyhedronFaceCurve>        curves;
	R::meshPtr_t                            mesh;
};

struct ENT_Polyhedron {
	const graph_t*              G;
    PolyhedronNodes             nodes;
	PolyhedronHull              hull;
    R::RenderList               renderList;
};

void Polyhedron_InitResources(void);
void Polyhedron_Init(ENT_Polyhedron& ph);
void Polyhedron_Rebuild(ENT_Polyhedron& ph);
void Polyhedron_BuildRenderList(ENT_Polyhedron& ph);
void Polyhedron_Update(ENT_Polyhedron& ph);
void Polyhedron_AddCurve(ENT_Polyhedron& ph, leda::edge edge, const R::Color& color);
void Polyhedron_UpdateCurve(PolyhedronFaceCurve& cv, float secsPassed);

} // namespace W