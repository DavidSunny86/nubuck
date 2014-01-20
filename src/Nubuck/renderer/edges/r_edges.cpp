#include "r_edges.h"

namespace R {

// remove edges with zero length
void EdgeRenderer::RemoveDegeneratedEdges(std::vector<Edge>& edges) {
    unsigned i = 0;
    while(edges.size() > i) {
        if(M::AlmostEqual(0.0f, M::Distance(edges[i].p0, edges[i].p1))) {
            std::swap(edges[i], edges.back());
            edges.pop_back();
        } else ++i;
    }
}

} // namespace R