#pragma once

#include <Nubuck\nubuck.h>
#include <common\types.h>

class Polyhedron : public IPolyhedron {
private:
    unsigned _entId;
public:
    Polyhedron(graph_t& G);

    void Destroy(void) override;

    void SetNodeColor(leda::node node, float r, float g, float b) override;
    void SetFaceColor(leda::edge edge, float r, float g, float b) override;

    void Update(void) override;
};