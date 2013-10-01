#include <assert.h>

#include <functional>
#include <algorithm>

#include <common\common.h>
#include <common\config\config.h>
#include <math\matrix3.h>
#include <math\matrix4.h>
#include <renderer\glew\glew.h>
#include <renderer\glcall.h>
#include <renderer\effects\effect.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\effects\pass.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\material\material.h>
#include <renderer\metrics\metrics.h>
#include "renderer.h"
#include <world\world.h>

namespace {

enum {
    IN_POSITION     = 0,
    IN_NORMAL       = 1,
    IN_COLOR        = 2,
    IN_TEXCOORDS    = 3,

    // encodes 3x4 matrix
    IN_A0           = 4,
    IN_A1           = 5,
    IN_A2           = 6,
    IN_A3           = 7,

    IN_HALF_HEIGHT_SQ  = 8
};

void BindVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_POSITION,
        3, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)offsetof(R::Mesh::Vertex, position)));
    GL_CALL(glEnableVertexAttribArray(IN_POSITION));

    GL_CALL(glVertexAttribPointer(IN_NORMAL,
        3, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)offsetof(R::Mesh::Vertex, normal)));
    GL_CALL(glEnableVertexAttribArray(IN_NORMAL));

    GL_CALL(glVertexAttribPointer(IN_COLOR,
        4, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)offsetof(R::Mesh::Vertex, color)));
    GL_CALL(glEnableVertexAttribArray(IN_COLOR));

    GL_CALL(glVertexAttribPointer(IN_TEXCOORDS,
        2, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)offsetof(R::Mesh::Vertex, texCoords)));
    GL_CALL(glEnableVertexAttribArray(IN_TEXCOORDS));
}

template<typename T> struct ToGLEnum { };
template<> struct ToGLEnum<unsigned>    { enum { ENUM = GL_UNSIGNED_INT }; };
template<> struct ToGLEnum<int>         { enum { ENUM = GL_INT }; };

} // unnamed namespace

COM::Config::Variable<int>		cvar_r_nodeType("r_nodeType", 0);
COM::Config::Variable<float>	cvar_r_nodeSize("r_nodeSize", 1.0f);
COM::Config::Variable<int>		cvar_r_nodeSubdiv("r_nodeSubdiv", 3);
COM::Config::Variable<int>		cvar_r_nodeSmooth("r_nodeSmooth", 1);

namespace R {

RenderList g_renderLists[2];
SYS::Semaphore g_rendererSem(0);
static int rlIdx = 0;

static meshPtr_t nodeMesh;

static State curState;

struct BillboardHotVertex {
    M::Vector3 position;
};

struct BillboardHot {
    BillboardHotVertex verts[4];
};

struct BillboardColdVertex {
    M::Vector2 texCoords; // used for clipping
};

struct BillboardCold {
    BillboardColdVertex verts[4];
};

static std::vector<M::Vector3>      billboardPositions;
static std::vector<Mesh::Index>     billboardIndices;
static std::vector<BillboardHot>    billboardsHot;
static std::vector<BillboardCold>   billboardsCold;
static GEN::Pointer<StaticBuffer>   billboardHotVertexBuffer;
static GEN::Pointer<StaticBuffer>   billboardColdVertexBuffer;
static GEN::Pointer<StaticBuffer>   billboardIndexBuffer;

static float RandFloat(float min, float max) {
    return min + (rand() % 1000 / 1000.0f) * (max - min);
}

static void ReserveBillboards(unsigned numBillboards) {
    if(numBillboards <= billboardsHot.size()) return;

    unsigned numBillboardIndices = 5 * numBillboards - 1;

    billboardsHot.clear();
    billboardsHot.resize(numBillboards);
    if(billboardHotVertexBuffer.IsValid()) billboardHotVertexBuffer->Destroy();
    billboardHotVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, NULL, sizeof(BillboardHot) * numBillboards));

    static const BillboardColdVertex coldVertices[4] = {
        { M::Vector2(-1.0f, -1.0f) },
        { M::Vector2(-1.0f,  1.0f) },
        { M::Vector2( 1.0f,  1.0f) },
        { M::Vector2( 1.0f, -1.0f) }
    };
    billboardsCold.clear();
    billboardsCold.resize(numBillboards);
    for(unsigned i = 0; i < numBillboards; ++i) {
        memcpy(billboardsCold[i].verts, coldVertices, sizeof(coldVertices));
    }
    if(billboardColdVertexBuffer.IsValid()) billboardColdVertexBuffer->Destroy();
    billboardColdVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, &billboardsCold[0], sizeof(BillboardCold) * numBillboards));

    billboardIndices.clear();
    billboardIndices.reserve(numBillboardIndices);
    for(unsigned i = 0; i < 4 * numBillboards; ++i) {
        if(0 < i && 0 == i % 4) billboardIndices.push_back(Mesh::RESTART_INDEX);
        billboardIndices.push_back(i);
    }
    assert(numBillboardIndices == billboardIndices.size());
    if(billboardIndexBuffer.IsValid()) billboardIndexBuffer->Destroy();
    billboardIndexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, &billboardIndices[0], sizeof(Mesh::Index) * numBillboardIndices));
}

