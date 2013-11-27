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
#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\cylinder\cylinder.h>
#include <renderer\material\material.h>
#include <renderer\metrics\metrics.h>
#include <renderer\giantbuffer\giant_buffer.h>
#include "renderer_local.h"
#include "renderer.h"
#include <world\world.h>

namespace {

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

COM::Config::Variable<float>	cvar_r_nodeSize("r_nodeSize", 0.2f);
COM::Config::Variable<float>    cvar_r_edgeRadius("r_edgeRadius", 0.1f);

namespace R {

RenderList g_renderLists[2];
SYS::Semaphore g_rendererSem(0);
static int rlIdx = 0;

static MeshMgr::meshPtr_t nodeMesh;

static State curState;

enum UniformBindingIndices {
    BINDING_INDEX_HOT       = 0,
    BINDING_INDEX_LIGHTS    = 1,
    BINDING_INDEX_SKELETON  = 2
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

static UniformsHot                  uniformsHot;
static UniformsLights               uniformsLights;
static UniformsSkeleton             uniformsSkeleton;
static GEN::Pointer<StaticBuffer>   uniformsHotBuffer;
static GEN::Pointer<StaticBuffer>   uniformsLightsBuffer;
static GEN::Pointer<StaticBuffer>   uniformsSkeletonBuffer;

static void Uniforms_InitBuffers(void) {
    uniformsHotBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsHot)));
    uniformsLightsBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsLights)));
    uniformsSkeletonBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(UniformsSkeleton)));
}

void Uniforms_BindBuffers(void) {
    // somehow these bindings break on intel gpus.
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_HOT, uniformsHotBuffer->GetID()));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_LIGHTS, uniformsLightsBuffer->GetID()));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_INDEX_SKELETON, uniformsSkeletonBuffer->GetID()));
}

void Uniforms_BindBlocks(const Program& prog) {
    GLuint idx = 0;
    
    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsHot");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_HOT));

    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsLights");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_LIGHTS));

    idx = glGetUniformBlockIndex(prog.GetID(), "UniformsSkeleton");
    if(GL_INVALID_INDEX != idx) GL_CALL(glUniformBlockBinding(prog.GetID(), idx, BINDING_INDEX_SKELETON));
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

static void PrintGLInfo(void) {
    common.printf("INFO - GL strings {\n");
    common.printf("\tGL_VENDOR                   = '%s'\n", glGetString(GL_VENDOR));
    common.printf("\tGL_RENDERER                 = '%s'\n", glGetString(GL_RENDERER));
    common.printf("\tGL_VERSION                  = '%s'\n", glGetString(GL_VERSION));
    common.printf("\tGL_SHADING_LANGUAGE_VERSION = '%s'\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    common.printf("} // GL strings\n");
}

void Renderer::Init(void) {
    GL_CHECK_ERROR;

    PrintGLInfo();

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

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    assert(0 < stencilBits);

    SetDefaultState(curState);

    GL_CALL(glEnable(GL_PRIMITIVE_RESTART));
    GL_CALL(glPrimitiveRestartIndex(Mesh::RESTART_INDEX));

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
}

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

static MeshJob* DrawMeshList(Program& prog, const State& state, int passType, int passFlags, const M::Matrix4& worldMat, MeshJob* first) {
    typedef std::vector<Mesh::Triangle>::iterator triIt_t;
    M::Matrix4 invWorld;
    M::TryInvert(worldMat, invWorld);
    M::Vector3 localEye = M::Transform(invWorld, M::Vector3::Zero);
    std::vector<Mesh::Triangle> tris;
    MeshJob* meshJob = first;
    while(meshJob && meshJob->fx == first->fx) {
        Mesh& mesh = meshMgr.GetMesh(meshJob->mesh);
        mesh.AppendTriangles(tris, localEye);
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
        BindVertices();
        GL_CALL(glDrawElements(GL_TRIANGLES, indices.size(), ToGLEnum<Mesh::Index>::ENUM, &indices[0]));
        metrics.frame.numDrawCalls++;
    }
    return meshJob;
}

static void DrawFrame(RenderList& renderList, const M::Matrix4& projectionMat, float time) {
    if(renderList.meshJobs.empty()) return;

    GB_Bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    MeshJob* cur = &renderList.meshJobs[0];
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

            next = DrawMeshList(prog, desc.state, desc.type, desc.flags, renderList.worldMat, cur);
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

static void Compile(R::MeshJob& rjob) {
    effectMgr.GetEffect(rjob.fx)->Compile();
    meshMgr.GetMesh(rjob.mesh).R_Compile();
    meshMgr.GetMesh(rjob.mesh).R_Touch();
}

static void Transform(const M::Matrix4& worldMat, R::MeshJob& rjob) {
    Mesh& mesh = meshMgr.GetMesh(rjob.mesh);
    mesh.Transform(rjob.transform);
}

static void CompileAndTransform(const M::Matrix4& worldMat, R::MeshJob& rjob) {
    Compile(rjob);
    Transform(worldMat, rjob);
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

void Renderer::Render(void) {
    float secsPassed = _timer.Stop();
    _time += secsPassed;
    _timer.Start();

    if(curState.depth.maskEnabled != GL_TRUE) {
        GL_CALL(glDepthMask(GL_TRUE));
        curState.depth.maskEnabled = GL_TRUE;
    }
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

    RenderList& renderList = g_renderLists[rlIdx];

    static SYS::Timer frameTime;
    frameTime.Start();

    metrics.frame.numDrawCalls = 0;

    M::Matrix4 projectionMat = ComputeProjectionMatrix(_aspect, renderList.worldMat, renderList.meshJobs);

    Uniforms_Update(projectionMat, renderList.worldMat, renderList.dirLights);

    for(unsigned i = 0; i < renderList.renderJobs.size(); ++i) {
        Renderable* r = renderList.renderJobs[i];
        r->R_Prepare(renderList.worldMat);
        r->R_Draw(renderList.worldMat, projectionMat);
    }

    if(!renderList.meshJobs.empty()) {
        std::sort(renderList.meshJobs.begin(), renderList.meshJobs.end(), CompareJobs);
        Link(renderList.meshJobs);
        GB_Bind();
        std::for_each(renderList.meshJobs.begin(), renderList.meshJobs.end(),
            std::bind(CompileAndTransform, renderList.worldMat, std::placeholders::_1));
        GB_CacheAll();
        DrawFrame(renderList, projectionMat, _time);
    }

    meshMgr.R_Update();

    metrics.frame.time = frameTime.Stop();

    g_rendererSem.Signal();
    W::g_worldSem.Wait();
    rlIdx = 1 - rlIdx;
}


} // namespace R
