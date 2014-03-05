#include "lmath.h"
#include "d3_point.h"

namespace {

    int Sign(const leda::rational& r) {
        if(0 == r) return 0;
        if(0 < r) return 1;
        return -1;
    }

} // unnamed namespace

namespace LM {

    int CmpLex(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
        int cmpX = Sign(lhp.xcoord() - rhp.xcoord());
        if(cmpX) return cmpX;

        int cmpY = Sign(lhp.ycoord() - rhp.ycoord());
        if(cmpY) return cmpY;

        return Sign(lhp.zcoord() - rhp.zcoord());
    }

    leda::rational Dot(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
        return lhp.xcoord() * rhp.xcoord() 
            + lhp.ycoord() * rhp.ycoord()
            + lhp.zcoord() * rhp.zcoord();
    }

    leda::d3_rat_point Cross(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
		return leda::d3_rat_point(lhp.ycoord() * rhp.zcoord() - lhp.zcoord() * rhp.ycoord(),
					   lhp.zcoord() * rhp.xcoord() - lhp.xcoord() * rhp.zcoord(),
					   lhp.xcoord() * rhp.ycoord() - lhp.ycoord() * rhp.xcoord());
	}

    leda::rational DotXY(const leda::d3_rat_point& lhp,
            const leda::d3_rat_point& rhp)
    {
        return lhp.xcoord() * rhp.xcoord() + lhp.ycoord() * rhp.ycoord();
    }

    int DistXY(const leda::d3_rat_point& p,
            const leda::d3_rat_point& q, // -1
            const leda::d3_rat_point& r) // +1
    {
        const leda::d3_rat_point a = r - p;
        const leda::d3_rat_point b = q - p;
        return Sign(DotXY(a, a) - DotXY(b, b));
    }

    int Dist(const leda::d3_rat_point& p,
            const leda::d3_rat_point& q, // -1
            const leda::d3_rat_point& r) // +1
    {
        const leda::d3_rat_point a = r - p;
        const leda::d3_rat_point b = q - p;
        return Sign(Dot(a, a) - Dot(b, b));
    }

    bool InHNeg(const leda::d3_rat_point& v0, 
            const leda::d3_rat_point& v1,
            const leda::d3_rat_point& v2,
            const leda::d3_rat_point& p)
    {
        return InHNeg(leda::orientation(v0, v1, v2, p));
    }

} // namespace LM
