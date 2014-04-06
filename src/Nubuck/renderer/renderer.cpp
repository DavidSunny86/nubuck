#include <assert.h>

#include <functional>
#include <algorithm>

#include <Nubuck\common\common.h>
#include <common\config\config.h>
#include <Nubuck\math\matrix3.h>
#include <Nubuck\math\matrix4.h>
#include <renderer\glew\glew.h>
#include <renderer\glcall.h>
#include <renderer\effects\effect.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\effects\pass.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\cylinder\cylinder.h>
#include <renderer\material\material.h>
#include <renderer\metrics\metrics.h>
#include <renderer\giantbuffer\giant_buffer.h>
#include <renderer\framebuffer\framebuffer.h>
#include <renderer\renderbuffer\renderbuffer.h>
#include <renderer\textures\texture.h>
#include "renderer_local.h"
#include "renderer.h"
#include <world\world.h>

namespace {

void BindMeshAttributes(void) {
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

    GL_CALL(glVertexAttribPointer(IN_A0,
        3, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)(offsetof(R::Mesh::Vertex, A) + sizeof(M::Vector3) * 0)));
    GL_CALL(glEnableVertexAttribArray(IN_A0));

    GL_CALL(glVertexAttribPointer(IN_A1,
        3, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)(offsetof(R::Mesh::Vertex, A) + sizeof(M::Vector3) * 1)));
    GL_CALL(glEnableVertexAttribArray(IN_A1));

    GL_CALL(glVertexAttribPointer(IN_A2,
        3, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)(offsetof(R::Mesh::Vertex, A) + sizeof(M::Vector3) * 2)));
    GL_CALL(glEnableVertexAttribArray(IN_A2));

    GL_CALL(glVertexAttribPointer(IN_A3,
        3, GL_FLOAT, GL_FALSE, sizeof(R::Mesh::Vertex),
        (void*)(offsetof(R::Mesh::Vertex, A) + sizeof(M::Vector3) * 3)));
    GL_CALL(glEnableVertexAttribArray(IN_A3));
}

void UnbindMeshAttributes() {
    GL_CALL(glDisableVertexAttribArray(IN_POSITION));
    GL_CALL(glDisableVertexAttribArray(IN_NORMAL));
    GL_CALL(glDisableVertexAttribArray(IN_COLOR));
    GL_CALL(glDisableVertexAttribArray(IN_TEXCOORDS));
    GL_CALL(glDisableVertexAttribArray(IN_A0));
    GL_CALL(glDisableVertexAttribArray(IN_A1));
    GL_CALL(glDisableVertexAttribArray(IN_A2));
    GL_CALL(glDisableVertexAttribArray(IN_A3));
}

template<typename T> struct ToGLEnum { };
template<> struct ToGLEnum<unsigned>    { enum { ENUM = GL_UNSIGNED_INT }; };
template<> struct ToGLEnum<int>         { enum { ENUM = GL_INT }; };

} // unnamed namespace

COM::Config::Variable<float>	cvar_r_nodeSize("r_nodeSize", 0.1f);
COM::Config::Variable<float>    cvar_r_edgeRadius("r_edgeRadius", 0.1f);