static void InitBillboards(const std::vector<M::Vector3>& positions) {
    unsigned numBillboards = positions.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;
    if(numBillboards) {
        billboardPositions = positions;
        ReserveBillboards(numBillboards);
    }
}

// camera position at 0
static M::Matrix4 Billboard_FaceCamera(const M::Matrix4& worldMat, const M::Vector3& billboardPos) {
    const M::Vector3 p = M::Transform(worldMat, billboardPos);
    const M::Vector3 v2 = M::Normalize(-M::Vector3(p.x, 0.0f, p.z));
    const M::Vector3 v1 = M::Vector3(0.0f, 1.0f, 0.0f);
    const M::Vector3 v0 = M::Normalize(M::Cross(v1, v2));
    const M::Matrix3 M = M::Mat3::FromColumns(v0, v1, v2);
    const M::Vector3 w2 = M::Normalize(-M::Vector3(0.0f, p.y, p.z));
    const M::Vector3 w0 = M::Vector3(1.0f, 0.0f, 0.0f);
    const M::Vector3 w1 = M::Normalize(M::Cross(w0, w2));
    const M::Matrix3 N = M::Mat3::FromColumns(w0, w1, w2);
    return M::Mat4::FromRigidTransform(M * N, p);
}

// omits scale
static M::Matrix4 Billboard_FaceViewingPlane(const M::Matrix4& worldMat, const M::Vector3& billboardPos) {
    const M::Vector3 p = M::Transform(worldMat, billboardPos);
    return M::Matrix4(
        1.0f, 0.0f, 0.0f, worldMat.m03 + p.x,
        0.0f, 1.0f, 0.0f, worldMat.m13 + p.y,
        0.0f, 0.0f, 1.0f, worldMat.m23 + p.z,
        0.0f, 0.0f, 0.0f, 1.0f);
}

static void BuildBillboards(const M::Matrix4& worldMat) {
	float nodeSize = cvar_r_nodeSize;
    const BillboardHotVertex hotVertices[4] = {
        { M::Vector3(-nodeSize, -nodeSize, 0.0f) },
        { M::Vector3(-nodeSize,  nodeSize, 0.0f) },
        { M::Vector3( nodeSize,  nodeSize, 0.0f) },
        { M::Vector3( nodeSize, -nodeSize, 0.0f) }
    };
    unsigned numBillboards = billboardPositions.size();
    for(unsigned i = 0; i < numBillboards; ++i) {
        for(unsigned k = 0; k < 4; ++k) {
            billboardsHot[i].verts[k].position = M::Transform(worldMat, billboardPositions[i]) + hotVertices[k].position;
        }
    }
}

