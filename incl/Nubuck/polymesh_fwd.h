#pragma once

#include <LEDA\geo\d3_rat_point.h>

namespace leda {
namespace nb {

template<typename VEC3> class PolyMesh;
typedef PolyMesh<leda::d3_rat_point> RatPolyMesh;

} // namespace nb

typedef nb::PolyMesh<d3_rat_point> NbGraph; // deprecated

} // namespace leda

namespace NB {

typedef leda::nb::RatPolyMesh   Graph;
typedef leda::d3_rat_point      Point3;

} // namespace NB