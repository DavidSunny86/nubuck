#include "lmath/d3_point.h"
#include "shared.h"

bool AdvanceH0(
    const mesh_t& G0,
    const mesh_t& G1,
    const leda::list<leda::node>& H0,
    const leda::list<leda::node>& H1,
    leda::list_item& it0,
    const leda::list_item it1,
    bool cont = true)
{
    const leda::d3_rat_point& p1 = G1[H1[it1]];
    leda::list_item pred;
    bool updated = false;
    bool step;

    do {
        pred = H0.cyclic_pred(it0);

        const leda::d3_rat_point& p0 = G0[H0[it0]];
        const leda::d3_rat_point& p2 = G0[H0[pred]];

        int orient = leda::orientation_xy(p1, p0, p2);
        int dist = LM::DistXY(p1, p0, p2);

        step = LM::IsLeftTurn(orient); // || (0 == orient && 0 < dist);

        if(step) {
            it0 = pred;
            updated = true;
        }
    } while(step && cont);

    return updated;
}

bool AdvanceH1(
	const mesh_t& G0,
    const mesh_t& G1,
    const leda::list<leda::node>& H0,
    const leda::list<leda::node>& H1,
    const leda::list_item it0,
    leda::list_item& it1,
    bool cont = true)
{
    const leda::d3_rat_point& p0 = G0[H0[it0]];
    leda::list_item succ;
    bool updated = false;
    bool step;

    do {
        succ = H1.cyclic_succ(it1);

        const leda::d3_rat_point& p1 = G1[H1[it1]];
        const leda::d3_rat_point& p2 = G1[H1[succ]];

        int orient = leda::orientation_xy(p0, p1, p2);
        int dist = LM::DistXY(p0, p1, p2);

        step = LM::IsRightTurn(orient); // || (0 == orient && 0 < dist);

        if(step) {
            it1 = succ;
            updated = true;
        }
    } while(step && cont);

    return updated;
}

void SuppEdgeXY(
	const mesh_t& G0,
    const mesh_t& G1,
    const hull2_t& H0,
    const hull2_t& H1,
    const leda::list_item maxH0,
    const leda::list_item minH1,
    leda::node& v0, leda::node& v1)
{
    leda::list_item it0 = maxH0; 
    leda::list_item it1 = minH1;

    std::cout << "finding a supp. edge..." << std::endl;

    int i = 0;
    while(AdvanceH0(G0, G1, H0, H1, it0, it1) 
            || AdvanceH1(G0, G1, H0, H1, it0, it1)) 
    {
        ++i;
        if(!(i % 100)) std::cout << "supp. edge, iteration i = " << i << std::endl;
    }

    v0 = H0[it0];
    v1 = H1[it1];
}

void SuppEdgeXY(
	const mesh_t& G,
    const hull2_t& H0,
    const hull2_t& H1,
    const leda::list_item maxH0,
    const leda::list_item minH1,
    leda::node& v0, leda::node& v1)
{
    SuppEdgeXY(G, G, H0, H1, maxH0, minH1, v0, v1);
}


