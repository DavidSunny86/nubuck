#include <algorithm>
#include <Nubuck\nubuck.h>

typedef leda::rational              scalar_t;
typedef leda::d3_rat_point          point3_t;
typedef leda::GRAPH<point3_t, int>  graph3_t;

enum EdgeColorName { BLUE = 0, RED, BLACK };

struct Color { 
    float r, g, b; 

    Color(void) { }
    Color(const Color& other) : r(other.r), g(other.g), b(other.b) { }
    Color(float r, float g, float b) : r(r), g(g), b(b) { }
};

static Color ColorFromName(EdgeColorName name) {
    switch(name) {
    case BLUE:  return Color(0.0f, 0.0f, 1.0f);
    case RED:   return Color(1.0f, 0.0f, 0.0f);
    case BLACK: return Color(0.0f, 0.0f, 0.0f);
    default:    assert(0);
    };
    assert(0);
    return Color();
}

static void ApplyColors(IPolyhedron* ph) {
    graph3_t& G = ph->GetGraph();
    leda::edge e;
    forall_edges(e, G) {
        Color c = ColorFromName(EdgeColorName(G[e]));
        ph->SetEdgeColor(e, c.r, c.g, c.b);
    }
}

struct Globals {
    Nubuck nb;
    IPolyhedron* ph;
    leda::edge_array<Color> edgeColors;
};

struct CmpNodes : leda::leda_cmp_base<leda::node> {
    const graph3_t& G;
    CmpNodes(const graph3_t& G) : G(G) { }
    int operator()(const leda::node& lhp, const leda::node& rhp) const override {
        return point3_t::cmp_xyz(G[lhp], G[rhp]);
    }
};

inline bool equal_xy(const point3_t& p, const point3_t& q)
{ return point3_t::cmp_x(p,q) == 0 && point3_t::cmp_y(p,q) == 0; }

// triangulates nodes of graph G. returns an edge of the projected hull
static leda::edge Triangulate(graph3_t& G) {
    assert(0 == G.number_of_edges());

    leda::list<leda::node> L(G.all_nodes());
    if(L.empty()) return NULL;
    L.sort(CmpNodes(G));

    leda::node          last_v = L.pop();
    leda::d3_rat_point  last_p = G[last_v];

  	while (!L.empty() && equal_xy(last_p,G[L.head()])) 
  	{ leda::node v = L.pop();
  	  G.del_node(v);
  	 }

  	if (!L.empty())
  	{ leda::node v = L.pop();
  	  last_p = G[v];
  	  leda::edge x = G.new_edge(last_v,v,0);
  	  leda::edge y = G.new_edge(v,last_v,0);
  	  G[x] = G[y] = BLACK;
  	  G.set_reversal(x,y);
  	  last_v = v;
  	 }

  	// scan remaining points

  	leda::node v;
  	forall(v,L) 
  	{ 
  	  leda::d3_rat_point p = G[v];
  	  
  	  if (equal_xy(p,last_p)) 
  	  { G.del_node(v);
  	    continue; 
  	   }


  	  // walk up to upper tangent
  	  leda::edge e = G.last_edge();
      int orient = 1;
  	  do e = G.pred_face_edge(e); 
  	  while (orientation_xy(p,G[source(e)],G[target(e)]) == orient);

  	  // walk down to lower tangent and triangulate
  	  do { leda::edge succ_e = G.succ_face_edge(e);
  	       leda::edge x = G.new_edge(succ_e,v,0, LEDA::after);
  	       leda::edge y = G.new_edge(v,source(succ_e),0);
  	       G[x] = G[y] = RED;
  	       G.set_reversal(x,y);
  	       e = succ_e;
  	     } while (orientation_xy(p,G[source(e)],G[target(e)]) == orient);

  	  last_p = p;
  	 }

  	 leda::edge e_hull = G.last_edge();
  	 leda::edge e = e_hull;
  	 do { leda::edge r = G.reversal(e);
  	      G[e] = G[r] = BLUE;
  	      e = G.face_cycle_succ(e);
  	 } while (e != e_hull);

  	return G.last_edge();
}

struct Phase0 : IPhase {
    Globals& g;

    Phase0(Globals& g) : g(g) { }

    void Enter(void) override { }
    void Leave(void) override { }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return true; }

    StepRet Step(void) override {
        return CONTINUE;
    }

    IPhase* NextPhase(void) override {
        return NULL;
    }

    void OnNodesMoved(void) override { }
    void OnKeyPressed(char) override { }
};

struct Algorithm : IAlgorithm {
    Globals g;

    IPhase* Init(const Nubuck& nb, const graph3_t& G) {
        g.nb = nb;
        g.ph = nb.world->CreatePolyhedron();
        g.ph->SetRenderFlags(POLYHEDRON_RENDER_NODES | POLYHEDRON_RENDER_EDGES | POLYHEDRON_RENDER_HULL);
        g.ph->GetGraph() = G;
        leda::edge hullEdge = Triangulate(g.ph->GetGraph());
        g.ph->HideFace(hullEdge);
        g.ph->Update();
        ApplyColors(g.ph);
        return new Phase0(g);
    }

    bool Run(void) { return false; }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}