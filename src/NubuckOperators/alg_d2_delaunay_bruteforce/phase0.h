#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase0 : public OP::ALG::Phase {
private:
    Globals& _g;
public:
    Phase0(Globals& g) : _g(g) { }

    void                Enter() override;

    StepRet::Enum       Step() override;
    GEN::Pointer<Phase> NextPhase() override;
};