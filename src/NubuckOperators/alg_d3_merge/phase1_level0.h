#pragma once

#include <Nubuck\generic\pointer.h>
#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"

struct Phase1_Level0 : OP::ALG::Phase {
    Conf& conf;
	GEN::Pointer<Phase> nextPhase;

	Phase1_Level0(Conf& conf);

	void SetNextPhase(const GEN::Pointer<Phase>& phase);

    void Enter() override;

	StepRet::Enum       Step() override;
	GEN::Pointer<Phase> NextPhase() override { return nextPhase; }

    bool IsWall() const override { return false; }
};
