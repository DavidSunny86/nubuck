#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"

struct Phase1_Level0 : OP::ALG::Phase {
    Conf& conf;

	Phase1_Level0(Conf& conf) : conf(conf) { }

	StepRet::Enum Step() override;
};