namespace R {

static meshPtr_t nodeMesh;

static State curState;

enum UniformBindingIndices {
    BINDING_INDEX_HOT           = 0,
    BINDING_INDEX_LIGHTS    	= 1,
    BINDING_INDEX_SKELETON  	= 2,
    BINDING_INDEX_RENDER_TARGET = 3
};

struct UniformsHot {
    M::Matrix4 uProjection;
    M::Matrix4 uTransform;
    M::Matrix4 uInvTransform;
    M::Matrix4 uNormalMat; // use mat4 to avoid padding issues
};

struct UniformsLights {
    M::Vector3  uLightVec0;
    float       padding0;
    M::Vector3  uLightVec1;
    float       padding1;
    M::Vector3  uLightVec2;
    float       padding2;
    Color       uLightDiffuseColor0;
    Color 		uLightDiffuseColor1;
    Color 		uLightDiffuseColor2;
    float       uShininess;
};

struct UniformsSkeleton {
    Color       uColor;
    float       uNodeSize;
};

struct UniformsRenderTarget {
    int uWidth;
    int uHeight;
};

static UniformsHot                  uniformsHot;
static UniformsLights               uniformsLights;
static UniformsSkeleton             uniformsSkeleton;
static UniformsRenderTarget         uniformsRenderTarget;
static GEN::Pointer<StaticBuffer>   uniformsHotBuffer;
static GEN::Pointer<StaticBuffer>   uniformsLightsBuffer;
static GEN::Pointer<StaticBuffer>   uniformsSkeletonBuffer;
static GEN::Pointer<StaticBuffer>   uniformsRenderTargetBuffer;

static void Uniforms_InitBuffers(void) {
    uniformsHotBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsHot)));
    uniformsLightsBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsLights)));
    uniformsSkeletonBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsSkeleton)));
    uniformsRenderTargetBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsRenderTarget)));
}

void Uniforms_BindBuffers(void) {
    // somehow these bindings break on intel gpus.
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_HOT, uniformsHotBuffer->GetID()));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_LIGHTS, uniformsLightsBuffer->GetID()));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_SKELETON, uniformsSkeletonBuffer->GetID()));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_RENDER_TARGET, uniformsRenderTargetBuffer->GetID()));
}

void Uniforms_BindBlocks(const Program& prog) {
    GLuint idx = 0;
    
    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsHot");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_HOT));

    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsLights");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_LIGHTS));

    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsSkeleton");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_SKELETON));

    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsRenderTarget");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_RENDER_TARGET));
}

static void Uniforms_Update(
    const M::Matrix4& projectionMat, 
    const M::Matrix4& worldMat,
    const DirectionalLight dirLights[])
{
    uniformsHot.uProjection = projectionMat;
    uniformsHot.uTransform = worldMat;
    M::TryInvert(worldMat, uniformsHot.uInvTransform);
    uniformsHot.uNormalMat = M::Mat4::FromRigidTransform(M::Transpose(M::Inverse(M::RotationOf(worldMat))), M::Vector3::Zero);
    uniformsHotBuffer->Update_Mapped(0, sizeof(UniformsHot), &uniformsHot);

    uniformsLights.uLightVec0 = -dirLights[0].direction;
    uniformsLights.uLightVec1 = -dirLights[1].direction;
    uniformsLights.uLightVec2 = -dirLights[2].direction;
    uniformsLights.uLightDiffuseColor0 = dirLights[0].diffuseColor;
    uniformsLights.uLightDiffuseColor1 = dirLights[1].diffuseColor;
    uniformsLights.uLightDiffuseColor2 = dirLights[2].diffuseColor;
    uniformsLights.uShininess = 200.0f;
    uniformsLightsBuffer->Update_Mapped(0, sizeof(UniformsLights), &uniformsLights);

    uniformsSkeleton.uColor = Color(0.4f, 0.4f, 0.4f, 1.0f);
    uniformsSkeleton.uNodeSize = cvar_r_nodeSize;
    uniformsSkeletonBuffer->Update_Mapped(0, sizeof(UniformsSkeleton), &uniformsSkeleton);
}

static void Uniforms_UpdateModelView(const M::Matrix4& modelView) {
    uniformsHot.uTransform = modelView;
    M::TryInvert(modelView, uniformsHot.uInvTransform);
    uniformsHot.uNormalMat = M::Mat4::FromRigidTransform(M::Transpose(M::Inverse(M::RotationOf(modelView))), M::Vector3::Zero);
    uniformsHotBuffer->Update_Mapped(0, sizeof(UniformsHot), &uniformsHot);
}

static void Uniforms_UpdateRenderTargetSize(const int width, const int height) {
    uniformsRenderTarget.uWidth = width;
    uniformsRenderTarget.uHeight = height;
    uniformsRenderTargetBuffer->Update_Mapped(0, sizeof(UniformsRenderTarget), &uniformsRenderTarget);
}

