#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Flip : public OP::ALG::Phase {
private:
    Globals& _g;

    leda::list<leda::edge> _S;

    struct FlipConf {
        int orient_130;
        int orient_132;

        leda::edge e, r;
        leda::edge e1, e2, e3, e4;
    } _fp;

    struct StepMode {
        enum Enum {
            SEARCH = 0,
            PERFORM_FLIP
        };
    };
    StepMode::Enum _stepMode;

    StepRet::Enum StepSearch();
    StepRet::Enum StepPerformFlip();
public:
    explicit Phase_Flip(Globals& g);

    void Enter() override;
    StepRet::Enum Step() override;
};