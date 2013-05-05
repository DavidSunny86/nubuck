#include <Nubuck\nubuck.h>

typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

struct Globals {
    Nubuck  nb;
    graph_t G;

    IPolyhedron* polyhedron;

    // vertices of the initial tetrahedron
    leda::node tVerts[4];

    // this is the list of nodes the algorithm successively
    // adds to the hull.
    // phase0 fills this list with every node except the
    // ones belonging to the initial tetrahedron
    leda::list<leda::node> L;
};

/*
phase1 constructs the intial tetrahedron
*/
struct Phase1 : IPhase {
    Globals& g;

    Phase1(Globals& g) : g(g) { }

    void BuildTetrahedron(graph_t& G, leda::node v0, leda::node v1, leda::node v2, leda::node v3) {
		if(0 < leda::orientation(G[v0], G[v1], G[v2], G[v3])) {
			std::swap(v2, v3);
		}

		leda::edge e01 = G.new_edge(v0, v1);
		leda::edge e02 = G.new_edge(v0, v2);
		leda::edge e03 = G.new_edge(v0, v3);

		leda::edge e10 = G.new_edge(v1, v0);
		leda::edge e13 = G.new_edge(v1, v3);
		leda::edge e12 = G.new_edge(v1, v2);
	
		leda::edge e21 = G.new_edge(v2, v1);
		leda::edge e23 = G.new_edge(v2, v3);
		leda::edge e20 = G.new_edge(v2, v0);

		leda::edge e30 = G.new_edge(v3, v0);
		leda::edge e32 = G.new_edge(v3, v2);
		leda::edge e31 = G.new_edge(v3, v1);
	
		G.set_reversal(e01, e10);
		G.set_reversal(e02, e20);
		G.set_reversal(e03, e30);
		G.set_reversal(e13, e31);
		G.set_reversal(e12, e21);
		G.set_reversal(e23, e32);
	}

    void Enter(void) override {
        g.nb.log->printf("--- entering Phase1.\n");
    }

    void Leave(void) override {
        g.nb.log->printf("--- leaving Phase1.\n");
    }

    bool IsWall(void) const override{ return true; }
    bool IsDone(void) const override { return true; }

    StepRet Step(void) override {
        BuildTetrahedron(g.G, g.tVerts[0], g.tVerts[1], g.tVerts[2], g.tVerts[3]);
        g.polyhedron->Update();
        return CONTINUE;
    }

    IPhase* NextPhase(void) override {
        return NULL;
    }
};

/*
phase0 finds four points that are not colinear.
these points are used in phase1 to construct the
initial tetrahedron.
*/
struct Phase0 : IPhase {
    Globals& g;
    
    unsigned    numVertices; // number of tetrahedron vertices found so far
    leda::node  curNode; // currently considered node

    Phase0(Globals& g) : g(g) { }

    bool Phase0::IsAffinelyIndependent(const leda::node node) const {
        leda::list<point_t> points;
        for(unsigned i = 0; i < numVertices; ++i) points.push_back(g.G[g.tVerts[i]]);
        points.push_back(g.G[node]);
        return leda::affinely_independent(points);
    }

    void Enter(void) override {
        g.nb.log->printf("--- entering Phase0.\n");

        numVertices = 0;
        curNode = g.G.first_node();
    }

    void Leave(void) override {
        g.nb.log->printf("--- leaving Phase0.\n");
    }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return true; }

    StepRet Step(void) {
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

    IPhase* NextPhase(void) override {
        return new Phase1(g);
    }
};

class Algorithm : public IAlgorithm {
private:
    Globals _globals;
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t& G) override {
        // we are expected to copy these parameters
        _globals.nb = nubuck;
        _globals.G  = G;

        // create the graphical representation of the graph
        _globals.polyhedron = _globals.nb.world->CreatePolyhedron(_globals.G);

        // we return the phase we like to start
        return new Phase0(_globals);
    }
};

int main(int argc, char *argv[])
{
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}
