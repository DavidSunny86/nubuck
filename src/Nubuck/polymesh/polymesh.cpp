#include <Nubuck\polymesh.h>

namespace leda {
namespace nb {

void D3_HULL(list<d3_rat_point> L, RatPolyMesh& mesh) {
    typedef d3_rat_point point3_t;

    mesh.clear();

    L.sort();
    L.unique();

    point3_t A = L.pop();
    assert(!L.empty());

    point3_t B = L.pop();
    while(!L.empty() && collinear(A, B, L.head())) B = L.pop();
    assert(!L.empty());

    point3_t C = L.pop();

    mesh.make_triangle(A, B, C);
}

} // namespace nb
} // namespace leda