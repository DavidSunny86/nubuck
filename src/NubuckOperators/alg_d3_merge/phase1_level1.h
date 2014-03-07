#pragma once

#include <Nubuck\operators\standard_algorithm.h>

struct Phase1_Level1 : OP::ALG::Phase {
    GEN::Pointer<Phase> nextPhase;

    Phase1_Level1();

    StepRet::Enum       Step() override;
    GEN::Pointer<Phase> NextPhase() override { return nextPhase; }
};