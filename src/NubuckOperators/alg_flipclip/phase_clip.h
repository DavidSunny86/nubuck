#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Clip : public OP::ALG::Phase {
private:
    Globals& _g;
public:
    explicit Phase_Clip(Globals& g);

    void Enter() override;
    StepRet::Enum Step() override;
};