static float RandFloat(float min, float max) {
    return min + (rand() % 1000 / 1000.0f) * (max - min);
}

void InitDebugOutput(void);

void SetEnabled(GLenum state, GLboolean enabled) {
    if(enabled) GL_CALL(glEnable(state));
    else        GL_CALL(glDisable(state));
}

void SetState(const State& state) {
    bool c0, c1, c2, c3;

    if(curState.culling.hw.enabled != state.culling.hw.enabled) {
        SetEnabled(GL_CULL_FACE, state.culling.hw.enabled);
        curState.culling.hw.enabled = state.culling.hw.enabled;
    }
    if(curState.culling.hw.enabled && curState.culling.hw.cullFace != state.culling.hw.cullFace) {
        GL_CALL(glCullFace(state.culling.hw.cullFace));
        curState.culling.hw.cullFace = state.culling.hw.cullFace;
    }
    curState.culling.sw = state.culling.sw;

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

    // SetEnabled(GL_STENCIL_TEST, state.stencil.enabled);
    c0 = curState.stencil.func.func != state.stencil.func.func;
    c1 = curState.stencil.func.ref  != state.stencil.func.ref;
    c2 = curState.stencil.func.mask != state.stencil.func.mask;
    if(c0 || c1 || c2) {
        GL_CALL(glStencilFunc(state.stencil.func.func, state.stencil.func.ref, state.stencil.func.mask));
        curState.stencil.func.func  = state.stencil.func.func;
        curState.stencil.func.ref   = state.stencil.func.ref;
        curState.stencil.func.mask  = state.stencil.func.mask;
    }
    c0 = curState.stencil.op.front.fail   != state.stencil.op.front.fail;
    c1 = curState.stencil.op.front.zfail  != state.stencil.op.front.zfail;
    c2 = curState.stencil.op.front.zpass  != state.stencil.op.front.zpass;
    if(c0 || c1 || c2) {
        GL_CALL(glStencilOpSeparate(GL_FRONT, state.stencil.op.front.fail, state.stencil.op.front.zfail, state.stencil.op.front.zpass));
        curState.stencil.op.front.fail    = state.stencil.op.front.fail;
        curState.stencil.op.front.zfail   = state.stencil.op.front.zfail;
        curState.stencil.op.front.zpass   = state.stencil.op.front.zpass;
    }
    c0 = curState.stencil.op.back.fail   != state.stencil.op.back.fail;
    c1 = curState.stencil.op.back.zfail  != state.stencil.op.back.zfail;
    c2 = curState.stencil.op.back.zpass  != state.stencil.op.back.zpass;
    if(c0 || c1 || c2) {
        GL_CALL(glStencilOpSeparate(GL_BACK, state.stencil.op.back.fail, state.stencil.op.back.zfail, state.stencil.op.back.zpass));
        curState.stencil.op.back.fail    = state.stencil.op.back.fail;
        curState.stencil.op.back.zfail   = state.stencil.op.back.zfail;
        curState.stencil.op.back.zpass   = state.stencil.op.back.zpass;
    }

    if(curState.raster.lineWidth != state.raster.lineWidth) {
        GL_CALL(glLineWidth(state.raster.lineWidth));
        curState.raster.lineWidth = state.raster.lineWidth;
    }

    c0 = curState.color.maskEnabled.red != state.color.maskEnabled.red;
    c1 = curState.color.maskEnabled.green != state.color.maskEnabled.green;
    c2 = curState.color.maskEnabled.blue != state.color.maskEnabled.blue;
    c3 = curState.color.maskEnabled.alpha != state.color.maskEnabled.alpha;
    if(c0 || c1 || c2 || c3) {
        GL_CALL(glColorMask(state.color.maskEnabled.red, state.color.maskEnabled.green, state.color.maskEnabled.blue, state.color.maskEnabled.alpha));
        curState.color.maskEnabled = state.color.maskEnabled;
    }
}

Renderer::Renderer(void) : _time(0.0f) { }

