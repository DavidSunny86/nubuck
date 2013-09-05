#include <assert.h>

#include <functional>
#include <algorithm>

#include <common\common.h>
#include <math\matrix3.h>
#include <math\matrix4.h>
#include <renderer\glew\glew.h>
#include <renderer\glcall.h>
#include <renderer\effects\effect.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\effects\pass.h>
#include <renderer\mesh\mesh.h>
#include <renderer\material\material.h>
#include <renderer\metrics\metrics.h>
#include "renderer.h"

namespace {

struct InstanceData {
    M::Matrix4  transform;
    R::Material    material;
};

enum {
    IN_POSITION         = 0,
    IN_NORMAL           = 1,
    IN_COLOR            = 2,
    IN_TEXCOORDS        = 3,

    IN_INS_TRANSFORM_A0             = 4,
    IN_INS_TRANSFORM_A1     		= 5,
    IN_INS_TRANSFORM_A2     		= 6,
    IN_INS_TRANSFORM_A3     		= 7,
    IN_INS_MATERIAL_DIFFUSE         = 8
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

void BindInstanceData(void) {
    // the 4x4 transformation matrix is passed as 3 vec3 values.
    // Ai is the vector of the top 3 elements fo the i-th column of the matrix
    for(int col = 0; col < 4; ++col) {
        GL_CALL(glVertexAttribPointer(IN_INS_TRANSFORM_A0 + col, 
            3, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
            (void*)(offsetof(InstanceData, transform) + sizeof(float) * 4 * col)));
        GL_CALL(glEnableVertexAttribArray(IN_INS_TRANSFORM_A0 + col));
        GL_CALL(glVertexAttribDivisorARB(IN_INS_TRANSFORM_A0 + col, 1));
    }

    GL_CALL(glVertexAttribPointer(IN_INS_MATERIAL_DIFFUSE,
        4, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
        (void*)offsetof(InstanceData, material)));
    GL_CALL(glEnableVertexAttribArray(IN_INS_MATERIAL_DIFFUSE));
    GL_CALL(glVertexAttribDivisorARB(IN_INS_MATERIAL_DIFFUSE, 1));
}

template<typename T> struct ToGLEnum { };
template<> struct ToGLEnum<unsigned>    { enum { ENUM = GL_UNSIGNED_INT }; };
template<> struct ToGLEnum<int>         { enum { ENUM = GL_INT }; };

} // unnamed namespace

namespace R {

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

static const float BILLBOARD_SIZE = 0.1f;
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

static void InitBillboards(const std::vector<M::Vector3>& positions) {
    unsigned numBillboards = positions.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;

    if(!numBillboards) return;

    billboardPositions = positions;

    billboardsHot.clear();
    billboardsHot.resize(numBillboards);
    if(!billboardHotVertexBuffer.IsValid() || sizeof(BillboardHot) * numBillboards > billboardHotVertexBuffer->GetSize()) {
        if(billboardHotVertexBuffer.IsValid()) billboardHotVertexBuffer->Destroy();
        billboardHotVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, NULL, sizeof(BillboardHot) * numBillboards));
    }

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
    if(!billboardColdVertexBuffer.IsValid() || sizeof(BillboardCold) * numBillboards > billboardColdVertexBuffer->GetSize()) {
        if(billboardColdVertexBuffer.IsValid()) billboardColdVertexBuffer->Destroy();
        billboardColdVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, &billboardsCold[0], sizeof(BillboardCold) * numBillboards));
    }

    billboardIndices.clear();
    billboardIndices.reserve(numBillboardIndices);
    for(unsigned i = 0; i < 4 * numBillboards; ++i) {
        if(0 < i && 0 == i % 4) billboardIndices.push_back(Mesh::RESTART_INDEX);
        billboardIndices.push_back(i);
    }
    assert(numBillboardIndices == billboardIndices.size());
    if(!billboardIndexBuffer.IsValid() || sizeof(Mesh::Index) * numBillboardIndices > billboardIndexBuffer->GetSize()) {
        if(billboardIndexBuffer.IsValid()) billboardIndexBuffer->Destroy();
        billboardIndexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, &billboardIndices[0], sizeof(Mesh::Index) * numBillboardIndices));
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
    static const BillboardHotVertex hotVertices[4] = {
        { M::Vector3(-BILLBOARD_SIZE, -BILLBOARD_SIZE, 0.0f) },
        { M::Vector3(-BILLBOARD_SIZE,  BILLBOARD_SIZE, 0.0f) },
        { M::Vector3( BILLBOARD_SIZE,  BILLBOARD_SIZE, 0.0f) },
        { M::Vector3( BILLBOARD_SIZE, -BILLBOARD_SIZE, 0.0f) }
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

struct DrawCall {
    GEN::Pointer<Effect>    fx;
    meshPtr_t               mesh;
    GLenum                  primType;
    SkinMgr::handle_t       skin;
    unsigned                insIdx;
    unsigned                insCnt;
};

static const unsigned INSTANCE_BUFFER_SIZE = 10 * 1024 * 1024 * sizeof(char);

static std::vector<InstanceData> instanceData;
static GEN::Pointer<StaticBuffer> instanceBuffer;

void InitDebugOutput(void);

void SetEnabled(GLenum state, GLboolean enabled) {
    if(enabled) GL_CALL(glEnable(state));
    else        GL_CALL(glDisable(state));
}

void SetState(const State& state) {
    SetEnabled(GL_BLEND, state.blend.enabled);
    glBlendFunc(state.blend.srcFactor, state.blend.dstFactor);

    // SetEnabled(GL_DEPTH_TEST, state.depth.enabled);
    GL_CALL(glDepthMask(state.depth.maskEnabled));
    GL_CALL(glDepthFunc(state.depth.func));

    GL_CALL(glLineWidth(state.raster.lineWidth));
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

Renderer::Renderer(void) : _time(0.0f) {
}

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
    GL_CALL(glEnable(GL_PRIMITIVE_RESTART));
    GL_CALL(glPrimitiveRestartIndex(Mesh::RESTART_INDEX));

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    instanceBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, NULL, INSTANCE_BUFFER_SIZE));

    _timer.Start();
}

