#pragma once

#include <Nubuck\nubuck.h>
#include "globals.h"

/* phase2 successively adds new points to the hull */
struct Phase2 : IPhase {
    enum FaceState {
        NOT_VISITED = 0,

        VISIBLE,
        INVISIBLE
    };

    Globals& g;

    leda::node curNode;
    leda::edge_map<FaceState> edges;

    Phase2(Globals& g) : g(g) { }

    leda::edge NextUnvisitedEdge(void) const;
    void MarkFace(leda::edge e, FaceState state);
    bool IsVisible(leda::node n, leda::edge e) const;
    leda::edge GetHorizontEdge(void) const;
    leda::edge NextHorizontEdge(leda::edge e) const;

    void Enter(void) override;
    void Leave(void) override;

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return false; }

    StepRet Step(void) override;
    IPhase* NextPhase(void) override;
};