#pragma once

#include <Nubuck\nubuck.h>
#include <common\types.h>

namespace Proxy {

class Polyhedron : public IPolyhedron {
private:
    graph_t     _G;
    unsigned    _entId;
    std::string _name;
public:
    Polyhedron(const graph_t& G);

    void Destroy(void) override;

    graph_t& GetGraph(void) { return _G; }

    void SetName(const std::string& name) override;
    void SetEffect(const char* fxName) override;
    void SetRenderFlags(int flags) override;
    void SetPickable(bool isPickable) override;
    void SetNodeColor(leda::node node, float r, float g, float b) override;
    void SetEdgeColor(leda::edge edge, float r, float g, float b) override;
    void SetFaceColor(leda::edge edge, float r, float g, float b) override;
    void SetFaceColor(leda::edge edge, float r, float g, float b, float a) override;
    void HideFace(leda::edge edge) override;
    void ShowFace(leda::edge edge) override;

    void Update(void) override;
};

} // namespace Proxy