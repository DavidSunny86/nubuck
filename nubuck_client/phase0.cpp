#include "phase1.h"
#include "phase0.h"

bool Phase0::IsAffinelyIndependent(const leda::node node) const {
    leda::list<point_t> points;
    for(unsigned i = 0; i < numVertices; ++i) points.push_back(g.G[g.tVerts[i]]);
    points.push_back(g.G[node]);
    return leda::affinely_independent(points);
}

void Phase0::Enter(void) {
    g.nb.log->printf("--- entering Phase0.\n");

    numVertices = 0;
    curNode = g.G.first_node();
}

void Phase0::Leave(void) {
    g.nb.log->printf("--- leaving Phase0.\n");
}

IPhase::StepRet Phase0::Step(void) {
    g.nb.log->printf("step: ");

    if(IsAffinelyIndependent(curNode)) {
        g.nb.log->printf("node with id %d is aff. indep.\n", curNode->id());
        g.polyhedron->SetNodeColor(curNode, 1.0f, 0.0f, 0.0f);

        g.tVerts[numVertices++] = curNode;

        if(4 == numVertices) {
            g.nb.log->printf("found 4 aff. indep. points, done\n");

            leda::node n = curNode;
            while(n = g.G.succ_node(n))
                g.L.append(n);

            return DONE;
        }
    } else { // curNode is affinely dependent
        g.nb.log->printf("node with id %d is aff. dep.\n", curNode->id());
        g.polyhedron->SetNodeColor(curNode, 0.0f, 0.0f, 1.0f);

        g.L.append(curNode);
    }

    curNode = g.G.succ_node(curNode);

    return CONTINUE;
}

IPhase* Phase0::NextPhase(void) {
    return new Phase1(g);
}