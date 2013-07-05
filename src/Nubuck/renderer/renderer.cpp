#include <assert.h>

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
#include "renderer.h"

namespace {

    enum {
        IN_POSITION     = 0,
        IN_NORMAL       = 1,
        IN_COLOR        = 2,
        IN_TEXCOORDS    = 3
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

namespace R {

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
        GL_CALL(glEnable(GL_PRIMITIVE_RESTART));
        GL_CALL(glPrimitiveRestartIndex(Mesh::RESTART_INDEX));

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        _timer.Start();
    }

    void Renderer::Resize(int width, int height) {
        glViewport(0, 0, width, height);
        _aspect = (float)width / height;
    }

    RenderJob* DrawMeshList(Program& prog, int passType, int passFlags, const M::Matrix4& worldMat, RenderJob* first) {
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

            GL_CALL(glDrawElements(primType, numIndices, ToGLEnum<Mesh::Index>::ENUM, NULL));

            meshJob = meshJob->next;
        }
        return meshJob;
    }

    void Renderer::DrawFrame(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
        typedef std::vector<RenderJob>::iterator rjobIt_t;

        RenderJob* cur = &_renderJobs[0];
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

                    if(USE_TIME & desc.flags) prog.SetUniform("uTime", _time);
                    if(USE_COLOR & desc.flags) prog.SetUniform("uColor", desc.state.color);

                    if(FIRST_LIGHT_PASS == desc.type || LIGHT_PASS == desc.type) {
                        M::Matrix3 normalMat = M::Transpose(M::Inverse(M::RotationOf(worldMat)));
                        prog.SetUniform("uNormalMat", normalMat);
                    }

                    if(FIRST_LIGHT_PASS == desc.type || DEFAULT == desc.type) {
                        if(FIRST_LIGHT_PASS == desc.type && !_lights.empty())
                            SetLightUniforms(prog, _lights[0]);

                        next = DrawMeshList(prog, desc.type, desc.flags, worldMat, cur);
                    }

                    if(LIGHT_PASS == desc.type) {
                        for(int j = 1; j < _lights.size(); ++j) {
                            SetLightUniforms(prog, _lights[j]);
                            next = DrawMeshList(prog, desc.type, desc.flags, worldMat, cur);
                        }
                    } // LIGHT_PASS == type
                }
            }
            cur = next;
        }

        glFinish();
    }

    void Renderer::Add(const Light& light) {
        _lights.push_back(light);
    }

    void Renderer::SetRenderList(const std::vector<RenderJob>& renderList) {
        _renderListLock.Lock();
        _renderList.clear();
        for(std::vector<RenderJob>::const_iterator rlistIt(renderList.cbegin());
            renderList.cend() != rlistIt; ++rlistIt)
        {
            const RenderJob& renderJob = *rlistIt;
            if(renderJob.fx.empty() || !renderJob.mesh.IsValid()) return;
            _renderList.push_back(renderJob);
            _renderList.back().next = NULL;
        }
        _renderListLock.Unlock();
    }

    void Renderer::Render(const M::Matrix4& worldMat) {
        typedef std::vector<RenderJob>::iterator rjobIt_t;

        float secsPassed = _timer.Stop();
        _time += secsPassed;
        _timer.Start();

        GL_CALL(glDepthMask(GL_TRUE));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        _renderJobs.clear();
        _renderListLock.Lock();
        _renderJobs = _renderList;
        _renderListLock.Unlock();

        if(_renderJobs.empty()) return;

        // TODO: sort

        float zMin = 10000.0f, zMax = -10000.0f;
        unsigned numJobs = _renderJobs.size();
        for(unsigned i = 0; i < numJobs; ++i) {
            RenderJob& rjob = _renderJobs[i];
            if(i < numJobs - 1) rjob.next = &_renderJobs[i + 1];
            
            effectMgr.GetEffect(rjob.fx)->Compile();
            rjob.mesh->R_Compile();
            if(rjob.skin.IsValid()) skinMgr.R_Compile(rjob.skin);

            rjob.transform = worldMat * rjob.transform;

            M::Vector3 v = M::Transform(rjob.transform, M::Vector3::Zero);
            float r = rjob.mesh->Radius();
            if(0 > v.z - r) zMin = M::Min(zMin, v.z - r);
            if(0 > v.z + r) zMax = M::Max(zMax, v.z + r);
        }

        //M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, _aspect, -zMax, -zMin);
        M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, _aspect, 0.1f, 1000.0f);

        DrawFrame(worldMat, projectionMat);

        meshMgr.R_Update();
    }


} // namespace R
