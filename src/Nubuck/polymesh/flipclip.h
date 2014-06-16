#pragma once

#include <LEDA\graph\graph.h>
#include <LEDA\geo\d3_rat_point.h>

void FlipClipHull(leda::list<leda::d3_rat_point> points, leda::GRAPH<leda::d3_rat_point, int>& mesh);