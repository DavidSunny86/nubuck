#pragma once

#include <LEDA\graph\node_map.h>

#include <Nubuck\nubuck.h>
#include <common\types.h>
#include <system\locks\semaphore.h>

class Polyhedron : public IPolyhedron {
private:
    graph_t& _G;

    leda::node_map<int> _nodeEntIDs;
    leda::edge_map<int> _faceEntIDs;
    int                 _hullID;

    SYS::Semaphore _rebuildSem;

    void SetFaceColorSolid(leda::edge edge, float r, float g, float b);
    void SetFaceColorRing(leda::edge edge, float r, float g, float b);
public:
    Polyhedron(graph_t& G);

    void Destroy(void) override;

    void SetNodePosition(leda::node node, const point_t& point) override;

    void SetNodeColor(leda::node node, float r, float g, float b) override;
    void SetFaceColor(leda::edge edge, float r, float g, float b) override;

    void Update(void) override;
};