#pragma once

#include <Windows.h> // defines MAX_INT
#include <LEDA/geo/d3_rat_point.h>
#include <LEDA/graph/graph.h>

typedef leda::rational      scalar_t;
typedef leda::d3_rat_point  point_t;

typedef leda::GRAPH<point_t, int> graph_t;
