#pragma once

namespace LM {

    bool IsLeftTurn(int orient);
    bool IsRightTurn(int orient);

    bool InHNeg(int orient);
    bool InHPos(int orient);

    template<typename TYPE> int Sign(TYPE val);

} // namespace LM

#include "lmath_inl.h"
