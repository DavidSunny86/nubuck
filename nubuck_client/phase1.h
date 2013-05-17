#pragma once

#include <Nubuck\nubuck.h>
#include "globals.h"

/*
phase1 constructs the intial tetrahedron
*/
struct Phase1 : IPhase {
    Globals& g;

    Phase1(Globals& g) : g(g) { }

    void Enter(void) override;
    void Leave(void) override;

    bool IsWall(void) const override{ return true; }
    bool IsDone(void) const override { return false; }

    StepRet Step(void) override;
    IPhase* NextPhase(void) override;
};