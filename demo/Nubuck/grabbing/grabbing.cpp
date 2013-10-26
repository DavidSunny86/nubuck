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
        // we are expected to copy these parameters
        _globals.nb = nubuck;

        _globals.showParaboloid = false;
        _globals.showHull = false;
        _globals.showVoronoi = false;
        _globals.showVoronoiEdges = false;

        std::vector<IWorld::PlaneDesc::Sample2>* paraboloidSamples = new std::vector<IWorld::PlaneDesc::Sample2>();

        // project all nodes on xy-plane. scale, too
        _globals.nb.log->printf("Scaling points by factor %f.\n", 1000);
        _globals.nmap.init(_globals.grNodesProj);
        leda::node n;
        forall_nodes(n, G) {
            const point_t& p = G[n];
            scalar_t z = 5 + (p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord()) / 100;
            leda::node n0 = _globals.grNodesProj.new_node();
            leda::node n1 = _globals.grNodes.new_node();
            _globals.nmap[n0] = n1;
            _globals.grNodesProj[n0] = point_t(p.xcoord(), p.ycoord(), 0);
            _globals.grNodes[n1] = point_t(p.xcoord(), p.ycoord(), z);
            float z2 = static_cast<float>(z.to_float());
            printf("z2 = %f\n", z2);

            IWorld::PlaneDesc::Sample2 s;
            s.x = static_cast<float>(_globals.grNodesProj[n0].xcoord().to_float());
            s.y = static_cast<float>(_globals.grNodesProj[n0].ycoord().to_float());
            paraboloidSamples->push_back(s);
        }

        IWorld::PlaneDesc desc;
        desc.heightFunc = ParaboloidHeightFunc;
        desc.addSamples = &(*paraboloidSamples)[0];
        desc.numAddSamples = paraboloidSamples->size();
        desc.size = 200.0f;
        desc.subdiv = 2;
        desc.flip = true;

        _globals.paraboloid = _globals.nb.world->CreatePlaneMesh(desc);
        _globals.paraboloid->SetVisible(_globals.showParaboloid);

        Delaunay2D(_globals.grNodesProj, _globals.grDelaunayProj);
        Voronoi2D(_globals.grNodesProj, _globals.grVoronoiTri, _globals.grVoronoiProj, _globals.emap);
        ConvexHull(_globals.grNodesProj, _globals.grHullProj);
        ConvexHull(_globals.grNodes, _globals.grHull);

        _globals.phNodesProj = _globals.nb.world->CreatePolyhedron(_globals.grNodesProj);
        _globals.phNodesProj->SetRenderFlags(POLYHEDRON_RENDER_NODES);
        _globals.phNodesProj->SetPickable(true);

        _globals.phDelaunayProj = _globals.nb.world->CreatePolyhedron(_globals.grDelaunayProj);
        _globals.phDelaunayProj->SetRenderFlags(POLYHEDRON_RENDER_EDGES);

        _globals.phHullProj = _globals.nb.world->CreatePolyhedron(_globals.grHullProj);
        _globals.phHullProj->SetRenderFlags(POLYHEDRON_RENDER_HULL | POLYHEDRON_RENDER_EDGES);
        _globals.phHullProj->Update();
        
        _globals.phVoronoiProj = _globals.nb.world->CreatePolyhedron(_globals.grVoronoiProj);
        // _globals.phVoronoiProj->SetRenderFlags(POLYHEDRON_RENDER_HULL);
        // _globals.phVoronoiProj->Update();

        float f = 1.0f / 255.0f;
        float r2 = f * 176;
        float g2 = f * 196;
        float b2 = f * 222;
        _globals.colors.init(_globals.grNodesProj);
        forall_nodes(n, _globals.grNodesProj) {
            float r = (RandFloat(0.5f, 1.0f) + r2) * 0.5f;
            float g = (RandFloat(0.5f, 1.0f) + g2) * 0.5f;
            float b = (RandFloat(0.5f, 1.0f) + b2) * 0.5f;
            Color c;
            c.r = r;
            c.g = g;
            c.b = b;
            _globals.colors[n] = c;
        }

        _globals.phNodes = _globals.nb.world->CreatePolyhedron(_globals.grNodes);

        _globals.phHull = _globals.nb.world->CreatePolyhedron(_globals.grHull);
        _globals.phHull->Update();

        return new Phase0(_globals);
    }

    bool Run(void) override { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}