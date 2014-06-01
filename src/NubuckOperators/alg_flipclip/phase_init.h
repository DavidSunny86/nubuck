#pragma once

#include <Nubuck\operators\standard_algorithm.h>

struct Globals;

class Phase_Init : public OP::ALG::Phase {
private:
    Globals& _g;
public:
    Phase_Init(Globals& g);

    void Enter() override;
};