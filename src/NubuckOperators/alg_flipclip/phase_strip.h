#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Strip : public OP::ALG::Phase {
private:
    Globals& _g;

    leda::list<leda::node>  _L;
    bool                    _needsClipping;

    int _numStrips;

    struct StepMode {
        enum Enum {
            SEARCH = 0,
            PERFORM_STRIP
        };
    };
    StepMode::Enum _stepMode;

    StepRet::Enum StepSearch();
    StepRet::Enum StepPerformStrip();
public:
    explicit Phase_Strip(Globals& g);

    void Enter() override;
    StepRet::Enum Step() override;
    GEN::Pointer<OP::ALG::Phase> NextPhase() override;
};