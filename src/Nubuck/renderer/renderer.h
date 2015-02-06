#pragma once

#include <vector>

#include <Nubuck\generic\pointer.h>
#include <common\config\config.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\semaphore.h>
#include <system\timer\timer.h>
#include <Nubuck\math\vector2.h>
#include <Nubuck\math\matrix4.h>
#include "material\material.h"
#include "mesh\meshmgr_fwd.h"
#include "skin\skinmgr.h"
#include "light\light.h"

/*
GENERAL NOTES:
pen drawing uses immediate mode line strips.
*/

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
extern COM::Config::Variable<int>   cvar_r_smoothEdges;
extern COM::Config::Variable<int>   cvar_r_transparencyMode;
extern COM::Config::Variable<int>   cvar_r_numDepthPeels;

namespace R {

class   Effect;
class   Mesh;

struct PenVertex {
    M::Vector2  pos;
    R::Color    col;
    float       size;
};

const PenVertex& Pen_RestartVertex();

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
    Color                       clearColor;
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
            GEOMETRY_0_SPINE_0, // depth-only
            GEOMETRY_0_USE_DEPTH_0,
            GEOMETRY_0_USE_SPINE_0,
            GEOMETRY_0_SOLID_1,
            GEOMETRY_0_TRANSPARENT_SORTED,
            GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_0, // first dp pass
            GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_N, // all subsequent passes
            GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_USE_DEPTH,
            GEOMETRY_0_DEPTH_ONLY,
            GEOMETRY_0_SPINE_1,
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

    Color _clearColor;

    void Render(
        const RenderList& renderList,
        const M::Matrix4& projection,
        const M::Matrix4& worldToEye,
        const GeomSortMode::Enum geomSortMode,
        std::vector<MeshJob>& rjobs);
public:
    Renderer(void);
    ~Renderer();

    // requires gl context
    void Init();
    void Destroy();

    const Color&    GetClearColor() const;
    void            SetClearColor(const Color& color);

    void Resize(int width, int height);
    void FinishResize();

    void BeginFrame();
    void Render(const M::Matrix4& perspective, const M::Matrix4& ortho, RenderList& renderList);
    void Render(RenderList& renderList);
    void RenderPen(const std::vector<PenVertex>& verts);
    void EndFrame(bool present = true);

    void Screenshot() { _screenshotRequested = true; }

    void LargeScreenshot(const int imgWidth, const int imgHeight, RenderList& renderList);
};

extern Renderer theRenderer;

} // namespace R
