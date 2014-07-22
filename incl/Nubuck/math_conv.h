#pragma once

#include <LEDA\geo\d3_rat_point.h>
#include <Nubuck\math\vector3.h>

// conversion functions

M::Vector3          ToVector(const leda::d3_rat_point& p);
leda::d3_rat_point  ToRatPoint(const M::Vector3& v);