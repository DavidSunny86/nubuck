#include <assert.h>
#include <algorithm>

#include <system\timer\timer.h>
#include <renderer\renderer.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\quad\quad.h>
#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include "ent_polyhedron.h"

COM::Config::Variable<float>    cvar_faceCurveSpeed("faceSpeed", 0.2f);
COM::Config::Variable<float>    cvar_faceCurveDecalSize("faceDecalSize", 0.2f);
COM::Config::Variable<float>    cvar_faceCurveSpacing("faceSpacing", 0.4f);
COM::Config::Variable<float>    cvar_faceCurveCurvature("faceCurvature", 0.2f);

static M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

namespace W {

static R::MeshMgr::meshPtr_t    g_faceCurveDecalMesh;
static R::SkinMgr::handle_t     g_faceCurveDecalSkin;

static void Polyhedron_ClearEdgeMasks(ENT_Polyhedron& ph) {
}

void Polyhedron_InitResources(void) {
    g_faceCurveDecalMesh = R::meshMgr.Create(R::CreateQuadDesc(cvar_faceCurveDecalSize));
    R::SkinDesc skinDesc;
    skinDesc.diffuseTexture = common.BaseDir() + "Textures\\circle.tga";
    g_faceCurveDecalSkin = R::skinMgr.Create(skinDesc);
}

void Polyhedron_Init(ENT_Polyhedron& ph) { 
    ph.flags            = 0;
    ph.renderFlags      = 0;
    ph.isPickable   	= false;

    ph.edges.baseColor  = R::Color(0.4f, 0.4f, 0.4f);
    ph.edges.radius     = 0.1f;

    ph.faces.maxFaceIdx = 0;
    ph.faces.baseColor  = R::Color::White;

    ph.mesh.mesh        = NULL;

    Polyhedron_ClearEdgeMasks(ph);
}

static void Polyhedron_RebuildSelection(ENT_Polyhedron& ph, const graph_t& G) {
    ph.selection.nodes.clear();
    ph.selection.nodes.resize(G.max_node_index() + 1, false);
}

static inline void Polyhedron_ResizeNodeMaps(ENT_Polyhedron& ph, unsigned sz) {
    ph.nodes.nodes.resize(sz);
    ph.nodes.positions.resize(sz);
    ph.nodes.oldPositions.resize(sz);
    ph.nodes.colors.resize(sz);
}

static void Polyhedron_RebuildNodes(ENT_Polyhedron& ph, const graph_t& G, const leda::node_map<bool>& cachedNodes) {
    Polyhedron_ResizeNodeMaps(ph, G.max_node_index() + 1);
    
    for(unsigned i = 0; i < ph.nodes.nodes.size(); ++i) ph.nodes.nodes[i].flags &= ~POLYHEDRON_VALID_FLAG;

    leda::node n;
    forall_nodes(n, G) {
        if(0 == cachedNodes[n]) { // reinitialize
            ph.nodes.colors[n->id()] = R::Color::Black;
        }
        ph.nodes.nodes[n->id()].flags |= POLYHEDRON_VALID_FLAG;
    }
}

static inline void Polyhedron_ResizeEdgeMaps(ENT_Polyhedron& ph, unsigned sz) {
    ph.edges.edges.resize(sz);
    ph.edges.colors.resize(sz);
}

static void Polyhedron_RebuildEdges(ENT_Polyhedron& ph, const graph_t& G, const leda::edge_map<bool>& cachedEdges) {
    Polyhedron_ResizeEdgeMaps(ph, G.max_edge_index() + 1);

    for(unsigned i = 0; i < ph.edges.edges.size(); ++i) ph.edges.edges[i].flags &= ~POLYHEDRON_VALID_FLAG;

    leda::edge e;
    forall_edges(e, G) {
        PolyhedronEdge& edge = ph.edges.edges[e->id()];
        if(0 == cachedEdges[e]) { // reinitialize
            edge.flags &= ~POLYHEDRON_HIDE_FLAG;
            edge.faceIdx = POLYHEDRON_INVALID_INDEX;

            PolyhedronEdgeColor edgeColor;
            edgeColor.cur = R::Color::White;
            edgeColor.t = 0.0f;
            edgeColor.ip = false;
            ph.edges.colors[e->id()] = edgeColor;
        }
        edge.flags |= POLYHEDRON_VALID_FLAG; 
        edge.n0 = leda::source(e)->id();
        edge.n1 = leda::target(e)->id();
        assert(G.reversal(e));
        edge.rev = G.reversal(e)->id();
    }
}

static void Polyhedron_ResizeFaceMaps(ENT_Polyhedron& ph, unsigned sz) {
    ph.faces.faces.resize(sz);
    ph.faces.lists.resize(sz);
    ph.faces.colors.resize(sz);
    ph.faces.trans.resize(sz);
}

static void Polyhedron_AllocFaceIndex(ENT_Polyhedron& ph, unsigned edgeIdx) {
    PolyhedronEdge& edge = ph.edges.edges[edgeIdx];
    if(POLYHEDRON_INVALID_INDEX == edge.faceIdx) {
        if(!ph.faces.freeIndices.empty()) {
            edge.faceIdx = ph.faces.freeIndices.back();
            ph.faces.freeIndices.pop_back();
        } else {
            edge.faceIdx = ph.faces.maxFaceIdx;
            Polyhedron_ResizeFaceMaps(ph, ++ph.faces.maxFaceIdx);
        }

        // initialize new face
        ph.faces.faces[edge.faceIdx].flags = 0;
        const R::Color defaultFaceColor = R::Color::White;
        PolyhedronFaceColor faceColor;
        faceColor.cur = defaultFaceColor;
        faceColor.t = 0.0f;
        faceColor.ip = false;
        ph.faces.colors[edge.faceIdx] = faceColor;
    }
}

static void Polyhedron_RebuildIndices(ENT_Polyhedron& ph) {
    ph.mesh.indices.clear();
    for(unsigned i = 0; i < ph.faces.faces.size(); ++i) {
        if(0 == (POLYHEDRON_HIDE_FLAG & ph.faces.faces[i].flags)) {
            const PolyhedronFaceList& list = ph.faces.lists[i];
            for(unsigned j = 0; j < list.size; ++j)
                ph.mesh.indices.push_back(list.base + j);
            ph.mesh.indices.push_back(R::Mesh::RESTART_INDEX);
        }
    }
}

static void Polyhedron_RebuildHull(ENT_Polyhedron& ph, const graph_t& G) {
	ph.mesh.vnmap.clear();
	ph.mesh.vertices.clear();

    leda::edge_array<bool> visitedEdge(G, false);

    unsigned indexCnt = 0;

    leda::edge e;
    forall_edges(e, G) {
        if(POLYHEDRON_RENDER_HULL & ph.renderFlags && !visitedEdge[e] && 0 == (ph.edges.edges[e->id()].flags & POLYHEDRON_HIDE_FLAG)) {
			leda::edge it = G.face_cycle_succ(e);

            Polyhedron_AllocFaceIndex(ph, e->id());
            unsigned faceIdx = ph.edges.edges[e->id()].faceIdx;
            assert(POLYHEDRON_INVALID_INDEX != faceIdx);

            PolyhedronFaceList& faceList = ph.faces.lists[faceIdx];
            faceList.base = indexCnt; // index of v0
            faceList.size = 2; // v0 and v1

			ph.mesh.vnmap.push_back(source(e)->id());
            indexCnt++;

			ph.mesh.vnmap.push_back(source(it)->id());
            indexCnt++;

			ph.edges.edges[e->id()].faceIdx = ph.edges.edges[it->id()].faceIdx = faceIdx;
            visitedEdge[e] = visitedEdge[it] = true;

            while((it = G.face_cycle_succ(it)) != e) {
				ph.mesh.vnmap.push_back(source(it)->id());
                indexCnt++;

                visitedEdge[it] = true;
				ph.edges.edges[it->id()].faceIdx = faceIdx;
                faceList.size += 1;
            }
        }
    } // forall_edges

#ifdef PARANOID
    if(POLYHEDRON_RENDER_HULL & ph.renderFlags) {
        forall_edges(e, G) {
            if(0 == (POLYHEDRON_HIDE_FLAG & ph.edges.edges[e->id()].flags)) {
                assert(visitedEdge[e]);
                assert(POLYHEDRON_INVALID_INDEX != ph.edges.edges[e->id()].faceIdx);
            }
        }
    }
#endif

    R::Mesh::Vertex vert;
    vert.position = M::Vector3::Zero;
    vert.color = ph.faces.baseColor;
    ph.mesh.vertices.resize(ph.mesh.vnmap.size(), vert);
    for(unsigned i = 0; i < ph.mesh.vnmap.size() /* ie. number of vertices */; ++i)
        ph.mesh.vertices[i].position = ph.nodes.positions[ph.mesh.vnmap[i]];

    ph.flags |= POLYHEDRON_REBUILD_MESH;
}

void Polyhedron_Update(ENT_Polyhedron& ph) {
    if(POLYHEDRON_REBUILD_MESH & ph.flags) {
        Polyhedron_RebuildIndices(ph);

        if(ph.mesh.mesh) R::meshMgr.Destroy(ph.mesh.mesh);
        ph.mesh.mesh = NULL;

        if(!ph.mesh.indices.empty() /* ie. hull exists */) {
            R::Mesh::Desc desc;
            desc.vertices = &ph.mesh.vertices[0];
            desc.numVertices = ph.mesh.vertices.size();
            desc.indices = &ph.mesh.indices[0];
            desc.numIndices = ph.mesh.indices.size();
            desc.primType = GL_TRIANGLE_FAN;
            ph.mesh.mesh = R::meshMgr.Create(desc);
        }

        ph.flags &= ~(POLYHEDRON_REBUILD_MESH | POLYHEDRON_UPDATE_MESH_VERTICES | POLYHEDRON_UPDATE_MESH_INDICES);
    }

    if(POLYHEDRON_UPDATE_MESH_VERTICES & ph.flags) {
        R::meshMgr.GetMesh(ph.mesh.mesh).Invalidate(&ph.mesh.vertices[0]);
        ph.flags &= ~POLYHEDRON_UPDATE_MESH_VERTICES;
    }

    if(POLYHEDRON_UPDATE_MESH_INDICES & ph.flags) {
        Polyhedron_RebuildIndices(ph);
        R::meshMgr.GetMesh(ph.mesh.mesh).Invalidate(&ph.mesh.indices[0], ph.mesh.indices.size());
        ph.flags &= ~POLYHEDRON_UPDATE_MESH_INDICES;
    }
}

static bool Polyhedron_IsPlanar(const graph_t& G) {
    const leda::list<leda::node>& L = G.all_nodes();
    point_t S[] = { 
        G[L[L.get_item(0)]],
        G[L[L.get_item(1)]],
        G[L[L.get_item(2)]]
    };
    for(unsigned i = 3; i < L.size(); ++i) {
        if(leda::orientation(S[0], S[1], S[2], G[L[L.get_item(i)]]))
            return false;
    }
    return true;
}

static leda::node MinXY(const graph_t& G, const leda::list<leda::node>& L) {
    leda::node min = L.front();
    for(unsigned i = 1; i < L.size(); ++i) {
        leda::node other = L[L.get_item(i)];
        if(G[other].xcoord() < G[min].xcoord() || (G[other].xcoord() == G[min].xcoord() && G[other].ycoord() < G[min].ycoord()))
            min = other;
    }
    return min;
}

static leda::edge Polyhedron_FindOuterFace(const graph_t& G) {
    leda::list<leda::node> L = G.all_nodes();
    leda::node n0 = MinXY(G, L);
    leda::edge e0 = NULL;
    leda::edge e;
    forall_out_edges(e, n0) {
        bool outerFace = true;
        leda::node n;
        forall_nodes(n, G) {
            if(0 < G.outdeg(n) && 0 < G.indeg(n) && 0 < leda::orientation_xy(G[leda::source(e)], G[leda::target(e)], G[n]))
                outerFace = false;
        }
        if(outerFace) {
            e0 = e;
            break;
        }
    }
    assert(e0);
    return e0;
}

static void Polyhedron_MaskFace(ENT_Polyhedron& ph, leda::edge e) {
    ph.faces.faces[ph.edges.edges[e->id()].faceIdx].flags |= POLYHEDRON_HIDE_FLAG;
    ph.flags |= POLYHEDRON_UPDATE_MESH_INDICES;
}

static void Polyhedron_UnmaskFace(ENT_Polyhedron& ph, leda::edge e) {
    ph.faces.faces[ph.edges.edges[e->id()].faceIdx].flags &= ~POLYHEDRON_HIDE_FLAG;
    ph.flags |= POLYHEDRON_UPDATE_MESH_INDICES;
}

static void Polyhedron_CacheAll(const graph_t& G, leda::node_map<bool>& cachedNodes, leda::edge_map<bool>& cachedEdges) {
    leda::node n;
    forall_nodes(n, G) cachedNodes[n] = 1;
    leda::edge e;
    forall_edges(e, G) cachedEdges[e] = 1;
}

void Polyhedron_Rebuild(ENT_Polyhedron& ph, const graph_t& G, leda::node_map<bool>& cachedNodes, leda::edge_map<bool>& cachedEdges) {
    SYS::Timer timer;
    timer.Start();

    Polyhedron_RebuildSelection(ph, G);
    Polyhedron_RebuildNodes(ph, G, cachedNodes);
    Polyhedron_RebuildEdges(ph, G, cachedEdges);

    if(Polyhedron_IsPlanar(G) && 0 < G.number_of_edges() && POLYHEDRON_RENDER_HULL & ph.renderFlags)  {
        Polyhedron_MaskFace(ph, Polyhedron_FindOuterFace(G));
    }

    Polyhedron_RebuildHull(ph, G);

    Polyhedron_CacheAll(G, cachedNodes, cachedEdges);

    float secsPassed = timer.Stop();
    // common.printf("Polyhedron_Rebuild took %f seconds\n", secsPassed);
    printf("Polyhedron_Rebuild took %f seconds\n", secsPassed);

    Polyhedron_PrintSize(ph);
}

// the renderlist of a polyhedron ph can be build even though
// it's graph ph.G is no longer valid.
void Polyhedron_BuildRenderList(ENT_Polyhedron& ph, const std::string& hullFx) {
    ph.renderList.meshJobs.clear();

	R::MeshJob renderJob;
	renderJob.fx = "LitDirectional";
    renderJob.material = R::Material::White;

    unsigned numNodes = ph.nodes.nodes.size();
    std::vector<R::Nodes::Node> rnodes;
    if(POLYHEDRON_RENDER_NODES & ph.renderFlags) {
        for(unsigned i = 0; i < numNodes; ++i) {
            if(POLYHEDRON_VALID_FLAG & ph.nodes.nodes[i].flags) {
                R::Nodes::Node rnode;
                rnode.position = ph.nodes.positions[i];
                if(ph.selection.nodes[i]) rnode.color = R::Color(1.0f, 1.0f, 0.0f, 1.0f);
                else rnode.color = ph.nodes.colors[i];
                rnodes.push_back(rnode);
            }
        }
        R::g_nodes.Draw(rnodes);
    } // if(renderNodes)

    R::Edge re;
    re.radius   = ph.edges.radius;
    unsigned numEdges = ph.edges.edges.size();
    if(POLYHEDRON_RENDER_EDGES & ph.renderFlags) {
        std::vector<R::Edge> redges;
        // TODO: store reversal information to draw edges only once
        for(unsigned i = 0; i < numEdges; ++i) {
            const PolyhedronEdge& e = ph.edges.edges[i];
            if(POLYHEDRON_VALID_FLAG & e.flags)
            {
                re.color = R::BlendMulRGB(ph.edges.colors[i].cur, ph.edges.baseColor);
                re.p0 = ph.nodes.positions[e.n0];
                re.p1 = ph.nodes.positions[e.n1];
                redges.push_back(re);
            }
        }
        R::g_cylinderEdges.Draw(redges);
    } // if(renderEdges)

    if(POLYHEDRON_RENDER_HULL & ph.renderFlags) {
        if(!ph.mesh.indices.empty() /* ie. hull exists */) {
            float alpha = ph.mesh.vertices[0].color.a; // constant for all verts
            if(1.0f > alpha) renderJob.fx = "LitDirectionalTransparent";
            else renderJob.fx = "LitDirectional";
            // renderJob.fx = hullFx;

            renderJob.material = R::Material::White;
            renderJob.mesh = ph.mesh.mesh;
            renderJob.primType = 0;
            renderJob.transform = M::Mat4::Identity();
            ph.renderList.meshJobs.push_back(renderJob);
        }
    } // if(renderHull)

    renderJob.fx = "TexDiffuse";
    renderJob.mesh = g_faceCurveDecalMesh;
    renderJob.primType = 0;
    renderJob.skin = g_faceCurveDecalSkin;
    for(unsigned i = 0; i < ph.faces.curves.size(); ++i) {
        for(unsigned j = 0; j < ph.faces.curves[i].curve.decalPos.size(); ++j) {
            renderJob.material.diffuseColor = ph.faces.curves[i].color;
            renderJob.transform = 
                M::Mat4::Translate(ph.faces.curves[i].curve.decalPos[j]) * 
                M::Mat4::FromRigidTransform(ph.faces.curves[i].localToWorld, M::Vector3::Zero);
            ph.renderList.meshJobs.push_back(renderJob);
        }
    }
}

void Polyhedron_Update(ENT_Polyhedron& ph, const graph_t& G) {
    // update node positions
	leda::node n;
	forall_nodes(n, G) {
		const M::Vector3 pos = ToVector(G[n]);
		ph.nodes.positions[n->id()] = pos;
	}

    // update vertex positions (mesh)
	for(unsigned i = 0; i < ph.mesh.vnmap.size() /* ie. number of vertices */; ++i)
        ph.mesh.vertices[i].position = ph.nodes.positions[ph.mesh.vnmap[i]];

    // update face normals (mesh)
	M::Vector3 p[3];
	for(unsigned i = 0; i < ph.faces.lists.size(); ++i) {
		for(unsigned j = 0; j < 3; ++j)
			p[j] = ph.mesh.vertices[ph.faces.lists[i].base + j].position;
		const M::Vector3 normal = M::Normalize(M::Cross(p[1] - p[0], p[2] - p[0]));
		for(unsigned j = 0; j < ph.faces.lists[i].size; ++j)
			ph.mesh.vertices[ph.faces.lists[i].base + j].normal = normal;
	}

    ph.flags |= POLYHEDRON_UPDATE_MESH_VERTICES;
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
    const PolyhedronFaceList& face = ph.faces.lists[cv.faceIdx];

    // create local copy of shrinked points
    std::vector<M::Vector3> points;
    points.reserve(face.size);
    M::Vector3 center = M::Vector3::Zero;
    for(unsigned i = 0; i < face.size; ++i) {
        const M::Vector3& p = ph.nodes.positions[ph.mesh.vnmap[face.base + i]];
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

void Polyhedron_AddCurve(ENT_Polyhedron& ph, leda::edge edge, const R::Color& color) {
    PolyhedronFaceCurve cv;
    cv.faceIdx = ph.edges.edges[edge->id()].faceIdx;
    cv.time = 0.0f;
    cv.color = color;
    Polyhedron_RebuildCurve(ph, cv);
    ph.faces.curves.push_back(cv);
}


void Polyhedron_UpdateCurve(PolyhedronFaceCurve& cv, float secsPassed) {
    cv.time += cvar_faceCurveSpeed * secsPassed;
    Polyhedron_ComputeCurveDecals(cv);
}

void Polyhedron_UpdateFaceColors(ENT_Polyhedron& ph, float secsPassed) {
    const float IP_DUR = 0.5f;
    for(unsigned i = 0; i < ph.faces.colors.size(); ++i) {
        PolyhedronFaceColor& fc = ph.faces.colors[i];
        if(fc.ip) {
            const float l = M::Min(1.0f, fc.t / IP_DUR);
            fc.cur = R::Lerp(fc.v0, fc.v1, l);
            fc.t += secsPassed;
            if(IP_DUR < fc.t) {
                fc.t = 0.0f;
                fc.ip = false;
            }
        }
    }

    // update vertex colors
    for(unsigned i = 0; i < ph.faces.lists.size(); ++i) {
        PolyhedronFaceList& fl = ph.faces.lists[i];
        PolyhedronFaceColor& fc = ph.faces.colors[i];
        for(unsigned j = 0; j < fl.size; ++j) {
            ph.mesh.vertices[fl.base + j].color = R::BlendMulRGBA(fc.cur, ph.faces.baseColor);
        }
    }

    if(!ph.mesh.indices.empty()) R::meshMgr.GetMesh(ph.mesh.mesh).Invalidate(&ph.mesh.vertices[0]);
}

void Polyhedron_SetFaceColor(ENT_Polyhedron& ph, const leda::edge e, const R::Color& color) {
    if(!(POLYHEDRON_VALID_FLAG & ph.edges.edges[e->id()].flags) || POLYHEDRON_HIDE_FLAG & ph.edges.edges[e->id()].flags) return;
    unsigned faceIdx = ph.edges.edges[e->id()].faceIdx;
    PolyhedronFaceColor& fc = ph.faces.colors[faceIdx];
    fc.v0 = fc.cur;
    fc.v1 = color;
    fc.t = 0.0f;
    // fc.t = 10.0f; // instant
    fc.ip = true;
}

void Polyhedron_SetFaceVisibility(ENT_Polyhedron& ph, const leda::edge e, bool visible) {
    if(visible) Polyhedron_UnmaskFace(ph, e);
    else Polyhedron_MaskFace(ph, e);
}

void Polyhedron_SetHullAlpha(ENT_Polyhedron& ph, float alpha) {
    assert(0.0f <= alpha && alpha <= 1.0f);
    for(unsigned i = 0; i < ph.mesh.vertices.size(); ++i)
        ph.mesh.vertices[i].color.a = alpha;
    ph.faces.baseColor.a = alpha;
    ph.flags |= POLYHEDRON_UPDATE_MESH_VERTICES;
}

struct Ray {
    M::Vector3 origin;
    M::Vector3 direction;
};

struct Sphere {
    M::Vector3  center;
    float       radius;
};

struct Triangle {
    M::Vector3 p0, p1, p2;
};

struct Info {
    M::Vector3  where;
    M::Vector3  normal;
    float       distance;
};

static bool RaycastSphere(const Ray& ray, const Sphere& sphere, Info* info) {
    const float B = 2.0f *
        (ray.direction.x * (ray.origin.x - sphere.center.x) +
         ray.direction.y * (ray.origin.y - sphere.center.y) +
         ray.direction.z * (ray.origin.z - sphere.center.z));
    const float C = (ray.origin.x - sphere.center.x) * (ray.origin.x - sphere.center.x)
        + (ray.origin.y - sphere.center.y) * (ray.origin.y - sphere.center.y)
        + (ray.origin.z - sphere.center.z) * (ray.origin.z - sphere.center.z)
        - sphere.radius * sphere.radius;

    const float M = B * B - 4.0f * C;
    if(0.0f > M) return false;
    const float D = sqrt(M);

    const float t0 = 0.5f * (-B - D);
    if(t0 > 0) {
        if(info) {
            info->where = ray.origin + t0 * ray.direction;
            info->normal = M::Normalize(info->where - sphere.center);
            info->distance = t0;
        }
        return true;
    }

    const float t1 = 0.5f * (-B + D);
    if(t1 > 0) {
        if(info) {
            info->where = ray.origin + t1 * ray.direction;
            info->normal = M::Normalize(info->where - sphere.center);
            info->distance = t1;
        }
        return true;
    }

    return false;
}

static bool RaycastTriangle(const Ray& ray, const Triangle& tri, Info* info) {
    const M::Vector3 q1 = tri.p1 - tri.p0;
    const M::Vector3 q2 = tri.p2 - tri.p0;
    const M::Vector3 n = M::Cross(q1, q2);
    const float d0 = M::Dot(n, ray.direction);
    if(M::AlmostEqual(d0, 0.0f)) return false;
    const float t = (M::Dot(n, tri.p0) - M::Dot(n, ray.origin)) / d0;
    const M::Vector3 r = ray.origin + t * ray.direction - tri.p0;
    const float q1Sq = M::Dot(q1, q1);
    const float q2Sq = M::Dot(q2, q2);
    const float q12 = M::Dot(q1, q2);
    const float d1 = 1.0f / (q1Sq * q2Sq - q12 * q12);
    const float x1 = M::Dot(r, q1);
    const float x2 = M::Dot(r, q2);
    const float w1 = d1 * (q2Sq * x1 - q12 * x2);
    const float w2 = d1 * (q1Sq * x2 - q12 * x1);
    const float w0 = 1.0f - w1 - w2;
    if(info) {
        info->distance = t;
    }
    return 0.0f <= w0 && 0.0f <= w1 && 0.0f <= w2;
}

bool Polyhedron_RaycastNodes(ENT_Polyhedron& ph, const M::Vector3& rayOrig, const M::Vector3& rayDir, leda::node& hitNode) {
    if(!ph.isPickable) return false;

    Ray ray = { rayOrig, rayDir };
    Sphere sphere;
    sphere.radius = cvar_r_nodeSize;
    Info info;
    std::vector<leda::node> nodes;
    std::vector<float>      dists;
    bool hit = false;
    leda::node n;
    /*
    forall_nodes(n, *ph.G) {
        if(ph.nodes.valid[n->id()]) {
            sphere.center = ph.nodes.positions[n->id()];
            if(RaycastSphere(ray, sphere, &info)) {
                nodes.push_back(n);
                dists.push_back(info.distance);
                hit = true;
            }
        }
    }
    */
    if(!hit) return false;
    unsigned minIdx = 0;
    for(unsigned i = 1; i < dists.size(); ++i)
        if(dists[minIdx] > dists[i]) minIdx = i;
    hitNode = nodes[minIdx];
    return true;
}

bool Polyhedron_RaycastFaces(ENT_Polyhedron& ph, const M::Vector3& rayOrig, const M::Vector3& rayDir, leda::edge& hitFace) {
    if(!ph.isPickable) return false;

    Ray ray = {rayOrig, rayDir };
    Triangle tri;
    Info info;
    std::vector<PolyhedronFaceList> faces;
    std::vector<float>              dists;
    bool hit = false;
    for(unsigned i = 0; i < ph.faces.lists.size(); ++i) {
        PolyhedronFaceList& fl = ph.faces.lists[i];
        tri.p0 = ph.mesh.vertices[fl.base].position;
        unsigned j = 0;
        while(j + 2 < fl.size) {
            tri.p1 = ph.mesh.vertices[fl.base + j + 1].position;
            tri.p2 = ph.mesh.vertices[fl.base + j + 2].position;
            if(RaycastTriangle(ray, tri, &info)) {
                faces.push_back(fl);
                dists.push_back(info.distance);
                hit = true;
            }
            j += 2;
        }
    }
    if(!hit) return false;
    unsigned minIdx = 0;
    for(unsigned i = 1; i < dists.size(); ++i)
        if(dists[minIdx] > dists[i]) minIdx = i;
    leda::edge e;
    hitFace = NULL;
    /*
    forall_edges(e, *ph.G) {
        if(ph.hull.faceLists[ph.hull.edges[e->id()].faceIdx].base == faces[minIdx].base) {
            hitFace = e;
            break;
        }
    }
    */
    assert(NULL != hitFace);
    return true;
}

void Polyhedron_GetInfo(const ENT_Polyhedron& ph, INF_Polyhedron& inf) {
    inf.edgeColor   = ph.edges.baseColor;
    inf.edgeRadius  = ph.edges.radius;
}

} // namespace W