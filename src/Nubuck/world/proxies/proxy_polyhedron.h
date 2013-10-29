#pragma once

#include <Nubuck\nubuck.h>
#include <common\types.h>

namespace Proxy {

class Polyhedron : public IPolyhedron {
private:
    unsigned    _entId;
    std::string _name;
public:
    Polyhedron(graph_t& G);

    void Destroy(void) override;

    void SetName(const std::string& name) override;
    void SetRenderFlags(int flags) override;
    void SetPickable(bool isPickable) override;
    void SetNodeColor(leda::node node, float r, float g, float b) override;
    void SetFaceColor(leda::edge edge, float r, float g, float b) override;

    void Update(void) override;
};

} // namespace Proxy