#pragma once

#include <string>
#include <vector>

#include <Windows.h> // defines MAX_INT
#include <LEDA/geo/d3_rat_point.h>
#include <LEDA/graph/graph.h>

#include <Nubuck\nubuck_api.h>
#include <Nubuck\polymesh_fwd.h>
#include <Nubuck\math\vector3.h>

class QMenu;
class QWidget;

struct ICommon {
    virtual ~ICommon(void) { }

    virtual void printf(const char* format, ...) = 0;
};

struct IGeometry {
    virtual void Destroy() = 0;

    virtual leda::nb::RatPolyMesh& GetRatPolyMesh() = 0;

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

    virtual const std::string& GetName() const = 0;

    virtual M::Vector3 GetPosition() const = 0;

    virtual void SetName(const std::string& name) = 0;

    virtual void ApplyTransformation() = 0;

    virtual void SetPosition(const M::Vector3& position) = 0;

    virtual void HideOutline() = 0;

    virtual void Hide() = 0;
    virtual void Show() = 0;

    virtual void SetSolid(bool solid) = 0;
    virtual void SetRenderMode(int flags) = 0;
    virtual void SetRenderLayer(unsigned layer) = 0;
    virtual void SetShadingMode(ShadingMode::Enum mode) = 0;
};

struct ISelection {
    enum SelectMode {
        SELECT_NEW = 0,
        SELECT_ADD
    };

    virtual ~ISelection() { }

    virtual void Set(IGeometry* geom) = 0;
    virtual void Add(IGeometry* geom) = 0;
    virtual void Clear() = 0;

    virtual M::Vector3 GetGlobalCenter() = 0;
    virtual std::vector<IGeometry*> GetList() const = 0;

    virtual void SelectVertex(SelectMode mode, IGeometry* geom, leda::node vert) = 0;
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

    virtual ISelection* GetSelection() = 0;

    virtual IGeometry* CreateGeometry() = 0;
};

struct ILog {
    virtual ~ILog(void) { }

    virtual void printf(const char* format, ...) = 0;
};

struct IMainWindow {
    virtual ~IMainWindow() { }

    virtual QMenu* GetSceneMenu() = 0;
    virtual QMenu* GetObjectMenu() = 0;
    virtual QMenu* GetAlgorithmMenu() = 0;
    virtual QMenu* GetVertexMenu() = 0;
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