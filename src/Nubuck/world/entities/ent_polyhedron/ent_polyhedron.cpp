#include <assert.h>
#include <algorithm>

#include <renderer\renderer.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\quad\quad.h>
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

static R::meshPtr_t         g_faceCurveDecalMesh;
static R::SkinMgr::handle_t g_faceCurveDecalSkin;

void Polyhedron_InitResources(void) {
    g_faceCurveDecalMesh = R::meshMgr.Create(R::CreateQuadDesc(cvar_faceCurveDecalSize));
    R::SkinDesc skinDesc;
    skinDesc.diffuseTexture = "C:\\Libraries\\LEDA\\LEDA-6.4\\res\\Textures\\circle.tga";
    g_faceCurveDecalSkin = R::skinMgr.Create(skinDesc);
}

void Polyhedron_Init(ENT_Polyhedron& ph) { 
    ph.renderFlags = 0;
    ph.isPickable = false;
}

static void Polyhedron_RebuildSelection(ENT_Polyhedron& ph) {
    ph.selection.nodes.clear();
    ph.selection.nodes.resize(ph.G->max_node_index() + 1, false);
}

static void Polyhedron_RebuildNodes(ENT_Polyhedron& ph) {
    leda::node n;
    ph.nodes.valid.clear();
    ph.nodes.valid.resize(ph.G->max_node_index() + 1, 0);
    forall_nodes(n, *ph.G) ph.nodes.valid[n->id()] = 1;
    ph.nodes.positions.resize(ph.G->max_node_index() + 1);
    ph.nodes.oldPositions.resize(ph.G->max_node_index() + 1);
    ph.nodes.colors.clear();
    ph.nodes.colors.resize(ph.G->max_node_index() + 1, R::Color::Black);
}