static void BindHotBillboardVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_POSITION,
        3, GL_FLOAT, GL_FALSE, sizeof(BillboardHotVertex),
        (void*)offsetof(BillboardHotVertex, position)));
    GL_CALL(glEnableVertexAttribArray(IN_POSITION));
}

static void BindColdBillboardVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_TEXCOORDS,
        2, GL_FLOAT, GL_FALSE, sizeof(BillboardColdVertex),
        (void*)offsetof(BillboardColdVertex, texCoords)));
    GL_CALL(glEnableVertexAttribArray(IN_TEXCOORDS));
}

void SetState(const State& state);

static void DrawBillboards(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
    unsigned numBillboards = billboardPositions.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;

    GEN::Pointer<Effect> fx = effectMgr.GetEffect("NodeBillboard");
    fx->Compile();
    Pass* pass = fx->GetPass(0);
    pass->Use();

    SetState(pass->GetDesc().state);

    Program& prog = pass->GetProgram();
    prog.SetUniform("uProjection", projectionMat);
    prog.SetUniform("uTransform", M::Mat4::Identity());

    prog.SetUniform("uMatDiffuseColor0", Color::White);
    prog.SetUniform("uMatDiffuseColor1", Color::Blue);
    prog.SetUniform("uMatDiffuseColor2", Color::Red);
    prog.SetUniform("uLightPos0", M::Vector3(20.0f, 20.0f, 20.0f));
    prog.SetUniform("uLightPos1", M::Vector3(-20.0f, 20.0f, 20.0f));
    prog.SetUniform("uLightPos2", M::Vector3(20.0f, -20.0f, 20.0f));
    prog.SetUniform("uLightConstantAttenuation", 1.0f);
    prog.SetUniform("uLightLinearAttenuation", 0.0f);
    prog.SetUniform("uLightQuadricAttenuation", 0.0f);

    billboardHotVertexBuffer->Bind();
    BindHotBillboardVertices();

    billboardColdVertexBuffer->Bind();
    BindColdBillboardVertices();

    billboardIndexBuffer->Bind();

    glDrawElements(GL_TRIANGLE_FAN, numBillboardIndices, GL_UNSIGNED_INT, NULL);
}

// rotates the coordinate space such that the new z axis coincides with the vector d.
// example. AlignZ(d) * d = Length(d) * (0, 0, 1)
static M::Matrix4 AlignZ(const M::Vector3& d) {
    const float len_yz_sq = d.y * d.y + d.z * d.z;
    const float len = d.Length();
    assert(0.0f < len);
    if(0.0f == len_yz_sq) { // case d, x collinear => rot_x = 0
        return M::Matrix4(
            0.0f, 0.0f, -d.x / len, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            d.x / len, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    } else {
        const float len_yz = sqrt(len_yz_sq);
        return M::Matrix4(
            len_yz * len_yz, -d.x * d.y, -d.x * d.z, 0.0f,
            0.0f, len * d.z, -len * d.y, 0.0f,
            len_yz * d.x, len_yz * d.y, len_yz * d.z, 0.0f,
            0.0f, 0.0f, 0.0f, len_yz * len
        ) / (len_yz * len);
    }
}

struct EdgeBBoxVertex {
    M::Vector3  position;
    M::Vector3  A[4];
    float       halfHeightSq;
};

static std::vector<EdgeBBoxVertex>  edgeBBoxVertices;
static std::vector<Mesh::Index>     edgeBBoxIndices;
static GEN::Pointer<StaticBuffer>   edgeBBoxVertexBuffer;
static GEN::Pointer<StaticBuffer>   edgeBBoxIndexBuffer;
static M::Matrix4 g_worldToObject;

static void BindEdgeBBoxVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_POSITION,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)offsetof(EdgeBBoxVertex, position)));
    GL_CALL(glEnableVertexAttribArray(IN_POSITION));

    GL_CALL(glVertexAttribPointer(IN_A0,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 0)));
    GL_CALL(glEnableVertexAttribArray(IN_A0));

    GL_CALL(glVertexAttribPointer(IN_A1,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 1)));
    GL_CALL(glEnableVertexAttribArray(IN_A1));

    GL_CALL(glVertexAttribPointer(IN_A2,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 2)));
    GL_CALL(glEnableVertexAttribArray(IN_A2));

    GL_CALL(glVertexAttribPointer(IN_A3,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 3)));
    GL_CALL(glEnableVertexAttribArray(IN_A3));

    GL_CALL(glVertexAttribPointer(IN_HALF_HEIGHT_SQ,
        1, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, halfHeightSq))));
    GL_CALL(glEnableVertexAttribArray(IN_HALF_HEIGHT_SQ));
}

