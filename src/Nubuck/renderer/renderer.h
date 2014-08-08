#pragma once

#include <vector>

#include <Nubuck\generic\pointer.h>
#include <common\config\config.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\semaphore.h>
#include <system\timer\timer.h>
#include <Nubuck\math\matrix4.h>
#include "material\material.h"
#include "mesh\meshmgr_fwd.h"
#include "skin\skinmgr.h"
#include "light\light.h"

namespace R {

struct TransparencyMode {
    enum Enum {
        BACKFACES_FRONTFACES = 0,
        SORT_TRIANGLES,
        DEPTH_PEELING,
        NUM_MODES
    };
};

} // namespace R

extern COM::Config::Variable<float>	cvar_r_nodeSize;
extern COM::Config::Variable<float> cvar_r_edgeRadius;
extern COM::Config::Variable<int>   cvar_r_transparencyMode;
extern COM::Config::Variable<int>   cvar_r_numDepthPeels;

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
    float                       projWeight;
    float                       zoom;
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
            GEOMETRY_0_SOLID_0 = 0,
            GEOMETRY_0_USE_DEPTH_0,
            GEOMETRY_0_SOLID_1,
            GEOMETRY_0_TRANSPARENT_SORTED,
            GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_0, // first dp pass
            GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_N, // all subsequent passes
            GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_USE_DEPTH,
            GEOMETRY_0_DEPTH_ONLY,
            GEOMETRY_0_USE_DEPTH_1,
            GEOMETRY_0_SOLID_2,
            GEOMETRY_1,
            NUM_LAYERS
        };
    };

    struct GeomSortMode {
        enum Enum {
            UNSORTED = 0,
            SORT_MESHES,
            SORT_TRIANGLES
        };
    };
private:
    SYS::Timer  _timer;
    float       _time;

    int     _width, _height;
    float   _aspect;

    bool _screenshotRequested;

    std::vector<MeshJob> _renderLayers[Layers::NUM_LAYERS];

    Color _bgColor;

    void Render(
        const RenderList& renderList,
        const M::Matrix4& projection,
        const M::Matrix4& worldToEye,
        const GeomSortMode::Enum geomSortMode,
        std::vector<MeshJob>& rjobs);
public:
    Renderer(void);
    ~Renderer();

    void Init(void); // requires gl context

    const Color&    GetBackgroundColor() const;
    void            SetBackgroundColor(const Color& color);

    void Resize(int width, int height);
    void FinishResize();

    void BeginFrame();
    void Render(RenderList& renderList);
    void EndFrame();

    void Screenshot() { _screenshotRequested = true; }
};

} // namespace R