void Renderer::Resize(int width, int height) {
    glViewport(0, 0, width, height);
    _aspect = (float)width / height;
}

static void Draw(Program& prog, int passFlags, const M::Matrix4& worldMat, DrawCall& drawCall) {
    if(USE_TEX_DIFFUSE & passFlags) {
        skinMgr.R_Bind(prog, drawCall.skin);
    }

    instanceBuffer->Update_Mapped(0, sizeof(InstanceData) * drawCall.insCnt, &instanceData[drawCall.insIdx]);
    // instanceBuffer->Bind(); bound by Update()
    BindInstanceData();

    drawCall.mesh->R_Bind();
    BindVertices();
    unsigned numIndices = drawCall.mesh->NumIndices();

    GLenum primType = drawCall.primType;
    if(!primType) primType = drawCall.mesh->PrimitiveType();

    metrics.frame.numDrawCalls++;
    GL_CALL(glDrawElementsInstanced(primType, numIndices, ToGLEnum<Mesh::Index>::ENUM, NULL, drawCall.insCnt));
    instanceBuffer->Discard();
}

static void DrawFrame(
    RenderList& renderList, 
    std::vector<DrawCall> drawCalls,
    const M::Matrix4& projectionMat, 
    float time) 
{
    for(unsigned i = 0; i < drawCalls.size(); ++i) {
        DrawCall& drawCall = drawCalls[i];
        GEN::Pointer<Effect> fx = drawCall.fx;

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
                Draw(prog, desc.flags, renderList.worldMat, drawCall);
            }

            if(LIGHT_PASS == desc.type) {
                for(int j = 1; j < renderList.lights.size(); ++j) {
                    SetLightUniforms(prog, renderList.lights[j]);
                    Draw(prog, desc.flags, renderList.worldMat, drawCall);
                }
            } // LIGHT_PASS == type
        } // forall passes
    } // forall drawcalls
}

void Renderer::SetRenderList(const RenderList& renderList) {
    _renderListLock.Lock();
    _nextRenderList.worldMat = renderList.worldMat;
    _nextRenderList.lights = renderList.lights;
    _nextRenderList.jobs.clear();
    for(std::vector<RenderJob>::const_iterator rlistIt(renderList.jobs.cbegin());
        renderList.jobs.cend() != rlistIt; ++rlistIt)
    {
        const RenderJob& renderJob = *rlistIt;
        if(renderJob.fx.empty() || !renderJob.mesh.IsValid()) return;
        _nextRenderList.jobs.push_back(renderJob);
        _nextRenderList.jobs.back().next = NULL;
    }
    _nextRenderList.nodePositions = renderList.nodePositions;
    _renderListLock.Unlock();
}

