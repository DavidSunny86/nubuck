#pragma once

#include <LEDA\geo\d3_rat_point.h>

namespace leda {
namespace nb {

template<typename VEC3> class PolyMesh;
typedef PolyMesh<leda::d3_rat_point> RatPolyMesh;

} // namespace nb
} // namespace leda