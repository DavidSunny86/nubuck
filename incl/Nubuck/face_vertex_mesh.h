#pragma once

#include <vector>
#include <Nubuck\polymesh_fwd.h>

void add_triangle(leda::list<unsigned>& indices,
    unsigned i0,
    unsigned i1,
    unsigned i2);

void add_quad(leda::list<unsigned>& indices,
    unsigned i0,
    unsigned i1,
    unsigned i2,
    unsigned i3);

void make_from_indices(
    const leda::list<leda::d3_rat_point>& positions,
    const leda::list<unsigned>& indices,
    leda::nb::RatPolyMesh& polyMesh);