static void CreateEdges(const std::vector<Edge>& edges) {
    if(edges.empty()) return;

    edgeBBoxVertices.clear();
    edgeBBoxIndices.clear();
    unsigned baseIdx = 0;
    for(unsigned i = 0; i < edges.size(); ++i) {
        const Edge& edge = edges[i];
        const M::Vector3 center = 0.5f * (edge.p1 + edge.p0);
        M::Matrix4 R = AlignZ(edge.p1 - edge.p0);
        M::Matrix4 worldToObject = R * M::Mat4::Translate(-center);
        M::Matrix4 objectToWorld = M::Mat4::Translate(center) * M::Transpose(R);
        g_worldToObject = worldToObject;

        const float h = 0.5f * M::Length(edge.p1 - edge.p0);
        const float r = 1.0f;
        M::Vector3 bboxVertexPositions[] = {
            M::Vector3( r,  r, -h),
            M::Vector3(-r,  r, -h),
            M::Vector3( r,  r,  h),
            M::Vector3(-r,  r,  h),
            M::Vector3( r, -r, -h),
            M::Vector3(-r, -r, -h),
            M::Vector3(-r, -r,  h),
            M::Vector3( r, -r,  h)
        };
        const unsigned numVertices = 8;
        Mesh::Index bboxIndices[] = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
        const unsigned numIndices = 14;
        EdgeBBoxVertex vertex;
        vertex.A[0] = M::Vector3(worldToObject.m00, worldToObject.m10, worldToObject.m20);
        vertex.A[1] = M::Vector3(worldToObject.m01, worldToObject.m11, worldToObject.m21);
        vertex.A[2] = M::Vector3(worldToObject.m02, worldToObject.m12, worldToObject.m22);
        vertex.A[3] = M::Vector3(worldToObject.m03, worldToObject.m13, worldToObject.m23);
        vertex.halfHeightSq = h * h;
        for(unsigned i = 0; i < numVertices; ++i) {
            vertex.position = M::Transform(objectToWorld, bboxVertexPositions[i]);
            edgeBBoxVertices.push_back(vertex);
        }
        for(unsigned i = 0; i < numIndices; ++i) edgeBBoxIndices.push_back(baseIdx + bboxIndices[i]);
        edgeBBoxIndices.push_back(Mesh::RESTART_INDEX);
        baseIdx += numVertices;
    } // for all edges

    if(!edgeBBoxVertexBuffer.IsValid()) edgeBBoxVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, &edgeBBoxVertices[0], sizeof(EdgeBBoxVertex) * edgeBBoxVertices.size()));
    if(!edgeBBoxIndexBuffer.IsValid()) edgeBBoxIndexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, &edgeBBoxIndices[0], sizeof(Mesh::Index) * edgeBBoxIndices.size()));
}

