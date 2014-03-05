#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>

typedef leda::d3_rat_point      point_t;
typedef leda::nb::RatPolyMesh   mesh_t;
typedef leda::list<leda::node>  hull2_t;

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

    /*
    virtual const char* Name(void) const = 0;
    virtual leda::edge Next(const graph_t& G, const leda::edge e) const = 0;
    */
};

struct Conf0 : Conf {
	const char* Name(void) const { return "P0"; }

	leda::edge Next(const mesh_t& G, const leda::edge e) const {
		return G.cyclic_adj_succ(e);
	}
};

struct Conf1 : Conf {
	const char* Name(void) const { return "P1"; }

	leda::edge Next(const mesh_t& G, const leda::edge e) const {
		return G.cyclic_adj_pred(e);
	}
};

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