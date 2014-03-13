#pragma once

#include <Nubuck\operators\standard_algorithm.h>

struct Phase1_Level1 : OP::ALG::Phase {
    GEN::Pointer<Phase> nextPhase;
    bool                isWall;

    Phase1_Level1();

    void Enter() override;

    StepRet::Enum       Step() override;
    GEN::Pointer<Phase> NextPhase() override { return nextPhase; }

    bool IsWall() const override { return isWall; }
};