static void Link(std::vector<R::RenderJob>& renderJobs) {
    unsigned numJobs = renderJobs.size();
    for(unsigned i = 0; i < numJobs; ++i) {
        RenderJob& rjob = renderJobs[i];
        if(i < numJobs - 1) rjob.next = &renderJobs[i + 1];
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

// arbitrary ordering on colors
static int CompareColors(const Color& lhp, const Color& rhp) {
    int s = 0;
    s = M::Sign(lhp.r - rhp.r);
    if(0 != s) return s;
    s = M::Sign(lhp.g - rhp.g);
    if(0 != s) return s;
    s = M::Sign(lhp.b - rhp.b);
    if(0 != s) return s;
    s = M::Sign(lhp.a - rhp.a);
    return s;
}

// arbitrary ordering on materials
static int CompareMaterials(const Material& lhp, const Material& rhp) {
    return CompareColors(lhp.diffuseColor, rhp.diffuseColor);
}

static int CompareRenderJobs(const RenderJob& lhp, const RenderJob& rhp) {
    int cmp = 0;
    cmp = lhp.fx.compare(rhp.fx);
    if(0 != cmp) return cmp;
    cmp = SkinMgr::Compare(lhp.skin, rhp.skin);
    if(0 != cmp) return cmp;
    cmp = CompareMaterials(lhp.material, rhp.material);
    return cmp;
}

void Renderer::Render(const RenderList& rlist) {
    SetRenderList(rlist);

    float secsPassed = _timer.Stop();
    _time += secsPassed;
    _timer.Start();

    GL_CALL(glDepthMask(GL_TRUE));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    RenderList renderList;
    _renderListLock.Lock();
    renderList = _nextRenderList;
    _renderListLock.Unlock();

    if(renderList.jobs.empty() && renderList.nodePositions.empty()) return;

    static SYS::Timer frameTime;
    frameTime.Start();

    Link(renderList.jobs);
    std::for_each(renderList.jobs.begin(), renderList.jobs.end(),
        std::bind(CompileAndTransform, renderList.worldMat, std::placeholders::_1));
    M::Matrix4 projectionMat = ComputeProjectionMatrix(_aspect, renderList.worldMat, renderList.jobs);

    instanceData.clear();
    std::vector<DrawCall> drawCalls;
    std::sort(renderList.jobs.begin(), renderList.jobs.end(), CompareRenderJobs);
    for(unsigned i = 0; i < renderList.jobs.size(); ) {
        const RenderJob& rjob = renderList.jobs[i];
        DrawCall drawCall;
        drawCall.fx = effectMgr.GetEffect(rjob.fx);
        drawCall.mesh = rjob.mesh;
        drawCall.primType = rjob.primType;
        drawCall.skin = rjob.skin;
        drawCall.insIdx = instanceData.size();
        drawCall.insCnt = 1;
        InstanceData insData;
        insData.material = rjob.material;
        insData.transform = rjob.transform;
        instanceData.push_back(insData);
        i++;
        while(i < renderList.jobs.size() && drawCall.mesh == renderList.jobs[i].mesh) {
            InstanceData insData;
            insData.material = renderList.jobs[i].material;
            insData.transform = renderList.jobs[i].transform;
            instanceData.push_back(insData);
            drawCall.insCnt++;
            i++;
        }
        drawCalls.push_back(drawCall);
    }

    metrics.frame.numDrawCalls = 0;
    DrawFrame(renderList, drawCalls, projectionMat, _time);

    InitBillboards(renderList.nodePositions);
    BuildBillboards(renderList.worldMat);
    unsigned numBillboards = billboardPositions.size();
    if(numBillboards) {
        billboardHotVertexBuffer->Update_Mapped(0, sizeof(BillboardHot) * numBillboards, &billboardsHot[0]);
        DrawBillboards(renderList.worldMat, projectionMat);
        billboardHotVertexBuffer->Discard();
    }

    meshMgr.R_Update();

    GL_CALL(glFinish());
    metrics.frame.time = frameTime.Stop();
}


} // namespace R
