#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\generic\pointer.h>
#include <Nubuck\polymesh.h>

typedef leda::d3_rat_point      point_t;
typedef leda::nb::RatPolyMesh   mesh_t;
typedef leda::list<leda::node>  hull2_t;

// forward declarations
struct Phase1_Level0;

// configuration (state) of P0 and P1
struct Conf { 
    leda::edge e; // pivot edge

    // node that was adjacent to first supporting edge,
    // used to determine termination of wrapping
    leda::node term;

    // current directed advancing edge, originating
    // from this polyhedron. initialized to null
    leda::edge adv;

    leda::edge first; // first edge processed in Level0Winner
    leda::edge it; // current edge processed in Level0Winner

    leda::edge self;

    Conf(void) : adv(NULL), first(NULL), it(NULL), self(NULL) { }

    virtual const char* Name(void) const = 0;
    virtual leda::edge Next(const mesh_t& G, const leda::edge e) const = 0;
};

struct Conf0 : Conf {
	const char* Name(void) const { return "P0"; }

	leda::edge Next(const mesh_t& G, const leda::edge e) const override {
		return G.cyclic_adj_succ(e);
	}
};

struct Conf1 : Conf {
	const char* Name(void) const { return "P1"; }

	leda::edge Next(const mesh_t& G, const leda::edge e) const override {
		return G.cyclic_adj_pred(e);
	}
};

enum Color { 
    BLUE, 
    ORANGE,         // processed by Level0Winner
    RED,            // non-optimal edges 
    PURPLE, 		// optimal edges
    YELLOW, 		// to be removed
    VIVID_VIOLET,   // new mantle edges
};

struct Globals {
    Nubuck nb;

	leda::edge_map<Color> edgeColors;

    leda::edge activeEdge;

    IGeometry* geom0;
    IGeometry* geom1;
    IGeometry* geom; // union of geom0, geom1

    IGeometry* geom_suppEdge;
    IGeometry* geom_activeEdge0;
    IGeometry* geom_activeEdge1;
    IGeometry* geom_activeEdge;

    // reentered phases
	GEN::Pointer<Phase1_Level0> phase1_level0P0;
	GEN::Pointer<Phase1_Level0> phase1_level0P1;

    Conf0 P0;
    Conf1 P1;

	Globals() : activeEdge(NULL), geom_activeEdge(NULL) { }
};

extern Globals g;

inline mesh_t& GetG() { return g.geom->GetRatPolyMesh(); }

void InitPhases();
void UpdateActiveEdge();

bool InHNeg(const leda::node v, const leda::node w);
bool InHPos(const leda::node v, const leda::node w);

void ConvexHullXY_Graham(
    const mesh_t& G,
    hull2_t& H,
    leda::list_item* plexMin,
    leda::list_item* plexMax,
    bool check);

void SuppEdgeXY(
	const mesh_t& G,
    const hull2_t& H0,
    const hull2_t& H1,
    const leda::list_item maxH0,
    const leda::list_item minH1,
    leda::node& v0, leda::node& v1);