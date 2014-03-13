#pragma once

#include <stack>
#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"

struct Phase2 : OP::ALG::Phase {
    std::stack<leda::edge> dfs;

    void Push(const leda::edge e);

    void Enter() override;

    StepRet::Enum Step() override;
};