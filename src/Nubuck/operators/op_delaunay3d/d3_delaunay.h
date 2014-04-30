#include <LEDA\geo\d3_rat_point.h>

namespace leda {
namespace fork {

struct simplex_t {
    d3_rat_point verts[4];
};

void D3_DELAUNAY(const list<d3_rat_point>& L0, list<simplex_t>& simplices);

} // namespace fork
} // namespace leda