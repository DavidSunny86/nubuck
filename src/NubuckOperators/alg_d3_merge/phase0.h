#pragma once

#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"

struct Phase0 : OP::ALG::Phase {
    IGeometry* CreateHighlightedEdgeGeometry(const point_t& p0, const point_t& p1);

    void            Enter() override;

	StepRet::Enum   Step() override;
    Phase*          NextPhase() override;
};
