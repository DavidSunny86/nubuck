#pragma once

#include <Windows.h> // defines MAX_INT
#include <LEDA/geo/d3_rat_point.h>
#include <LEDA/graph/graph.h>

#ifdef NUBUCK_LIB
#define NUBUCK_API __declspec(dllexport)
#else
#define NUBUCK_API __declspec(dllimport)
#endif

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

    virtual void SetRenderFlags(int flags) = 0;
    virtual void SetPickable(bool isPickable) = 0;
    virtual void SetNodeColor(leda::node node, float r, float g, float b) = 0;
    virtual void SetFaceColor(leda::edge edge, float r, float g, float b) = 0;

    virtual void Update(void) = 0;
};

struct IWorld {
    virtual IPolyhedron* CreatePolyhedron(leda::GRAPH<leda::d3_rat_point, int>& G) = 0;
};

struct ILog {
    virtual ~ILog(void) { }

    virtual void printf(const char* format, ...) = 0;
};

struct Nubuck {
    ICommon*    common;
    IWorld*     world;
    ILog*       log;
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