/*
(c) Christian Jaeger 2012
file: hull_xy_graham.cpp
*/

#include "shared.h"

static const mesh_t* theGraph;

inline bool LexLess(const point_t& lhp, const point_t& rhp) {
    return lhp.xcoord() < rhp.xcoord() ||
        (lhp.xcoord() == rhp.xcoord() && lhp.ycoord() < rhp.ycoord());
}

static int Sign(const leda::rational& rat) {
    return 0 > rat ? -1 : 0 == rat ? 0 : 1;
}

static int CompareXCoord(const leda::node& lhp, const leda::node& rhp) {
    return Sign((*theGraph)[lhp].xcoord() - (*theGraph)[rhp].xcoord());
}

static int CompareLex(const leda::node& lhp, const leda::node& rhp) {
    const point_t& v = (*theGraph)[lhp];
    const point_t& w = (*theGraph)[rhp];

    if(LexLess(v, w)) return -1;
    if(v == w) return 0;
    return 1;
}

inline bool LeftTurnXY(const leda::d3_rat_point& a,
        const leda::d3_rat_point& b,
        const leda::d3_rat_point& c)
{
    return 0 < leda::orientation_xy(a, b, c);
}

inline bool RightTurnXY(const leda::d3_rat_point& a,
        const leda::d3_rat_point& b,
        const leda::d3_rat_point& c)
{
    return 0 > leda::orientation_xy(a, b, c);
}

static void CheckSorting(const hull2_t& H, const mesh_t& G) {
    std::cout << "Graham: CheckSorting: ";
    leda::list_item it = H.first();
    leda::list_item succ;
    while((succ = H.succ(it))) {
        if(G[H[it]].xcoord() > G[H[succ]].xcoord()) {
            std::cout << "FAILED" << std::endl;
            return;
        }
        it = succ;
    }
    std::cout << "PASSED" << std::endl;
}

static void CheckLexMin(leda::node min, const mesh_t& G) {
    std::cout << "Graham CheckLexMin: ";
    leda::node n;
    forall_nodes(n, G) {
        if(LexLess(G[n], G[min])) {
            std::cout << "FAILED" << std::endl;
            return;
        }
    }
    std::cout << "PASSED" << std::endl;
}

static void CheckLexMax(leda::node max, const mesh_t& G) {
    std::cout << "Graham CheckLexMax: ";
    leda::node n;
    forall_nodes(n, G) {
        if(LexLess(G[max], G[n])) {
            std::cout << "FAILED" << std::endl;
            return;
        }
    }
    std::cout << "PASSED" << std::endl;
}

static void CheckHull(hull2_t& H, const mesh_t& G) {
    std::cout << "Graham CheckHull: ";
    leda::list_item it = H.first();
    while(it) {
        leda::list_item pred = H.cyclic_pred(it);
        leda::list_item succ = H.cyclic_succ(it);
        if(RightTurnXY(G[H[pred]], G[H[it]], G[H[succ]])) {
            std::cout << "FAILED" << std::endl;
            return;
        }
        it = H.succ(it);
    }
    std::cout << "PASSED" << std::endl;
}

void ConvexHullXY_Graham(
    const mesh_t& G,
    hull2_t& H,
    leda::list_item* plexMin,
    leda::list_item* plexMax,
    bool check)
{
    std::cout << "Graham "
        << "|V| = " << G.number_of_nodes() << ", "
        << "|E| = " << G.number_of_edges() << std::endl;

    theGraph = &G;

    leda::list_item lexMin = NULL;
    leda::list_item lexMax = NULL;
    
    hull2_t S;

    leda::node n;
    forall_nodes(n, G) S.push_back(n);

    leda::list_item it = S.first();
    while(it) {
        if(!lexMin) lexMin = lexMax = it;
        if(LexLess(G[S[it]], G[S[lexMin]])) lexMin = it;
        if(LexLess(G[S[lexMax]], G[S[it]])) lexMax = it;
        it = S.succ(it);
    }

    std::cout << "Graham found lex. bounds" << std::endl;

    if(check) {
        CheckLexMin(S[lexMin], G);
        CheckLexMax(S[lexMax], G);
    }

    forall_items(it, S) {
        if(LeftTurnXY(G[S[lexMin]], G[S[lexMax]], G[S[it]])) {
            /*
            leda::list_item tmp = S.succ(it);
            S.del(it);
            it = tmp;
            */
            S.del(it);
        }
    }

    std::cout << "Graham removed upper hull" << std::endl;

    /*
    there are a few strategies to deal with points that may have identical
    coordinates.
    - search for lex. bounds twice (the second time on the sorted list)
    - swap lex. bounds to ends of the list
    - require stable sorting (this is why merge_sort is used below)
    note that seems desirable to remove points from the upper hull
    before sorting
    */
    S.merge_sort(CompareLex);

    std::cout << "Graham sorted, |S| = " << S.size() << std::endl;

    if(check) CheckSorting(S, G);

    /*
    assert(lexMin == S.first());
    assert(lexMax == S.last());
    */
    if(check) {
        CheckLexMin(S[lexMin], G);
        CheckLexMax(S[lexMax], G);
    }

    H.clear();
    H.push_back(S[lexMax]);
    H.push_back(S[lexMin]);

    it = S.succ(lexMin);
    while(it) {
        while(1 < H.size() /* !!! */ && 0 >= leda::orientation_xy(G[H[H.pred(H.last())]],
                    G[H.back()], G[S[it]]))
                H.pop_back();
        H.push_back(S[it]);
        it = S.succ(it);
    }
    H.pop_front();

    std::cout << "Graham is done" << std::endl;

    if(check) {
        CheckLexMin(H.front(), G);
        CheckLexMax(H.back(), G);

        CheckHull(H, G);
    }

    if(plexMin) *plexMin = H.first();
    if(plexMax) *plexMax = H.last();
}
