#pragma once

#include <LEDA\graph\edge_set.h>
#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class Phase_Flip : public OP::ALG::Phase {
private:
    typedef leda::rat_point point2_t;

    Globals& _g;

    struct Quadrilateral {
        typedef leda::rat_point point2_t;

        leda::edge  e, r, e1, e2, e3, e4;
        point2_t    p0, p1, p2, p3;

        Quadrilateral();
        Quadrilateral(const leda::nb::RatPolyMesh&, leda::edge e);

        bool IsNull() const;
    } _q;

    leda::edge_set _S;

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
    Phase_Flip(Globals& g)
        : _g(g)
        , _S(nubuck().poly_mesh(g.delaunay))
        , _stepMode(StepMode::SEARCH)
    { }

    void            Enter() override;
    StepRet::Enum   Step() override;
};