#include <vector>

#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase0.h"

static float ParaboloidHeightFunc(float x, float y) {
    scalar_t rat_x(x);
    scalar_t rat_y(y);
    scalar_t z = 5 + (rat_x * rat_x + rat_y * rat_y) / 100;
    return static_cast<float>(z.to_float());
}

class Algorithm : public IAlgorithm {
private:
    Globals _globals;

    float RandFloat(float min, float max) {
        const float f = 1.0f / RAND_MAX;
        float t = (rand() % RAND_MAX) * f;
        return min + t * (max - min);
    }
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t& G) override {
        Globals& g = _globals;

        // we are expected to copy these parameters
        g.nb = nubuck;

        g.showParaboloid = false;
        g.showHull = false;
        g.showVoronoi = false;
        g.showVoronoiEdges = false;

        std::vector<IWorld::PlaneDesc::Sample2>* paraboloidSamples = new std::vector<IWorld::PlaneDesc::Sample2>();

        g.phNodes           = g.nb.world->CreatePolyhedron();
        g.phHull            = g.nb.world->CreatePolyhedron();
        g.phNodesProj       = g.nb.world->CreatePolyhedron();
        g.phDelaunayProj    = g.nb.world->CreatePolyhedron();
        g.phVoronoiProj     = g.nb.world->CreatePolyhedron();
        g.phHullProj        = g.nb.world->CreatePolyhedron();

        // project all nodes on xy-plane. scale, too
        float scale = 1000.0f;
        g.nb.log->printf("Scaling points by factor %f.\n", scale);
        g.nmap.init(g.phNodesProj->GetGraph());
        leda::node n;
        forall_nodes(n, G) {
            const point_t& p = G[n];
            scalar_t z = 5 + (p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord()) / 100;
            leda::node n0 = g.phNodesProj->GetGraph().new_node();
            leda::node n1 = g.phNodes->GetGraph().new_node();
            g.nmap[n0] = n1;
            g.phNodesProj->GetGraph()[n0] = point_t(p.xcoord(), p.ycoord(), 0);
            g.phNodes->GetGraph()[n1] = point_t(p.xcoord(), p.ycoord(), z);
            float z2 = static_cast<float>(z.to_float());
            printf("z2 = %f\n", z2);

            IWorld::PlaneDesc::Sample2 s;
            s.x = static_cast<float>(g.phNodesProj->GetGraph()[n0].xcoord().to_float());
            s.y = static_cast<float>(g.phNodesProj->GetGraph()[n0].ycoord().to_float());
            paraboloidSamples->push_back(s);
        }

        IWorld::PlaneDesc desc;
        desc.heightFunc = ParaboloidHeightFunc;
        desc.addSamples = &(*paraboloidSamples)[0];
        desc.numAddSamples = paraboloidSamples->size();
        desc.size = 200.0f;
        desc.subdiv = 2;
        desc.flip = true;

        g.paraboloid = g.nb.world->CreatePlaneMesh(desc);
        g.paraboloid->SetEffect("LitDirectionalTransparent");
        g.paraboloid->SetVisible(g.showParaboloid);

        IWorld::SphereDesc sphereDesc;
        sphereDesc.numSubdiv = 4;
        sphereDesc.smooth = true;
        g.decal = g.nb.world->CreateSphereMesh(sphereDesc);
        g.decal->SetEffect("StencilDecal");
        g.overlay = g.nb.world->CreateSphereMesh(sphereDesc);
        g.overlay->SetEffect("StencilOverlay");

        Delaunay2D(g.phNodesProj->GetGraph(), g.phDelaunayProj->GetGraph());
        Voronoi2D(g.phNodesProj->GetGraph(), g.grVoronoiTri, g.phVoronoiProj->GetGraph(), g.emap);
        ConvexHull(g.phNodesProj->GetGraph(), g.phHullProj->GetGraph());
        ConvexHull(g.phNodes->GetGraph(), g.phHull->GetGraph());

        g.phNodesProj->SetName("Nodes (Projection)");
        g.phNodesProj->SetRenderFlags(POLYHEDRON_RENDER_NODES);
        g.phNodesProj->SetPickable(true);

        g.phDelaunayProj->SetName("Delaunay Triangulation");
        g.phDelaunayProj->SetRenderFlags(POLYHEDRON_RENDER_EDGES);

        g.phHullProj->SetName("Convex Hull (Projection)");
        g.phHullProj->SetRenderFlags(POLYHEDRON_RENDER_HULL | POLYHEDRON_RENDER_EDGES);
        g.phHullProj->SetEffect("LitDirectionalStencilFill");
        g.phHullProj->Update();
        
        g.phVoronoiProj->SetName("Voronoi Diagram");
        // g.phVoronoiProj->SetRenderFlags(POLYHEDRON_RENDER_HULL);
        // g.phVoronoiProj->Update();

        float f = 1.0f / 255.0f;
        float r2 = f * 176;
        float g2 = f * 196;
        float b2 = f * 222;
        g.colors.init(g.phNodesProj->GetGraph());
        forall_nodes(n, g.phNodesProj->GetGraph()) {
            float cr = (RandFloat(0.5f, 1.0f) + r2) * 0.5f;
            float cg = (RandFloat(0.5f, 1.0f) + g2) * 0.5f;
            float cb = (RandFloat(0.5f, 1.0f) + b2) * 0.5f;
            Color c;
            c.r = cr;
            c.g = cg;
            c.b = cb;
            g.colors[n] = c;
        }

        g.phNodes->SetName("Nodes");

        g.phHull->SetName("Convex Hull");

        g.phNodes->Update();
        g.phHull->Update();
        g.phNodesProj->Update();
        g.phDelaunayProj->Update();
        g.phVoronoiProj ->Update();
        g.phHull->Update();

        return new Phase0(g);
    }

    bool Run(void) override { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}