static void DrawEdges(const M::Matrix4& projectionMat, const M::Matrix4& worldMat) {
    GEN::Pointer<Effect> fx = effectMgr.GetEffect("EdgeBillboard");
    // GEN::Pointer<Effect> fx = effectMgr.GetEffect("GenericWireframe");
    fx->Compile();
    Pass* pass = fx->GetPass(0);
    pass->Use();
    pass->GetProgram().SetUniform("uProjection", projectionMat);
    pass->GetProgram().SetUniform("uTransform", worldMat);

    M::Matrix4 invWorldMat;
    assert(M::TryInvert(worldMat, invWorldMat));
    pass->GetProgram().SetUniform("uInvTransform", invWorldMat);

    SetState(pass->GetDesc().state);

    edgeBBoxVertexBuffer->Bind();
    BindEdgeBBoxVertices();

    edgeBBoxIndexBuffer->Bind();

    glDrawElements(GL_TRIANGLE_STRIP, edgeBBoxIndices.size(), GL_UNSIGNED_INT, NULL);
}

void InitDebugOutput(void);

void SetEnabled(GLenum state, GLboolean enabled) {
    if(enabled) GL_CALL(glEnable(state));
    else        GL_CALL(glDisable(state));
}

void SetState(const State& state) {
    if(curState.blend.enabled != state.blend.enabled) {
        SetEnabled(GL_BLEND, state.blend.enabled);
        curState.blend.enabled = state.blend.enabled;
    }
    if(curState.blend.srcFactor != state.blend.srcFactor || curState.blend.dstFactor != state.blend.dstFactor) {
        glBlendFunc(state.blend.srcFactor, state.blend.dstFactor);
        curState.blend.srcFactor = state.blend.srcFactor;
        curState.blend.dstFactor = state.blend.dstFactor;
    }

    // SetEnabled(GL_DEPTH_TEST, state.depth.enabled);
    if(curState.depth.maskEnabled != state.depth.maskEnabled) {
        GL_CALL(glDepthMask(state.depth.maskEnabled));
        curState.depth.maskEnabled = state.depth.maskEnabled;
    }
    if(curState.depth.func != state.depth.func) {
        GL_CALL(glDepthFunc(state.depth.func));
        curState.depth.func = state.depth.func;
    }

    if(curState.raster.lineWidth != state.raster.lineWidth) {
        GL_CALL(glLineWidth(state.raster.lineWidth));
        curState.raster.lineWidth = state.raster.lineWidth;
    }
}

void SetLightUniforms(Program& prog, const Light& light) {
    prog.SetUniform("uLightPosition", light.position);
    prog.SetUniform("uLightConstantAttenuation", light.constantAttenuation);
    prog.SetUniform("uLightLinearAttenuation", light.linearAttenuation);
    prog.SetUniform("uLightQuadricAttenuation", light.quadricAttenuation);
    prog.SetUniform("uLightDiffuseColor", light.diffuseColor);
}

void SetMaterialUniforms(Program& prog, const Material& mat) {
    prog.SetUniform("uMatDiffuseColor", mat.diffuseColor);
}

static void CreateNodeMesh(void) {
	Sphere sphere(cvar_r_nodeSubdiv, cvar_r_nodeSmooth);
	sphere.Scale(cvar_r_nodeSize);
	nodeMesh = meshMgr.Create(sphere.GetDesc());
}

static void CreateNodeMeshCallback(const std::string& name) {
	if("r_nodeSize" == name || "r_nodeSubdiv" == name || "r_nodeSmooth" == name) 
        CreateNodeMesh();
}

Renderer::Renderer(void) : _time(0.0f) { }

void Renderer::Init(void) {
    GL_CHECK_ERROR;

    InitDebugOutput();

    /*
    uncomment this when using QGLWidget
    GLenum ret;
    if(GLEW_OK != (ret = glewInit())) {
        common.printf("ERROR - glewInit() failed with code %d, \"%s\"\n",
                ret, glewGetErrorString(ret));
        Crash();
    }
    common.printf("INFO - using glew version %s.\n", glewGetString(GLEW_VERSION));

    const GLubyte* glVersion = glGetString(GL_VERSION);
    common.printf("INFO - supported GL version: '%s'.\n", glVersion);
    */

    curState.SetDefault();

    GL_CALL(glEnable(GL_PRIMITIVE_RESTART));
    GL_CALL(glPrimitiveRestartIndex(Mesh::RESTART_INDEX));

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

	cvar_r_nodeSize.Register(CreateNodeMeshCallback);
	cvar_r_nodeSubdiv.Register(CreateNodeMeshCallback);
    cvar_r_nodeSmooth.Register(CreateNodeMeshCallback);

	CreateNodeMesh();

    _timer.Start();
}

