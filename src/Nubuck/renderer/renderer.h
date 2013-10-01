#pragma once

#include <vector>

#include <generic\pointer.h>
#include <common\config\config.h>
#include <system\locks\spinlock.h>
#include <system\locks\semaphore.h>
#include <system\timer\timer.h>
#include <math\matrix4.h>
#include "material\material.h"
#include "mesh\meshmgr.h"
#include "skin\skinmgr.h"
#include "light\light.h"

enum NodeRenderType {
	R_NODETYPE_GEOMETRY		= 0,
	R_NODETYPE_BILLBOARD	= 1
};

extern COM::Config::Variable<int>	cvar_r_nodeType;
extern COM::Config::Variable<float>	cvar_r_nodeSize;
extern COM::Config::Variable<int>	cvar_r_nodeSubdiv;
extern COM::Config::Variable<int>   cvar_r_nodeSmooth;

namespace R {
	
class   Effect;
class   Mesh;

struct RenderJob {
    std::string         fx;
    meshPtr_t           mesh;
    GLenum              primType; // value != 0 overrides prim type of mesh
    Material	        material;
    SkinMgr::handle_t   skin;
    M::Matrix4          transform;

    // handled by renderer
    RenderJob* next;
};

struct Edge {
    M::Vector3 p0, p1;
};

struct RenderList {
    M::Matrix4              worldMat;
    Light                   dirLights[3];
    std::vector<Light>      lights;
    std::vector<RenderJob>  jobs;
    std::vector<M::Vector3> nodePositions;
    std::vector<Edge>       edges;
};

extern RenderList g_renderLists[2];
extern SYS::Semaphore g_rendererSem;

class Renderer {
private:
    SYS::Timer  _timer;
    float       _time;

    float _aspect;
public:
    Renderer(void);

    void Init(void); // requires gl context

    void Resize(int width, int height);

    void Render(void);
};

} // namespace R
