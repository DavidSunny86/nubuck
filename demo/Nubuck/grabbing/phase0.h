#include <Nubuck\nubuck.h>
#include <LEDA\geo\geo_alg.h>
#include "globals.h"

struct Phase0 : IPhase {
    typedef leda::rat_point point2_t;
    typedef leda::GRAPH<point2_t, int> graph2_t;

    Globals& g;

    Phase0(Globals& g) : g(g) { }

    void Enter(void) override { g.nb.log->printf("--- entering Phase0.\n"); }
    void Leave(void) override { g.nb.log->printf("--- leaving Phase0.\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return false; }

    StepRet Step(void) override {
        return DONE;
    }

    IPhase* NextPhase(void) override { return NULL; }

    leda::list<point2_t> ToPointList2(const graph_t& G) {
        leda::list<point2_t> L;
        leda::node n;
        forall_nodes(n, G) L.push_back(point2_t(G[n].xcoord(), G[n].ycoord()));
        return L;
    }

    void FromProjection(const graph2_t& G2, graph_t& G3) {
        leda::node_array<leda::node> nmap(G2, NULL);

        leda::node n2;
        forall_nodes(n2, G2) {
            leda::node n3 = nmap[n2] = G3.new_node();
            G3[n3] = point_t(G2[n2].xcoord(), G2[n2].ycoord(), 0);
        }

        leda::edge e2;
        forall_edges(e2, G2) {
            G3.new_edge(nmap[leda::source(e2)], nmap[leda::target(e2)]); 
        }
    }

    void OnNodesMoved(void) override {
        leda::node n;
        forall_nodes(n, g.phNodesProj->GetGraph()) {
            const point_t& p = g.phNodesProj->GetGraph()[n];
            scalar_t z = 5 + (p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord()) / 100;
            g.phNodes->GetGraph()[g.nmap[n]] = point_t(p.xcoord(), p.ycoord(), z);
        }
        Delaunay2D(g.phNodesProj->GetGraph(), g.phDelaunayProj->GetGraph());
        ConvexHull(g.phNodes->GetGraph(), g.phHull->GetGraph());
        g.phDelaunayProj->Update();
        g.phNodes->Update();
        g.phHull->Update();
        if(g.showVoronoi) {
            Voronoi2D(g.phNodesProj->GetGraph(), g.grVoronoiTri, g.phVoronoiProj->GetGraph(), g.emap);
            g.phVoronoiProj->Update();
            Colorize();
        } else {
            ConvexHull(g.phNodesProj->GetGraph(), g.phHullProj->GetGraph());
            g.phHullProj->Update();
        }
    }

    void Colorize(void) {
        leda::node n;
        forall_nodes(n, g.phNodesProj->GetGraph()) {
            leda::edge ft = leda::LOCATE_IN_TRIANGULATION(g.grVoronoiTri, point2_t(g.phNodesProj->GetGraph()[n].xcoord(), g.phNodesProj->GetGraph()[n].ycoord()));
            leda::edge f = g.emap[ft];
            Color c = g.colors[n];
            g.phVoronoiProj->SetFaceColor(f, c.r, c.g, c.b);
        }
    }

    void OnKeyPressed(char c) override {
        if('S' == c) {
            if(g.showHull) {
                g.phNodes->SetRenderFlags(0);
                g.phHull->SetRenderFlags(0);
                g.showHull = false;
            } else {
                g.phNodes->SetRenderFlags(POLYHEDRON_RENDER_NODES);
                g.phHull->SetRenderFlags(POLYHEDRON_RENDER_EDGES | POLYHEDRON_RENDER_HULL);
                g.phNodes->Update();
                g.phHull->Update();
                g.showHull = true;
            }
        }
        if('V' == c) {
            if(g.showVoronoi) {
                g.phHullProj->SetRenderFlags(POLYHEDRON_RENDER_HULL | POLYHEDRON_RENDER_EDGES);
                g.phVoronoiProj->SetRenderFlags(0);
                g.phHullProj->Update();
            } else {
                g.phHullProj->SetRenderFlags(0);
                int flags = POLYHEDRON_RENDER_HULL;
                if(g.showVoronoiEdges) flags |= POLYHEDRON_RENDER_EDGES;
                g.phVoronoiProj->SetRenderFlags(flags);
                g.phVoronoiProj->Update();
                Colorize();
            }
            g.showVoronoi = !g.showVoronoi;
        }
        if('C' == c) {
            g.showVoronoiEdges = !g.showVoronoiEdges;
            if(g.showVoronoi) {
                int flags = POLYHEDRON_RENDER_HULL;
                if(g.showVoronoiEdges) flags |= POLYHEDRON_RENDER_EDGES;
                g.phVoronoiProj->SetRenderFlags(flags);
                g.phVoronoiProj->Update();
                Colorize();
            }
        }
        if('P' == c) {
            g.showParaboloid = !g.showParaboloid;
            g.paraboloid->SetVisible(g.showParaboloid);
        }
    }
};