void Renderer::Resize(int width, int height) {
    glViewport(0, 0, width, height);
    _aspect = (float)width / height;
}

static RenderJob* DrawMeshList(Program& prog, int passType, int passFlags, const M::Matrix4& worldMat, RenderJob* first) {
    RenderJob* meshJob = first;
    while(meshJob && meshJob->fx == first->fx) {
        if(FIRST_LIGHT_PASS == passType || LIGHT_PASS == passType || USE_MATERIAL & passFlags)
            SetMaterialUniforms(prog, meshJob->material);

        prog.SetUniform("uTransform", meshJob->transform);

        if(USE_TEX_DIFFUSE & passFlags) {
            skinMgr.R_Bind(prog, meshJob->skin);
        }

        meshJob->mesh->R_Bind();
        BindVertices();
        unsigned numIndices = meshJob->mesh->NumIndices();

        GLenum primType = meshJob->primType;
        if(!primType) primType = meshJob->mesh->PrimitiveType();

        metrics.frame.numDrawCalls++;
        GL_CALL(glDrawElements(primType, numIndices, ToGLEnum<Mesh::Index>::ENUM, NULL));
        meshJob = meshJob->next;
    }
    return meshJob;
}

static void DrawFrame(RenderList& renderList, const M::Matrix4& projectionMat, float time) {
    typedef std::vector<RenderJob>::iterator rjobIt_t;

    if(renderList.jobs.empty()) return;

    RenderJob* cur = &renderList.jobs[0];
    RenderJob* next = NULL;
    while(cur) {
        next = NULL;
        if(true) {
            GEN::Pointer<Effect> fx = effectMgr.GetEffect(cur->fx);

            int numPasses = fx->NumPasses();
            for(int i = 0; i < numPasses; ++i) {
                Pass* pass = fx->GetPass(i);
                pass->Use();
                
                Program&        prog = pass->GetProgram();
                const PassDesc& desc = pass->GetDesc();

                SetState(desc.state);

                prog.SetUniform("uProjection", projectionMat);
                // prog.SetUniform("uTransform", worldMat);

                if(USE_TIME & desc.flags) prog.SetUniform("uTime", time);
                if(USE_COLOR & desc.flags) prog.SetUniform("uColor", desc.state.color);

                if(FIRST_LIGHT_PASS == desc.type || LIGHT_PASS == desc.type) {
                    M::Matrix3 normalMat = M::Transpose(M::Inverse(M::RotationOf(renderList.worldMat)));
                    prog.SetUniform("uNormalMat", normalMat);
                }

                if(FIRST_LIGHT_PASS == desc.type || DEFAULT == desc.type) {
                    if(FIRST_LIGHT_PASS == desc.type && !renderList.lights.empty())
                        SetLightUniforms(prog, renderList.lights[0]);

                    next = DrawMeshList(prog, desc.type, desc.flags, renderList.worldMat, cur);
                }

                if(LIGHT_PASS == desc.type) {
                    for(int j = 1; j < renderList.lights.size(); ++j) {
                        SetLightUniforms(prog, renderList.lights[j]);
                        next = DrawMeshList(prog, desc.type, desc.flags, renderList.worldMat, cur);
                    }
                } // LIGHT_PASS == type
            }
        }
        cur = next;
    }
}

static void Link(std::vector<R::RenderJob>& renderJobs) {
    unsigned numJobs = renderJobs.size();
	if(0 < numJobs) {
		for(unsigned i = 0; i < numJobs; ++i) {
			RenderJob& rjob = renderJobs[i];
			rjob.next = &renderJobs[(i + 1) % numJobs];
		}
		renderJobs[numJobs - 1].next = NULL;
	}
}

