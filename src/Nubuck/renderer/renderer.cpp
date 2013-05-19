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

namespace R {

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

    void Renderer::Init(void) {
        GLenum ret;
        if(GLEW_OK != (ret = glewInit())) {
            common.printf("ERROR - glewInit() failed with code %d, \"%s\"\n",
                    ret, glewGetErrorString(ret));
            Crash();
        }
        common.printf("INFO - using glew version %s.\n", glewGetString(GLEW_VERSION));

        const GLubyte* glVersion = glGetString(GL_VERSION);
        common.printf("INFO - supported GL version: '%s'.\n", glVersion);

        GL_CALL(glEnable(GL_PRIMITIVE_RESTART));
        GL_CALL(glPrimitiveRestartIndex(RESTART_INDEX));

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
    }

    void Renderer::Resize(int width, int height) {
        glViewport(0, 0, width, height);
    }

    RenderJob* DrawMeshList(Program& prog, int passType, const M::Matrix4& worldMat, RenderJob* first) {
        RenderJob* meshJob = first;
        while(meshJob && meshJob->fx == first->fx) {
            if(FIRST_LIGHT_PASS == passType || LIGHT_PASS == passType)
                SetMaterialUniforms(prog, meshJob->material);

            prog.SetUniform("uTransform", worldMat * meshJob->transform);

            Mesh& mesh = MeshMgr::Instance().GetMesh(meshJob->mesh);
            mesh.Bind();
            mesh.Draw();

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

                    if(USE_COLOR & desc.flags) {
                        prog.SetUniform("uColor", desc.state.color);
                    }

                    if(FIRST_LIGHT_PASS == desc.type || LIGHT_PASS == desc.type) {
                        M::Matrix3 normalMat = M::Transpose(M::Inverse(M::RotationOf(worldMat)));
                        prog.SetUniform("uNormalMat", normalMat);
                    }

                    if(FIRST_LIGHT_PASS == desc.type || DEFAULT == desc.type) {
                        if(FIRST_LIGHT_PASS == desc.type && !_lights.empty())
                            SetLightUniforms(prog, _lights[0]);

                        next = DrawMeshList(prog, desc.type, worldMat, cur);
                    }

                    if(LIGHT_PASS == desc.type) {
                        for(int j = 1; j < _lights.size(); ++j) {
                            SetLightUniforms(prog, _lights[j]);
                            next = DrawMeshList(prog, desc.type, worldMat, cur);
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

    void Renderer::BeginFrame(void) {
        _renderJobs.clear();

        GL_CALL(glDepthMask(GL_TRUE));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void Renderer::Add(const RenderJob& renderJob) {
		if(renderJob.fx.empty() || !renderJob.mesh) return;
        _renderJobs.push_back(renderJob);
        _renderJobs.back().next = NULL;
    }

    void Renderer::EndFrame(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
        typedef std::vector<RenderJob>::iterator rjobIt_t;

        if(_renderJobs.empty()) return;

        // TODO: sort

        unsigned numJobs = _renderJobs.size();
        for(unsigned i = 0; i < numJobs; ++i) {
            RenderJob& rjob = _renderJobs[i];
            if(i < numJobs - 1) rjob.next = &_renderJobs[i + 1];
            
			effectMgr.GetEffect(rjob.fx)->Compile();
            MeshMgr::Instance().GetMesh(rjob.mesh).Compile();
        }

        DrawFrame(worldMat, projectionMat);

        R::MeshMgr::Instance().Update();
    }


} // namespace R
