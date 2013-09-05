#pragma once

#include <vector>

#include <generic\pointer.h>
#include <system\locks\spinlock.h>
#include <system\timer\timer.h>
#include <math\matrix4.h>
#include "material\material.h"
#include "mesh\meshmgr.h"
#include "skin\skinmgr.h"
#include "light\light.h"

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

struct RenderList {
    M::Matrix4              worldMat;
    Light                   dirLights[3];
    std::vector<Light>      lights;
    std::vector<RenderJob>  jobs;
    std::vector<M::Vector3> nodePositions;
};

class Renderer {
private:
    RenderList              _nextRenderList;
    SYS::SpinLock           _renderListLock;

    SYS::Timer  _timer;
    float       _time;

    float _aspect;

    void SetRenderList(const RenderList& renderList);
public:
    Renderer(void);

    void Init(void); // requires gl context

    void Resize(int width, int height);

    void Render(const RenderList& rlist);
};

} // namespace R