static void Compile(R::RenderJob& rjob) {
    effectMgr.GetEffect(rjob.fx)->Compile();
    rjob.mesh->R_Compile();
    if(rjob.skin.IsValid()) skinMgr.R_Compile(rjob.skin);
}

static void Transform(const M::Matrix4& worldMat, R::RenderJob& rjob) {
    rjob.transform = worldMat * rjob.transform;
}

static void CompileAndTransform(const M::Matrix4& worldMat, R::RenderJob& rjob) {
    Compile(rjob);
    Transform(worldMat, rjob);
}

static M::Matrix4 ComputeProjectionMatrix(float aspect, const M::Matrix4& worldMat, const std::vector<R::RenderJob>& renderJobs) {
    /*
    float zMin = 10000.0f, zMax = -10000.0f;
    unsigned numJobs = renderJobs.size();
    for(unsigned i = 0; i < numJobs; ++i) {
        const RenderJob& rjob = renderJobs[i];
        M::Vector3 v = M::Transform(rjob.transform, M::Vector3::Zero);
        float r = rjob.mesh->Radius();
        if(0 > v.z - r) zMin = M::Min(zMin, v.z - r);
        if(0 > v.z + r) zMax = M::Max(zMax, v.z + r);
    }
    return M::Mat4::Perspective(45.0f, aspect, -zMax, -zMin);
    */
    return M::Mat4::Perspective(45.0f, aspect, 0.1f, 1000.0f);
}

void Renderer::Render(void) {
    float secsPassed = _timer.Stop();
    _time += secsPassed;
    _timer.Start();

    if(curState.depth.maskEnabled != GL_TRUE) {
        GL_CALL(glDepthMask(GL_TRUE));
        curState.depth.maskEnabled = GL_TRUE;
    }
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    RenderList& renderList = g_renderLists[rlIdx];

	if(R_NODETYPE_GEOMETRY == cvar_r_nodeType) {
		RenderJob rjob;
		rjob.fx = "Lit";
		rjob.material.diffuseColor = Color::White;
		rjob.mesh = nodeMesh;
		rjob.primType = 0;
		for(unsigned i = 0; i < renderList.nodePositions.size(); ++i) {
			rjob.transform = M::Mat4::Translate(renderList.nodePositions[i]);
			renderList.jobs.push_back(rjob);
		}
	}

    if(renderList.jobs.empty() && renderList.nodePositions.empty()) {
    }
    else {

    static SYS::Timer frameTime;
    frameTime.Start();

    // TODO: sort

    Link(renderList.jobs);
    std::for_each(renderList.jobs.begin(), renderList.jobs.end(),
        std::bind(CompileAndTransform, renderList.worldMat, std::placeholders::_1));
    M::Matrix4 projectionMat = ComputeProjectionMatrix(_aspect, renderList.worldMat, renderList.jobs);

    metrics.frame.numDrawCalls = 0;
    DrawFrame(renderList, projectionMat, _time);

	if(R_NODETYPE_BILLBOARD == cvar_r_nodeType) {
		InitBillboards(renderList.nodePositions);
		BuildBillboards(renderList.worldMat);
		unsigned numBillboards = billboardPositions.size();
		if(numBillboards) {
			billboardHotVertexBuffer->Update_Mapped(0, sizeof(BillboardHot) * numBillboards, &billboardsHot[0]);
			DrawBillboards(renderList.worldMat, projectionMat);
			billboardHotVertexBuffer->Discard();
		}
	} 

    if(!renderList.edges.empty()) {
        CreateEdges(renderList.edges);
        DrawEdges(projectionMat, renderList.worldMat);
    }

    meshMgr.R_Update();

    // GL_CALL(glFinish());
    metrics.frame.time = frameTime.Stop();
    }

    g_rendererSem.Signal();
    W::g_worldSem.Wait();
    rlIdx = 1 - rlIdx;
}


} // namespace R
