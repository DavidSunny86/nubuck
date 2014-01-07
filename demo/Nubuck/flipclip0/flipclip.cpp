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
    Nubuck                  nb;
    IPolyhedron*            ph;
    leda::edge              hullEdge;
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

// Phase1: flipping
struct Phase1 : IPhase {
    enum State {
        STATE_SELECT = 0,
        STATE_FLIP
    };

    Globals&                g;
    leda::list<leda::edge>  S;
    State                   state;
    leda::edge              selEdge;

    Phase1(Globals& g) : g(g) { 
        state = STATE_SELECT;
    }

    void Enter(void) override { 
        g.nb.log->printf("--- Entering Phase1 (Flipping) \n"); 
    
        const graph3_t& G = g.ph->GetGraph();
        leda::edge e;
        forall_edges(e, G)
            if(G[e] == RED) S.append(e);
        g.nb.log->printf("collected red edges, |S| = %d\n", S.size());
    }

    void Leave(void) override { g.nb.log->printf("--- Leaving Phase1\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return true; }

    StepRet StepSelect(void) {
        graph3_t& G = g.ph->GetGraph();
        if(!S.empty()) {
            selEdge = S.pop();

            // highlight adjacent faces
            leda::edge f;
            forall_edges(f, G) {
                g.ph->HideFace(f);
            }
            leda::edge e = selEdge;
            leda::edge r = G.reversal(e);
            if(G[e] != BLUE && G[r] != BLUE) {
                g.ph->ShowFace(e);
                g.ph->ShowFace(r);
            }
            leda::node n;
            forall_nodes(n, g.ph->GetGraph()) {
                g.ph->SetNodeColor(n, 0.0f, 0.0f, 0.0f);
            }
            g.ph->SetNodeColor(G.source(e), 1.0f, 1.0f, 1.0f);
            g.ph->SetNodeColor(G.target(e), 1.0f, 1.0f, 1.0f);

            state = STATE_FLIP;
            return CONTINUE;
        }
        else {
            g.nb.log->printf("S is empty => done flipping\n");

            leda::edge e;
            forall_edges(e, g.ph->GetGraph())
                g.ph->ShowFace(e);
            g.ph->HideFace(g.hullEdge);

            return DONE;
        }
    }

    StepRet StepFlip(void) {
        state = STATE_SELECT;
        graph3_t& G = g.ph->GetGraph();
        // leda::edge e = S.pop();
        leda::edge e = selEdge;
        leda::edge r = G.reversal(e);

        if (G[e] == BLUE) {
            g.nb.log->printf("skipping blue edge\n");
            return CONTINUE;
        }

        leda::edge e1 = G.face_cycle_succ(r);
        leda::edge e3 = G.face_cycle_succ(e);

        point3_t a = G[source(e1)];
        point3_t b = G[target(e1)];
        point3_t c = G[source(e3)];
        point3_t d = G[target(e3)];


        if (orientation(a,b,c,d) <= 0) 
          G[e] = G[r] = BLACK;
        else
          { G[e] = G[r] = RED;
            int orient_bda = orientation_xy(b,d,a);
            int orient_bdc = orientation_xy(b,d,c);
            if (orient_bda != orient_bdc
            && (orient_bda != 0 || G.outdeg(source(e1)) == 4 || G[e1] == BLUE)
            && (orient_bdc != 0 || G.outdeg(source(e3)) == 4 || G[e3] == BLUE))
              { 
                leda::edge e2 = G.face_cycle_succ(e1);
                leda::edge e4 = G.face_cycle_succ(e3);
          
                S.push(e1); 
                S.push(e2); 
                S.push(e3); 
                S.push(e4); 
            
                // flip
                G.move_edge(e,e2,source(e4));
                G.move_edge(r,e4,source(e2));
                if ((orient_bda==0 && G[e1]==BLUE)||(orient_bdc==0 && G[e3]==BLUE))
                   G[e] = G[r] = BLUE;
                else
                   G[e] = G[r] = BLACK;
                // flip_count++;
              }
          }
        g.ph->Update();
        ApplyColors(g.ph);
        return CONTINUE;
    }

    StepRet Step(void) override {
        g.nb.log->printf("flipping\n");
        if(STATE_SELECT == state) return StepSelect();
        else return StepFlip();
    }

    IPhase* NextPhase(void) override { return NULL; }

    void OnNodesMoved(void) override { }
    void OnKeyPressed(char) override { }
};

// Phase0: initial triangulation
struct Phase0 : IPhase {
    Globals& g;

    Phase0(Globals& g) : g(g) { }

    void Enter(void) override { g.nb.log->printf("--- Entering Phase0 (Triangulation)\n"); }
    void Leave(void) override { g.nb.log->printf("--- Leaving Phase0\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return true; }

    StepRet Step(void) override {
        g.nb.log->printf("compute arbitrary triangulation\n");

        g.hullEdge = Triangulate(g.ph->GetGraph());
        g.ph->Update();
        g.ph->HideFace(g.hullEdge);
        g.ph->Update();
        ApplyColors(g.ph);

        return DONE;
    }

    IPhase* NextPhase(void) override {
        return new Phase1(g);
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
        g.ph->Update();

        // REMOVEME
        IGeometry* geom = nb.world->CreateGeometry();
        NB::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        size_t h = mesh.MakeTetrahedron(
            point3_t(-1, -1,  1),
            point3_t( 1, -1,  1),
            point3_t( 0, -1, -1),
            point3_t( 0,  1,  0));
        size_t h0 = mesh.SplitHalfEdge(h);
        size_t h1 = mesh.H_FacePred(mesh.H_Reversal(h0));
        size_t h2 = mesh.H_FaceSucc(mesh.H_Reversal(h0));
        mesh.V_SetPosition(mesh.H_StartVertex(h0), point3_t(0, -1, 1));
        size_t f0 = mesh.SplitFace(h1, h2);
        size_t f1 = mesh.H_Reversal(f0);
        mesh.DeleteFace(mesh.H_Face(f0));
        mesh.DeleteFace(mesh.H_Face(f1));

        geom->Update();

        return new Phase0(g);
    }

    bool Run(void) { return false; }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}