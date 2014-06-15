#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Stitch : public OP::ALG::Phase {
private:
    Globals& _g;
public:
    Phase_Stitch(Globals& g);

    void Enter() override;
    StepRet::Enum Step() override;
    GEN::Pointer<OP::ALG::Phase> NextPhase() override;

    bool IsWall() const override;
};