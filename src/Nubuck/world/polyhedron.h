#pragma once

#include <LEDA\graph\node_map.h>

#include <Nubuck\nubuck.h>
#include <common\types.h>

class Polyhedron : public IPolyhedron {
private:
    const graph_t& _G;

    leda::node_map<int> _nodeEntIDs;
    int                 _hullID;
public:
    Polyhedron(const graph_t& G);

    void SetNodeColor(leda::node node, float r, float g, float b) override;

    void Update(void) override;
};