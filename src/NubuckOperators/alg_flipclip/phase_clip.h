#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Clip : public OP::ALG::Phase {
private:
    Globals& _g;

    unsigned _numClips;

    leda::node_list         _L;
    leda::node_array<int>   _rdeg;
    leda::node              _clipV;

    struct StepMode {
        enum Enum {
            SEARCH = 0,
            PERFORM_CLIP
        };
    };
    StepMode::Enum _stepMode;

    StepRet::Enum StepSearch();
    StepRet::Enum StepPerformClip();
public:
    explicit Phase_Clip(Globals& g);

    void Enter() override;
    StepRet::Enum Step() override;
    GEN::Pointer<OP::ALG::Phase> NextPhase() override;
};