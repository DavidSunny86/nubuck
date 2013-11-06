#include <LEDA\geo\d3_hull.h>
#include <LEDA\geo\circle.h>
#include <Nubuck\nubuck.h>

typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

struct Globals {
    Nubuck nb;

    IPolyhedron*    polyhedron;
    leda::node      c[3]; // define circle

    IMesh*          cylinder;
};

static leda::point ProjectXY(const leda::d3_rat_point& p) {
    return leda::rat_point(p.xcoord(), p.ycoord()).to_float();
}

static leda::circle CircleXY(const point_t& p0, const point_t& p1, const point_t& p2) {
    return leda::circle(ProjectXY(p0), ProjectXY(p1), ProjectXY(p2));
}

class Phase0 : public IPhase {
private:
    Globals _g;
public:
    Phase0(const Globals& g) : _g(g) { }

    void Enter(void) override { }
    void Leave(void) override { }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return true; }

    StepRet Step(void) override { return CONTINUE; }
    IPhase* NextPhase(void) override { return NULL; }

    void OnNodesMoved(void) override {
        graph_t& G = _g.polyhedron->GetGraph();
        leda::circle circle = CircleXY(G[_g.c[0]], G[_g.c[1]], G[_g.c[2]]);
        if(circle.is_degenerate()) {
            _g.cylinder->SetVisible(false);
        }
        else {
            _g.cylinder->SetVisible(true);
            leda::point center = circle.center();
            _g.cylinder->SetPosition((float)center.xcoord(), (float)center.ycoord(), 2.5f);
            _g.cylinder->SetScale((float)circle.radius(), (float)circle.radius(), 1.0f);
        }
    }

    void OnKeyPressed(char c) override { }
};

class Algorithm : public IAlgorithm {
private:
    Globals _g;
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t&) override {
        _g.nb = nubuck;

        _g.polyhedron = _g.nb.world->CreatePolyhedron();
        _g.polyhedron->SetRenderFlags(POLYHEDRON_RENDER_NODES);
        _g.polyhedron->SetPickable(true);
        graph_t& G = _g.polyhedron->GetGraph();
        for(int i = 0; i < 3; ++i) _g.c[i] = G.new_node();
        G[_g.c[0]] = point_t(-1, -1, 0);
        G[_g.c[1]] = point_t( 1, -1, 0);
        G[_g.c[2]] = point_t( 0,  1, 0);
        _g.polyhedron->Update();

        IWorld::CylinderDesc cylinderDesc;
        cylinderDesc.radius     = 1.0f;
        cylinderDesc.height 	= 5.0f;
        cylinderDesc.numSlices  = 64;
        cylinderDesc.caps       = true;
        _g.cylinder = _g.nb.world->CreateCylinderMesh(cylinderDesc);

        leda::circle circle = CircleXY(G[_g.c[0]], G[_g.c[1]], G[_g.c[2]]);
        leda::point center = circle.center();
        _g.cylinder->SetPosition((float)center.xcoord(), (float)center.ycoord(), 2.5f);
        _g.cylinder->SetScale((float)circle.radius(), (float)circle.radius(), 1.0f);

        return new Phase0(_g);
    }

    bool Run(void) override { return false; }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}