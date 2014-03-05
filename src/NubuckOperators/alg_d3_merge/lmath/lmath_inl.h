#pragma once

namespace LM {

    inline bool IsLeftTurn(int orient) { return 0 < orient; }
    inline bool IsRightTurn(int orient) { return 0 > orient; }

    inline bool InHNeg(int orient) { return 0 > orient; }
    inline bool InHPos(int orient) { return 0 < orient; }

    template<typename TYPE>
    inline int Sign(TYPE val) {
        if(0 < val) return  1;
        if(0 > val) return -1;
        return 0;
    }

} // namespace LM
