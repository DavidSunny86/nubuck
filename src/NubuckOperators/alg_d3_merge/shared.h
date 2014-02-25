#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>

typedef leda::d3_rat_point      point_t;
typedef leda::nb::RatPolyMesh   mesh_t;
typedef leda::list<leda::node>  hull2_t;

void ConvexHullXY_Graham(
    const mesh_t& G,
    hull2_t& H,
    leda::list_item* plexMin,
    leda::list_item* plexMax,
    bool check);