static void Polyhedron_RebuildHull(ENT_Polyhedron& ph) {
	ph.hull.faceLists.clear();
    ph.hull.faceColors.clear();
    ph.hull.faceTrans.clear();
	ph.hull.edges.clear();
	ph.hull.vnmap.clear();
	ph.hull.vertices.clear();
	ph.hull.indices.clear();

    PolyhedronHullEdge invalidEdge = { PolyhedronHullEdge::INVALID_FACE_INDEX, 0, 0 };
	ph.hull.edges.resize(ph.G->max_edge_index() + 1, invalidEdge);

    leda::edge_array<bool> visitedEdge(*ph.G, false);

    unsigned indexCnt = 0;

    const R::Color defaultFaceColor = R::Color::White;
    PolyhedronHullFaceColor faceColor;
    faceColor.cur = defaultFaceColor;
    faceColor.t = 0.0f;
    faceColor.ip = false;

    leda::edge e;
    forall_edges(e, *ph.G) {
        ph.hull.edges[e->id()].n0 = leda::source(e)->id();
        ph.hull.edges[e->id()].n1 = leda::target(e)->id();
        if(POLYHEDRON_RENDER_HULL & ph.renderFlags && !visitedEdge[e]) {
            assert(ph.G->reversal(e));
			leda::edge it = ph.G->face_cycle_succ(e);

            PolyhedronHullFaceList face;
            face.base = indexCnt; // index of v0
            face.size = 2; // v0 and v1

			ph.hull.faceLists.push_back(face);
            ph.hull.faceColors.push_back(faceColor);
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
    if(POLYHEDRON_RENDER_HULL & ph.renderFlags) {
        forall_edges(e, *ph.G) {
            assert(visitedEdge[e]);
            assert(PolyhedronHullEdge::INVALID_FACE_INDEX != ph.hull.edges[e->id()].faceIdx);
        }
    }
#endif

    if(!ph.hull.indices.empty() /* ie. hull exists */) {
        R::Mesh::Vertex vert;
        vert.position = M::Vector3::Zero;
        vert.color = defaultFaceColor;
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

    Polyhedron_RebuildSelection(ph);
    Polyhedron_RebuildNodes(ph);
    Polyhedron_RebuildHull(ph);
}

// the renderlist of a polyhedron ph can be build even though
// it's graph ph.G is no longer valid.
void Polyhedron_BuildRenderList(ENT_Polyhedron& ph) {
    ph.renderList.jobs.clear();

	R::RenderJob renderJob;
	renderJob.fx = "LitDirectional";
    renderJob.material = R::Material::White;

    unsigned numNodes = ph.nodes.positions.size();
    ph.renderList.nodePositions.clear();
    ph.renderList.nodeColors.clear();
    if(POLYHEDRON_RENDER_NODES & ph.renderFlags) {
        for(unsigned i = 0; i < numNodes; ++i) {
            if(ph.nodes.valid[i]) {
                ph.renderList.nodePositions.push_back(ph.nodes.positions[i]);

                if(ph.selection.nodes[i]) ph.renderList.nodeColors.push_back(R::Color(1.0f, 1.0f, 0.0f, 1.0f));
                else ph.renderList.nodeColors.push_back(R::Color(0.4f, 0.4f, 0.4f, 1.0f)); 
            }
        }
    } // if(renderNodes)

    R::Edge re;
    unsigned numEdges = ph.hull.edges.size();
    ph.renderList.edges.clear();
    if(POLYHEDRON_RENDER_EDGES & ph.renderFlags) {
        for(unsigned i = 0; i < numEdges; ++i) {
            const PolyhedronHullEdge& e = ph.hull.edges[i];
            if(true || PolyhedronHullEdge::INVALID_FACE_INDEX != e.faceIdx) // ie. edge is valid
            {
                re.p0 = ph.nodes.positions[e.n0];
                re.p1 = ph.nodes.positions[e.n1];
                ph.renderList.edges.push_back(re);
            }
        }
    } // if(renderEdges)

    if(POLYHEDRON_RENDER_HULL & ph.renderFlags) {
        if(!ph.hull.indices.empty() /* ie. hull exists */) {
            renderJob.material = R::Material::White;
            renderJob.mesh = ph.hull.mesh;
            renderJob.primType = GL_TRIANGLE_FAN;
            renderJob.transform = M::Mat4::Identity();
            ph.renderList.jobs.push_back(renderJob);
        }
    } // if(renderHull)

    renderJob.fx = "TexDiffuse";
    renderJob.mesh = g_faceCurveDecalMesh;
    renderJob.primType = 0;
    renderJob.skin = g_faceCurveDecalSkin;
    for(unsigned i = 0; i < ph.hull.curves.size(); ++i) {
        for(unsigned j = 0; j < ph.hull.curves[i].curve.decalPos.size(); ++j) {
            renderJob.material.diffuseColor = ph.hull.curves[i].color;
            renderJob.transform = 
                M::Mat4::Translate(ph.hull.curves[i].curve.decalPos[j]) * 
                M::Mat4::FromRigidTransform(ph.hull.curves[i].localToWorld, M::Vector3::Zero);
            ph.renderList.jobs.push_back(renderJob);
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

void Polyhedron_AddCurve(ENT_Polyhedron& ph, leda::edge edge, const R::Color& color) {
    PolyhedronFaceCurve cv;
    cv.faceIdx = ph.hull.edges[edge->id()].faceIdx;
    cv.time = 0.0f;
    cv.color = color;
    Polyhedron_RebuildCurve(ph, cv);
    ph.hull.curves.push_back(cv);
}


void Polyhedron_UpdateCurve(PolyhedronFaceCurve& cv, float secsPassed) {
    cv.time += cvar_faceCurveSpeed * secsPassed;
    Polyhedron_ComputeCurveDecals(cv);
}

void Polyhedron_UpdateFaceColors(ENT_Polyhedron& ph, float secsPassed) {
    const float IP_DUR = 2.0f;
    for(unsigned i = 0; i < ph.hull.faceColors.size(); ++i) {
        PolyhedronHullFaceColor& fc = ph.hull.faceColors[i];
        if(fc.ip) {
            const float l = M::Min(1.0f, fc.t / IP_DUR);
            fc.cur = (1.0f - l) * fc.v0 + l * fc.v1;
            fc.t += secsPassed;
            if(IP_DUR < fc.t) {
                fc.t = 0.0f;
                fc.ip = false;
            }
        }
    }

    // update vertex colors
    for(unsigned i = 0; i < ph.hull.faceLists.size(); ++i) {
        PolyhedronHullFaceList& fl = ph.hull.faceLists[i];
        PolyhedronHullFaceColor& fc = ph.hull.faceColors[i];
        for(unsigned j = 0; j < fl.size; ++j) {
            ph.hull.vertices[fl.base + j].color = fc.cur;
        }
    }

    if(!ph.hull.indices.empty()) ph.hull.mesh->Invalidate(&ph.hull.vertices[0]);
}

void Polyhedron_SetFaceColor(ENT_Polyhedron& ph, const leda::edge e, const R::Color& color) {
    unsigned faceIdx = ph.hull.edges[e->id()].faceIdx;
    PolyhedronHullFaceColor& fc = ph.hull.faceColors[faceIdx];
    fc.v0 = fc.cur;
    fc.v1 = color;
    fc.t = 0.0f;
    fc.ip = true;
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
    std::vector<PolyhedronHullFaceList> faces;
    std::vector<float>                  dists;
    bool hit = false;
    for(unsigned i = 0; i < ph.hull.faceLists.size(); ++i) {
        PolyhedronHullFaceList& fl = ph.hull.faceLists[i];
        tri.p0 = ph.hull.vertices[fl.base].position;
        for(unsigned j = 0; j < fl.size - 1; ++j) {
            tri.p1 = ph.hull.vertices[fl.base + j + 0].position;
            tri.p2 = ph.hull.vertices[fl.base + j + 1].position;
            if(RaycastTriangle(ray, tri, &info)) {
                faces.push_back(fl);
                dists.push_back(info.distance);
                hit = true;
            }
        }
    }
    if(!hit) return false;
    unsigned minIdx = 0;
    for(unsigned i = 1; i < dists.size(); ++i)
        if(dists[minIdx] > dists[i]) minIdx = i;
    return true;
}

} // namespace W