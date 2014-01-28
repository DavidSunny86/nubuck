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

struct MeshJob {
    unsigned        layer;
    std::string     fx;
    tfmeshPtr_t     tfmesh;
    GLenum          primType; // value != 0 overrides prim type of mesh
    Material	    material;

    // handled by renderer
    MeshJob* next;

    MeshJob() : layer(0) { }
};

struct DirectionalLight {
    M::Vector3  direction; // NOTE: lightVec = -direction
    Color       diffuseColor;
};

struct RenderList {
    M::Matrix4                  worldMat;
    DirectionalLight        	dirLights[3];
    std::vector<MeshJob>        meshJobs;

    void Clear() {
        meshJobs.clear();
    }
};

class Renderer {
public:
    struct Layers {
        enum Enum {
            GEOMETRY_0 = 0,
            GEOMETRY_1,

            NUM_LAYERS
        };
    };
private:
    SYS::Timer  _timer;
    float       _time;

    float _aspect;

    std::vector<MeshJob> _renderLayers[Layers::NUM_LAYERS];

    void Render(
        const RenderList& renderList, 
        const M::Matrix4& projection,
        const M::Matrix4& worldToEye, 
        std::vector<MeshJob>& rjobs);
public:
    Renderer(void);

    void Init(void); // requires gl context

    void Resize(int width, int height);

    void BeginFrame();
    void Render(RenderList& renderList);
    void EndFrame();
};

} // namespace R
