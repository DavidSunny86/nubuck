#pragma once

#include "globals.h"
#include "phase_init.h"

class FlipClip : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;
protected:
    const char* GetName() const;

    OP::ALG::Phase* Init(const Nubuck& nb);
};