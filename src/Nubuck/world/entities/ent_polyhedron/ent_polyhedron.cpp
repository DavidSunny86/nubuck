#include <assert.h>
#include <algorithm>

#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\quad\quad.h>
#include "ent_polyhedron.h"

COM::Config::Variable<float>    cvar_nodeSize("nodeSize", 0.2f);
COM::Config::Variable<int>      cvar_nodeSubdiv("nodeSubdiv", 2);

COM::Config::Variable<float>    cvar_faceCurveSpeed("faceSpeed", 0.2f);
COM::Config::Variable<float>    cvar_faceCurveDecalSize("faceDecalSize", 0.2f);
COM::Config::Variable<float>    cvar_faceCurveSpacing("faceSpacing", 0.4f);
COM::Config::Variable<float>    cvar_faceCurveCurvature("faceCurvature", 0.2f);

static M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

namespace W {

static R::meshPtr_t         g_nodeMesh;
static R::meshPtr_t         g_faceCurveDecalMesh;
static R::SkinMgr::handle_t g_faceCurveDecalSkin;

void Polyhedron_InitResources(void) {
	R::Sphere sphere(cvar_nodeSubdiv, true);
    sphere.Scale(cvar_nodeSize);
	g_nodeMesh = R::meshMgr.Create(sphere.GetDesc());
    g_faceCurveDecalMesh = R::meshMgr.Create(R::CreateQuadDesc(cvar_faceCurveDecalSize));
    R::SkinDesc skinDesc;
    skinDesc.diffuseTexture = "C:\\Libraries\\LEDA\\LEDA-6.4\\res\\Textures\\circle.tga";
    g_faceCurveDecalSkin = R::skinMgr.Create(skinDesc);
}

void Polyhedron_Init(ENT_Polyhedron&) { }

static void Polyhedron_RebuildNodes(ENT_Polyhedron& ph) {
    ph.nodes.positions.resize(ph.G->max_node_index() + 1);
    ph.nodes.colors.clear();
    ph.nodes.colors.resize(ph.G->max_node_index() + 1, R::Color::Black);
}

static void Assert(int exp) { assert(exp); }

static void CheckGraphIndices(const graph_t& G) {
#ifdef NDEBUG
#else
    std::vector<int> v;
    v.resize(G.max_node_index() + 1, 0);
    leda::node n;
    forall_nodes(n, G) v[n->id()] = 1;
    std::for_each(v.begin(), v.end(), Assert);
    v.clear();
    v.resize(G.max_edge_index() + 1, 0);
    leda::edge e;
    forall_edges(e, G) v[e->id()] = 1;
    std::for_each(v.begin(), v.end(), Assert);
#endif
}

static void Polyhedron_RebuildHull(ENT_Polyhedron& ph) {
    CheckGraphIndices(*ph.G);

	ph.hull.faceLists.clear();
    ph.hull.faceTrans.clear();
	ph.hull.edges.clear();
	ph.hull.vnmap.clear();
	ph.hull.vertices.clear();
	ph.hull.indices.clear();

	ph.hull.edges.resize(ph.G->max_edge_index() + 1);

    leda::edge_array<bool> visitedEdge(*ph.G, false);

    unsigned indexCnt = 0;

    leda::edge e;
    forall_edges(e, *ph.G) {
        if(!visitedEdge[e]) {
			leda::edge it = ph.G->face_cycle_succ(e);

            PolyhedronHullFaceList face;
            face.base = indexCnt; // index of v0
            face.size = 2; // v0 and v1

			ph.hull.faceLists.push_back(face);
			unsigned faceIndex = ph.hull.faceLists.size() - 1;
			PolyhedronHullFaceList& faceRef = ph.hull.faceLists.back();

			ph.hull.vnmap.push_back(source(e)->id());
            ph.hull.indices.push_back(indexCnt);
            indexCnt++;

			ph.hull.vnmap.push_back(source(it)->id());
            ph.hull.indices.push_back(indexCnt);
            indexCnt++;

			ph.hull.edges[e->id()].faceIdx = ph.hull.edges[it->id()].faceIdx = faceIndex;
            visitedEdge[e] = visitedEdge[it] = true;

            while((it = ph.G->face_cycle_succ(it)) != e) {
				ph.hull.vnmap.push_back(source(it)->id());
                ph.hull.indices.push_back(indexCnt++);

                visitedEdge[it] = true;
				ph.hull.edges[it->id()].faceIdx = faceIndex;
                faceRef.size += 1;
            }

			ph.hull.indices.push_back(R::Mesh::RESTART_INDEX);
        }
    } // forall_edges

#ifdef PARANOID
    forall_edges(e, *ph.G) {
        assert(visitedEdge[e]);
		assert(PolyhedronHullEdge::INVALID_FACE_INDEX != ph.hull.edges[e->id()].faceIdx);
    }
#endif

    if(!ph.hull.indices.empty() /* ie. hull exists */) {
        R::Mesh::Vertex vert;
        vert.position = M::Vector3::Zero;
        vert.color = R::Color::White;
        ph.hull.vertices.resize(ph.hull.indices.size(), vert);

        R::Mesh::Desc desc;
        desc.vertices = &ph.hull.vertices[0];
        desc.numVertices = ph.hull.vertices.size();
        desc.indices = &ph.hull.indices[0];
        desc.numIndices = ph.hull.indices.size();
        desc.primType = 0;
        ph.hull.mesh = R::meshMgr.Create(desc);
    }
}

void Polyhedron_Rebuild(ENT_Polyhedron& ph) {
    common.printf("INFO - Polyhedron_Rebuild: rebuilding polyhedron with |V| = %d, |E| = %d.\n",
        ph.G->number_of_nodes(), ph.G->number_of_edges());

    Polyhedron_RebuildNodes(ph);
    Polyhedron_RebuildHull(ph);
}

// the renderlist of a polyhedron ph can be build even though
// it's graph ph.G is no longer valid.
void Polyhedron_BuildRenderList(ENT_Polyhedron& ph) {
	ph.renderList.clear();

	R::RenderJob renderJob;
	renderJob.fx = "Lit";
    renderJob.material = R::Material::White;

    unsigned numNodes = ph.nodes.positions.size();
    for(unsigned i = 0; i < numNodes; ++i) {
        renderJob.material.diffuseColor = ph.nodes.colors[i];
		renderJob.mesh = g_nodeMesh;
		renderJob.primType = 0;
        renderJob.transform = M::Mat4::Translate(ph.nodes.positions[i]);
		ph.renderList.push_back(renderJob);
	}

    if(!ph.hull.indices.empty() /* ie. hull exists */) {
        renderJob.material = R::Material::White;
        renderJob.mesh = ph.hull.mesh;
        renderJob.primType = GL_TRIANGLE_FAN;
        renderJob.transform = M::Mat4::Identity();
        ph.renderList.push_back(renderJob);
    }

    renderJob.fx = "TexDiffuse";
    renderJob.material.diffuseColor = R::Color::Black;
    renderJob.mesh = g_faceCurveDecalMesh;
    renderJob.primType = 0;
    renderJob.skin = g_faceCurveDecalSkin;
    for(unsigned i = 0; i < ph.hull.curves.size(); ++i) {
        for(unsigned j = 0; j < ph.hull.curves[i].curve.decalPos.size(); ++j) {
            renderJob.transform = 
                M::Mat4::Translate(ph.hull.curves[i].curve.decalPos[j]) * 
                M::Mat4::FromRigidTransform(ph.hull.curves[i].localToWorld, M::Vector3::Zero);
            ph.renderList.push_back(renderJob);
        }
    }
}

void Polyhedron_Update(ENT_Polyhedron& ph) {
    // update node positions
	leda::node n;
	forall_nodes(n, *ph.G) {
		const M::Vector3 pos = ToVector((*ph.G)[n]);
		ph.nodes.positions[n->id()] = pos;
	}

    // update vertex positions (hull)
	for(unsigned i = 0; i < ph.hull.vnmap.size() /* ie. number of vertices */; ++i)
        ph.hull.vertices[i].position = ph.nodes.positions[ph.hull.vnmap[i]];

    // update face normals (hull)
	M::Vector3 p[3];
	for(unsigned i = 0; i < ph.hull.faceLists.size(); ++i) {
		for(unsigned j = 0; j < 3; ++j)
			p[j] = ph.hull.vertices[ph.hull.faceLists[i].base + j].position;
		const M::Vector3 normal = M::Normalize(M::Cross(p[1] - p[0], p[2] - p[0]));
		for(unsigned j = 0; j < ph.hull.faceLists[i].size; ++j)
			ph.hull.vertices[ph.hull.faceLists[i].base + j].normal = normal;
	}

    if(!ph.hull.indices.empty()) ph.hull.mesh->Invalidate(&ph.hull.vertices[0]);
}

static inline M::Vector2 ProjXY(const M::Vector3& v) {
    return M::Vector2(v.x, v.y);
}

static void Polyhedron_ComputeCurveDecals(PolyhedronFaceCurve& cv) {
    // avoid overlapping decals
    float w = cvar_faceCurveDecalSize + cvar_faceCurveSpacing;
    int n = (int)(cv.curve.length / w);
    float def = cv.curve.length - n * w;
    w += def / n;

    std::vector<M::Vector2> decalPos2;
    cv.curve.decalPos.clear();
    R::SampleEquidistantPoints(cv.curve, cv.time, w, decalPos2);
    for(unsigned i = 0; i < decalPos2.size(); ++i) {
        M::Vector3 p = M::Transform(cv.localToWorld, M::Vector3(decalPos2[i].x, decalPos2[i].y, 0.0f)) + cv.origin;
        const float eps = 0.001f; // resolves z-fighting of faces and hull
        p += eps * cv.normal;
        cv.curve.decalPos.push_back(p);
    }
}

static void Polyhedron_RebuildCurve(const ENT_Polyhedron& ph, PolyhedronFaceCurve& cv) {
    const PolyhedronHullFaceList& face = ph.hull.faceLists[cv.faceIdx];

    // create local copy of shrinked points
    std::vector<M::Vector3> points;
    points.reserve(face.size);
    M::Vector3 center = M::Vector3::Zero;
    for(unsigned i = 0; i < face.size; ++i) {
        const M::Vector3& p = ph.nodes.positions[ph.hull.vnmap[face.base + i]];
        center += p;
        points.push_back(p);
    }
    center /= points.size();
    const float f = 0.2f;
    for(unsigned i = 0; i < points.size(); ++i)
        points[i] = points[i] - f * (points[i] - center);

    const M::Vector3& p0 = points[0];
    const M::Vector3& p1 = points[1];
    const M::Vector3& p2 = points[2];

    M::Vector3 v0 = M::Normalize(p1 - p0);
    M::Vector3 v1 = M::Normalize(p2 - p0);
    M::Vector3 v2 = M::Normalize(M::Cross(v0, v1));

    M::Matrix3 M(M::Mat3::FromColumns(v0, v1, v2));
    if(M::AlmostEqual(0.0f, M::Det(M))) common.printf("ERROR - Polyhedron_AddCurve: matrix M is not invertable.\n");
    M::Orthonormalize(M);
    M::Matrix3 invM(M::Inverse(M)); // TODO transpose   

    cv.localToWorld = M;
    cv.origin = p0;
    cv.normal = v2;

    cv.curve.points.clear();
    const float s = 0.2f;
    // compute endpoints and transform both control and endpoints to local space.
    for(unsigned i = 0; i < face.size; ++i) {
        const M::Vector3& c0 = points[i];
        const M::Vector3& c1 = points[(i + 1) % face.size];
        cv.curve.points.push_back(ProjXY(M::Transform(invM, (1.0f - s) * c0 + s * c1 - p0))); // must start at first endpoint!
        cv.curve.points.push_back(ProjXY(M::Transform(invM,  0.5f * c0 + 0.5f * c1 - p0)));
        cv.curve.points.push_back(ProjXY(M::Transform(invM,  s * c0 + (1.0f - s) * c1 - p0)));
        cv.curve.points.push_back(ProjXY(M::Transform(invM,  c1 - p0)));
    }
    cv.curve.points.push_back(cv.curve.points.front()); // close poly

    R::ComputeTSamples(cv.curve);
    Polyhedron_ComputeCurveDecals(cv);
}

void Polyhedron_AddCurve(ENT_Polyhedron& ph, leda::edge edge) {
    PolyhedronFaceCurve cv;
    cv.faceIdx = ph.hull.edges[edge->id()].faceIdx;
    cv.time = 0.0f;
    Polyhedron_RebuildCurve(ph, cv);
    ph.hull.curves.push_back(cv);
}


void Polyhedron_UpdateCurve(PolyhedronFaceCurve& cv, float secsPassed) {
    cv.time += cvar_faceCurveSpeed * secsPassed;
    Polyhedron_ComputeCurveDecals(cv);
}

} // namespace W