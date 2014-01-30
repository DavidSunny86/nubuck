#pragma once

#include <string>

#include <Windows.h> // defines MAX_INT
#include <LEDA/geo/d3_rat_point.h>
#include <LEDA/graph/graph.h>

#include <Nubuck\polymesh_fwd.h>

#ifdef NUBUCK_LIB
#define NUBUCK_API __declspec(dllexport)
#else
#define NUBUCK_API __declspec(dllimport)
#endif

class QMenu;
class QWidget;

struct ICommon {
    virtual ~ICommon(void) { }

    virtual void printf(const char* format, ...) = 0;
};

#define POLYHEDRON_RENDER_HULL      (1 << 0)
#define POLYHEDRON_RENDER_NODES     (1 << 1)
#define POLYHEDRON_RENDER_EDGES     (1 << 2)

struct IPolyhedron {
    virtual ~IPolyhedron(void) { }

    virtual void Destroy(void) = 0;

    virtual leda::GRAPH<leda::d3_rat_point, int>& GetGraph(void) = 0;

    virtual void SetName(const std::string& name) = 0;
    virtual void SetEffect(const char* fxName) = 0;
    virtual void SetRenderFlags(int flags) = 0;
    virtual void SetPickable(bool isPickable) = 0;
    virtual void SetNodeColor(leda::node node, float r, float g, float b) = 0;
    virtual void SetEdgeColor(leda::edge edge, float r, float g, float b) = 0;
    virtual void SetFaceColor(leda::edge edge, float r, float g, float b) = 0;
    virtual void SetFaceColor(leda::edge edge, float r, float g, float b, float a) = 0;
    virtual void HideFace(leda::edge edge) = 0;
    virtual void ShowFace(leda::edge edge) = 0;

    virtual void Update(void) = 0;
};

struct IMesh {
    virtual ~IMesh(void) { }

    virtual void SetPosition(float x, float y, float z) = 0;
    virtual void SetScale(float sx, float sy, float sz) = 0;
    virtual void AlignZ(float x, float y, float z) = 0;
    virtual void SetOrient(float x0, float y0, float z0, float x1, float y1, float z1) = 0;
    virtual void SetEffect(const char* fxName) = 0;
    virtual void SetVisible(bool isVisible) = 0;
};

struct IGeometry {
    virtual void Destroy() = 0;

    virtual leda::nb::RatPolyMesh& GetRatPolyMesh() = 0;
    virtual void Update() = 0;

    struct RenderMode {
        enum Flags {
            FACES    = (1 << 0),
            NODES    = (1 << 1),
            EDGES    = (1 << 2)
        };
    };

    struct ShadingMode {
        enum Enum {
            NICE = 0,
            FAST
        };
    };

    virtual float GetPosX() const = 0;
    virtual float GetPosY() const = 0;
    virtual float GetPosZ() const = 0;

    virtual void SetPosition(float x, float y, float z) = 0;
    virtual void Rotate(float ang, float x, float y, float z) = 0;

    virtual void Hide() = 0;
    virtual void Show() = 0;

    virtual void SetRenderMode(int flags) = 0;
    virtual void SetRenderLayer(unsigned layer) = 0;
    virtual void SetShadingMode(ShadingMode::Enum mode) = 0;
};

struct IWorld {
    struct PlaneDesc {
        struct Sample2 { float x, y; };
        typedef float (*heightFunc_t)(float x, float y);

        heightFunc_t    heightFunc;
        bool            flip;
        float           size;
        int             subdiv;
        Sample2*        addSamples;
        unsigned        numAddSamples;
    };

    struct SphereDesc {
        int     numSubdiv;
        bool    smooth;
    };

    struct CylinderDesc {
        float       radius;
        float   	height;
        unsigned    numSlices;
        bool        caps;
    };

    virtual IPolyhedron*    CreatePolyhedron(void) = 0;
    virtual IGeometry*      CreateGeometry() = 0;
    virtual IMesh*          CreatePlaneMesh(const PlaneDesc& desc) = 0;
    virtual IMesh*          CreateSphereMesh(const SphereDesc& desc) = 0;
    virtual IMesh*          CreateCylinderMesh(const CylinderDesc& desc) = 0;

    virtual void SelectGeometry(IGeometry* geometry) = 0;
};

struct ILog {
    virtual ~ILog(void) { }

    virtual void printf(const char* format, ...) = 0;
};

struct IMainWindow {
    virtual ~IMainWindow() { }

    virtual QMenu* GetSceneMenu() = 0;
    virtual QMenu* GetObjectMenu() = 0;
    virtual void SetOperatorName(const char* name) = 0;
    virtual void SetOperatorPanel(QWidget* panel) = 0;
};

struct Nubuck {
    ICommon*        common;
    IWorld*     	world;
    ILog*           log;
    IMainWindow*    ui;
};

struct IPhase {
    enum StepRet {
        DONE = 0,
        CONTINUE
    };

    virtual ~IPhase(void) { }

    virtual void Enter(void) = 0;
    virtual void Leave(void) = 0;

    virtual bool IsWall(void) const = 0;
    virtual bool IsDone(void) const = 0;

    virtual StepRet     Step(void) = 0;
    virtual IPhase*     NextPhase(void) = 0;

    virtual void        OnNodesMoved(void) = 0;
    virtual void        OnKeyPressed(char c) = 0;
};

struct IAlgorithm {
    virtual ~IAlgorithm(void) { }

    virtual IPhase* Init(const Nubuck& nubuck, const leda::GRAPH<leda::d3_rat_point, int>& G) = 0;

    // return false if the algorithm implements no special Run method
    virtual bool Run(void) = 0;
};

typedef IAlgorithm* (*algAlloc_t)(void);

template<typename ALGORITHM>
IAlgorithm* CreateAlgorithm(void) {
    return new ALGORITHM();
}

// main entry point, returns exit code
NUBUCK_API int RunNubuck(int argc, char* argv[], algAlloc_t algAlloc);