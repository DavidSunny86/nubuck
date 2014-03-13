#pragma once

#include <Nubuck\operators\standard_algorithm.h>

struct Phase3 : OP::ALG::Phase {
    void Enter() override;

    StepRet::Enum Step() override;
};