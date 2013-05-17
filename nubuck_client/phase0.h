#pragma once

#include <Nubuck\nubuck.h>
#include "globals.h"

/*
phase0 finds four points that are not colinear.
these points are used in phase1 to construct the
initial tetrahedron.
*/
struct Phase0 : IPhase {
    Globals& g;
    
    unsigned    numVertices; // number of tetrahedron vertices found so far
    leda::node  curNode; // currently considered node

    Phase0(Globals& g) : g(g) { }

    bool Phase0::IsAffinelyIndependent(const leda::node node) const;

    void Enter(void) override;
    void Leave(void) override;

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return true; }

    StepRet Step(void) override;
    IPhase* NextPhase(void) override;
};