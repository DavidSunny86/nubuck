#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Simplify : public OP::ALG::Phase {
private:
    Globals& _g;

    leda::node_array<int>   _deg;
    leda::edge_array<bool>  _inL;
    leda::list<leda::edge>  _L;
public:
    Phase_Simplify(Globals& g);

    void Enter() override;
    StepRet::Enum Step() override;
    GEN::Pointer<OP::ALG::Phase> NextPhase() override;
};