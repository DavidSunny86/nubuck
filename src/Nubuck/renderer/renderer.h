#pragma once

#include <vector>

#include <generic\pointer.h>
#include <common\config\config.h>
#include <system\locks\spinlock.h>
#include <system\locks\semaphore.h>
#include <system\timer\timer.h>
#include <math\matrix4.h>
#include "material\material.h"
#include "mesh\meshmgr_fwd.h"
#include "skin\skinmgr.h"
#include "light\light.h"

extern COM::Config::Variable<float>	cvar_r_nodeSize;
extern COM::Config::Variable<float> cvar_r_edgeRadius;

namespace R {
	
class   Effect;
class   Mesh;

struct Renderable {
    virtual ~Renderable(void) { }

    virtual void R_Prepare(const M::Matrix4& worldMat) = 0;
    virtual void R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) = 0;
};

struct MeshJob {
    std::string     fx;
    meshPtr_t       mesh;
    GLenum          primType; // value != 0 overrides prim type of mesh
    Material	    material;
    M::Matrix4      transform;

    // handled by renderer
    MeshJob* next;
};

struct DirectionalLight {
    M::Vector3  direction; // NOTE: lightVec = -direction
    Color       diffuseColor;
};

struct RenderList {
    M::Matrix4                  worldMat;
    DirectionalLight        	dirLights[3];
    std::vector<Renderable*>    renderJobs;
    std::vector<MeshJob>        meshJobs;

    void Clear() {
        renderJobs.clear();
        meshJobs.clear();
    }
};

class Renderer {
private:
    SYS::Timer  _timer;
    float       _time;

    float _aspect;
public:
    Renderer(void);

    void Init(void); // requires gl context

    void Resize(int width, int height);

    void BeginFrame();
    void Render(RenderList& renderList);
    void EndFrame();
};

} // namespace R