Renderer::~Renderer() { }

static void PrintGLInfo(void) {
    common.printf("INFO - GL strings {\n");
    common.printf("\tGL_VENDOR                   = '%s'\n", glGetString(GL_VENDOR));
    common.printf("\tGL_RENDERER                 = '%s'\n", glGetString(GL_RENDERER));
    common.printf("\tGL_VERSION                  = '%s'\n", glGetString(GL_VERSION));
    common.printf("\tGL_SHADING_LANGUAGE_VERSION = '%s'\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    common.printf("} // GL strings\n");
}

#define STRINGIFY(x) #x
#define REQUIRE_EXT(x)                                                                  \
    do {                                                                                \
        if(!GLEW_##x) {                                                                 \
            common.printf("ERROR - GL extension '%s' required.\n", STRINGIFY(GL_##x));  \
            Crash();                                                                    \
        }                                                                               \
    } while(0)

static void CheckRequiredExtensions() {
    REQUIRE_EXT(NV_primitive_restart);
    REQUIRE_EXT(ARB_uniform_buffer_object);
    REQUIRE_EXT(ARB_texture_non_power_of_two);
    REQUIRE_EXT(EXT_framebuffer_object);
}

void Renderer::Init(void) {
    GL_CHECK_ERROR;

    PrintGLInfo();

    InitDebugOutput();
    CheckRequiredExtensions();

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

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    assert(0 < stencilBits);

    SetDefaultState(curState);

    GL_CALL(glEnableClientState(GL_PRIMITIVE_RESTART_NV));
    GL_CALL(glPrimitiveRestartIndexNV(Mesh::RESTART_INDEX));

    float f = 1.0f / 255.0f;
    // glClearColor(f * 176, f * 196, f * 222, 1.0f); // light steel blue
    // glClearColor(f * 70, f * 130, f * 180, 1.0f); // steel blue
    glClearColor(f * 154, f * 206, f * 235, 1.0f); // cornflower blue (crayola)
    glClearDepth(1.0f);
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    Uniforms_InitBuffers();

    _timer.Start();
}

void Renderer::Resize(int width, int height) {
    glViewport(0, 0, width, height);
    _aspect = (float)width / height;

    Uniforms_UpdateRenderTargetSize(width, height);
}

void Renderer::FinishResize() { }

static bool Cmp_Dist(const Mesh::Triangle& lhp, const Mesh::Triangle& rhp) {
    return lhp.dist > rhp.dist;
}

namespace {

    struct IsBackFace {
        float alpha;
        IsBackFace(float alpha) : alpha(alpha) { }
        bool operator()(const Mesh::Triangle& tri) const { return alpha > tri.viewAngle; }
    };

    struct IsFrontFace {
        float alpha;
        IsFrontFace(float alpha) : alpha(alpha) { }
        bool operator()(const Mesh::Triangle& tri) const { return alpha < tri.viewAngle; }
    };

    struct IsViewedEdgeOn {
        bool operator()(const Mesh::Triangle& tri) const { return 0.0f <= tri.viewAngle && tri.viewAngle < 0.01f; }
    };

} // unnamed namespace

static MeshJob* DrawMeshList(
    Program& prog,
    const State& state,
    int passType,
    int passFlags,
    const M::Matrix4& worldMat,
    const R::Renderer::GeomSortMode::Enum geomSortMode,
    MeshJob* first) 
{
    typedef std::vector<Mesh::Triangle>::iterator triIt_t;
    M::Matrix4 invWorld;
    M::TryInvert(worldMat, invWorld);
    M::Vector3 localEye = M::Transform(invWorld, M::Vector3::Zero);
    std::vector<Mesh::Triangle> tris;
    MeshJob* meshJob = first;
    while(meshJob && meshJob->fx == first->fx) {
        TFMesh& tfmesh = meshMgr.GetMesh(meshJob->tfmesh);
        Mesh& mesh = tfmesh.GetMesh();
        if(Renderer::GeomSortMode::SORT_TRIANGLES == geomSortMode && mesh.IsSolid()) {
            mesh.R_AppendTriangles(tris, localEye, tfmesh.R_TF_IndexOff());
        }
        else {
            mesh.R_TouchBuffer();
            Uniforms_UpdateModelView(worldMat * tfmesh.GetTransform());
            BindMeshAttributes();
            GL_CALL(glDrawElements(mesh.PrimitiveType(), mesh.NumIndices(), ToGLEnum<Mesh::Index>::ENUM, mesh.OffIndices()));
            metrics.frame.numDrawCalls++;
            UnbindMeshAttributes();
        }
        meshJob = meshJob->next;
    }
    triIt_t trisEnd = tris.end();
    if(state.culling.sw.enabled) {
        switch(state.culling.sw.cullFace) {
        case GL_FRONT: trisEnd = std::remove_if(tris.begin(), tris.end(), IsFrontFace(state.culling.sw.alpha)); break;
        case GL_BACK: trisEnd = std::remove_if(tris.begin(), tris.end(), IsBackFace(state.culling.sw.alpha)); break;
        default: assert(false);
        };
    }
    std::sort(tris.begin(), trisEnd, Cmp_Dist);
    std::vector<Mesh::Index> indices;
    for(triIt_t triIt(tris.begin()); trisEnd != triIt; ++triIt) {
        indices.push_back(triIt->bufIndices.indices[0]);
        indices.push_back(triIt->bufIndices.indices[1]);
        indices.push_back(triIt->bufIndices.indices[2]);
    }
    if(!indices.empty()) {
        Uniforms_UpdateModelView(worldMat);
        BindMeshAttributes();
        GL_CALL(glDrawElements(GL_TRIANGLES, indices.size(), ToGLEnum<Mesh::Index>::ENUM, &indices[0]));
        metrics.frame.numDrawCalls++;
        UnbindMeshAttributes();
    }
    return meshJob;
}

static void DrawFrame(
    const M::Matrix4& modelView,
    const M::Matrix4& projectionMat,
    R::Renderer::GeomSortMode::Enum geomSortMode,
    float time, std::vector<MeshJob>& rjobs) 
{
    if(rjobs.empty()) return;

    MeshJob* meshJob = &rjobs[0];
    while(meshJob) {
        TFMesh& tfmesh = meshMgr.GetMesh(meshJob->tfmesh);
        if(R::Renderer::GeomSortMode::SORT_TRIANGLES == geomSortMode) {
            tfmesh.TransformVertices();
            tfmesh.R_TF_Touch();
        } else {
            tfmesh.R_Touch();
        }

        meshJob = meshJob->next;
    }
    GB_CacheBuffer();

    GB_Bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    MeshJob* cur = &rjobs[0];
    MeshJob* next = NULL;
    while(cur) {
        next = NULL;

        GEN::Pointer<Effect> fx = effectMgr.GetEffect(cur->fx);
        int numPasses = fx->NumPasses();
        for(int i = 0; i < numPasses; ++i) {
            Pass* pass = fx->GetPass(i);
            pass->Use();
            
            Program&        prog = pass->GetProgram();
            const PassDesc& desc = pass->GetDesc();
            Uniforms_BindBlocks(prog);
            Uniforms_BindBuffers();
            SetState(desc.state);

            next = DrawMeshList(prog, desc.state, desc.type, desc.flags, modelView, geomSortMode, cur);
        }
        cur = next;
    }
}

static bool CompareJobs(const R::MeshJob& lhp, const R::MeshJob& rhp) {
    return effectMgr.GetEffect(lhp.fx)->SortKey() < effectMgr.GetEffect(rhp.fx)->SortKey();
}

static void Link(std::vector<R::MeshJob>& renderJobs) {
    unsigned numJobs = renderJobs.size();
	if(0 < numJobs) {
		for(unsigned i = 0; i < numJobs; ++i) {
			MeshJob& rjob = renderJobs[i];
			rjob.next = &renderJobs[(i + 1) % numJobs];
		}
		renderJobs[numJobs - 1].next = NULL;
	}
}

static M::Matrix4 ComputeProjectionMatrix(float aspect, const M::Matrix4& worldMat, const std::vector<R::MeshJob>& renderJobs) {
    /*
    float zMin = 10000.0f, zMax = -10000.0f;
    unsigned numJobs = renderJobs.size();
    for(unsigned i = 0; i < numJobs; ++i) {
        const MeshJob& rjob = renderJobs[i];
        M::Vector3 v = M::Transform(rjob.transform, M::Vector3::Zero);
        float r = rjob.mesh->Radius();
        if(0 > v.z - r) zMin = M::Min(zMin, v.z - r);
        if(0 > v.z + r) zMax = M::Max(zMax, v.z + r);
    }
    return M::Mat4::Perspective(45.0f, aspect, -zMax, -zMin);
    */
    return M::Mat4::Perspective(45.0f, aspect, 0.1f, 100.0f);
}

static SYS::Timer   frame_time;
static float        frame_secsPassed = 0.0f;

void Renderer::BeginFrame() {
    float secsPassed = _timer.Stop();
    _time += secsPassed;
    _timer.Start();

    frame_time.Start();

    metrics.frame.numDrawCalls = 0;

    if(curState.depth.maskEnabled != GL_TRUE) {
        GL_CALL(glDepthMask(GL_TRUE));
        curState.depth.maskEnabled = GL_TRUE;
    }
    const float f = 1.0f / 255.0f;
    glClearColor(f * 154, f * 206, f * 235, 1.0f); // cornflower blue (crayola)
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void Renderer::EndFrame() {
    meshMgr.R_Update();

    metrics.frame.time = frame_time.Stop();
    metrics.EndFrame();
}

void Renderer::Render(
    const RenderList& renderList, 
    const M::Matrix4& projection,
    const M::Matrix4& worldToEye, 
    const GeomSortMode::Enum geomSortMode,
    std::vector<MeshJob>& rjobs) 
{
    if(curState.depth.maskEnabled != GL_TRUE) {
        GL_CALL(glDepthMask(GL_TRUE));
        curState.depth.maskEnabled = GL_TRUE;
    }
    GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));

    Uniforms_Update(projection, worldToEye, renderList.dirLights);

    if(!rjobs.empty()) {
        GB_Bind();
        std::sort(rjobs.begin(), rjobs.end(), CompareJobs);
        Link(rjobs);
        for(unsigned i = 0; i < rjobs.size(); ++i) {
            MeshJob& rjob = rjobs[i];
            effectMgr.GetEffect(rjob.fx)->Compile();
        }
        DrawFrame(renderList.worldMat, projection, geomSortMode, _time, rjobs);
    }
}

void Renderer::Render(RenderList& renderList) {
    for(unsigned i = 0; i < Layers::NUM_LAYERS; ++i) _renderLayers[i].clear();

    for(unsigned i = 0; i < renderList.meshJobs.size(); ++i) {
        const MeshJob& rjob = renderList.meshJobs[i];
        _renderLayers[rjob.layer].push_back(rjob);
    }

    for(unsigned i = 0; i < Layers::NUM_LAYERS; ++i) {
        M::Matrix4 projection = M::Mat4::Perspective(45.0f, _aspect, 0.1f, 100.0f);
        M::Matrix4 worldToEye = renderList.worldMat;
        /*
        M::Matrix4 worldToEye = M::Mat4::Identity();
        if(Layers::GEOMETRY_0 == i) worldToEye = renderList.worldMat;
        if(Layers::GEOMETRY_1 == i) worldToEye = M::Mat4::Identity();
        */
        GeomSortMode::Enum geomSortMode = GeomSortMode::UNSORTED;
        if(Layers::GEOMETRY_TRANSPARENT == i) geomSortMode = GeomSortMode::SORT_TRIANGLES;
        if(!_renderLayers[i].empty()) Render(renderList, projection, worldToEye, geomSortMode, _renderLayers[i]);
    }
}

} // namespace R
