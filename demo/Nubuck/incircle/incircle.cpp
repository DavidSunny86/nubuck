#include <LEDA\geo\d3_hull.h>
#include <LEDA\geo\circle.h>
#include <Nubuck\nubuck.h>

typedef leda::rational              scalar_t;
typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

struct Globals {
    Nubuck nb;

    IPolyhedron*    ph2;
    IPolyhedron*    ph3;
    leda::node      c2[3];
    leda::node      c3[3];

    IMesh*          cylinder;
    IMesh*          wireframe;
    IMesh*          plane;
    IMesh*          ground;
};

static float Zero(float, float) { return 0.0f; }

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
        graph_t& G2 = _g.ph2->GetGraph();
        leda::circle circle = CircleXY(G2[_g.c2[0]], G2[_g.c2[1]], G2[_g.c2[2]]);
        if(circle.is_degenerate()) {
            _g.cylinder->SetVisible(false);
        }
        else {
            _g.cylinder->SetVisible(true); // !!!
            leda::point center = circle.center();
            // _g.cylinder->SetPosition((float)center.xcoord(), (float)center.ycoord(), 2.5f);
            // _g.cylinder->SetScale((float)circle.radius(), (float)circle.radius(), 1.0f);
            _g.cylinder->SetPosition((float)center.xcoord(), (float)center.ycoord(), 0.0f);
            _g.cylinder->SetScale((float)circle.radius(), (float)circle.radius(), (float)circle.radius());
            _g.wireframe->SetPosition((float)center.xcoord(), (float)center.ycoord(), 0.0f);
            _g.wireframe->SetScale((float)circle.radius(), (float)circle.radius(), (float)circle.radius());
        }

        graph_t& G3 = _g.ph3->GetGraph();
        for(int i = 0; i < 3; ++i) {
            scalar_t x = G2[_g.c2[i]].xcoord();
            scalar_t y = G2[_g.c2[i]].ycoord();
            scalar_t z = 2 + (x * x + y * y) / 10;
            G3[_g.c3[i]] = point_t(x, y, z);
        }
        _g.ph3->Update();

        /*
        leda::vector v0 = (G3[_g.c3[2]].to_vector() - G3[_g.c3[0]].to_vector()).to_vector();
        leda::vector v1 = (G3[_g.c3[1]].to_vector() - G3[_g.c3[0]].to_vector()).to_vector();
        leda::vector normal = leda::cross_product(v1, v0);
        _g.plane->AlignZ(normal.xcoord(), normal.ycoord(), normal.zcoord());
        _g.plane->SetOrient(v1.xcoord(), v1.ycoord(), v1.zcoord(), normal.xcoord(), normal.ycoord(), normal.zcoord());
        _g.plane->SetPosition(G3[_g.c3[0]].xcoord().to_float(), G3[_g.c3[0]].ycoord().to_float(), G3[_g.c3[0]].zcoord().to_float());
        */
    }

    void OnKeyPressed(char c) override { 
        if('a' == c || 'A' == c) _g.wireframe->SetVisible(true);
        if('b' == c || 'B' == c) _g.wireframe->SetVisible(false);
    }
};

class Algorithm : public IAlgorithm {
private:
    Globals _g;
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t&) override {
        _g.nb = nubuck;

        _g.ph2 = _g.nb.world->CreatePolyhedron();
        _g.ph2->SetRenderFlags(POLYHEDRON_RENDER_NODES);
        _g.ph2->SetPickable(true);
        graph_t& G2 = _g.ph2->GetGraph();
        for(int i = 0; i < 3; ++i) _g.c2[i] = G2.new_node();
        G2[_g.c2[0]] = point_t(-1, -1, 0);
        G2[_g.c2[1]] = point_t( 1, -1, 0);
        G2[_g.c2[2]] = point_t( 0,  1, 0);
        _g.ph2->Update();

        _g.ph3 = _g.nb.world->CreatePolyhedron();
        // _g.ph3->SetRenderFlags(POLYHEDRON_RENDER_NODES);
        graph_t& G3 = _g.ph3->GetGraph();
        for(int i = 0; i < 3; ++i) {
            scalar_t x = G2[_g.c2[i]].xcoord();
            scalar_t y = G2[_g.c2[i]].ycoord();
            scalar_t z = 2 + (x * x + y * y) / 10;
            _g.c3[i] = G3.new_node();
            G3[_g.c3[i]] = point_t(x, y, z);
        }
        _g.ph3->Update();

        /*
        IWorld::CylinderDesc cylinderDesc;
        cylinderDesc.radius     = 1.0f;
        cylinderDesc.height 	= 50.0f;
        cylinderDesc.numSlices  = 1024;
        cylinderDesc.caps       = true;
        _g.cylinder = _g.nb.world->CreateCylinderMesh(cylinderDesc);
        _g.cylinder->SetEffect("StencilDecal");
        */
        IWorld::SphereDesc sphereDesc;
        sphereDesc.numSubdiv = 4;
        sphereDesc.smooth = true;
        _g.cylinder = _g.nb.world->CreateSphereMesh(sphereDesc);
        _g.cylinder->SetEffect("StencilDecal");
        _g.wireframe = _g.nb.world->CreateSphereMesh(sphereDesc);
        _g.wireframe->SetEffect("GenericWireframe");

        leda::circle circle = CircleXY(G2[_g.c2[0]], G2[_g.c2[1]], G2[_g.c2[2]]);
        leda::point center = circle.center();
        _g.cylinder->SetPosition((float)center.xcoord(), (float)center.ycoord(), 2.5f);
        _g.cylinder->SetScale((float)circle.radius(), (float)circle.radius(), 1.0f);

        IWorld::PlaneDesc planeDesc;
        planeDesc.size = 100;
        planeDesc.subdiv = 0;
        planeDesc.heightFunc = Zero;
        planeDesc.numAddSamples = 0;
        planeDesc.addSamples = NULL;
        planeDesc.flip = false;
        _g.plane = _g.nb.world->CreatePlaneMesh(planeDesc);
        _g.plane->SetEffect("LitDirectional");

        _g.ground = _g.nb.world->CreatePlaneMesh(planeDesc);
        _g.ground->SetEffect("StencilOverlayFS");

        return new Phase0(_g);
    }

    bool Run(void) override { return false; }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}