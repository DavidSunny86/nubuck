#pragma once

#include <LEDA\graph\node_map.h>

#include <Nubuck\nubuck.h>
#include <common\types.h>
#include <system\locks\semaphore.h>

class Polyhedron : public IPolyhedron {
private:
    const graph_t& _G;

    leda::node_map<int> _nodeEntIDs;
    int                 _hullID;

    SYS::Semaphore _rebuildSem;
public:
    Polyhedron(const graph_t& G);

    void SetNodeColor(leda::node node, float r, float g, float b) override;

    void Update(void) override;
};