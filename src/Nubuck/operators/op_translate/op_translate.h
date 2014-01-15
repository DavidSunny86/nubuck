#pragma once

#include <vector>

#include <QMenu>

#include <Nubuck\nubuck.h>
#include <operators\operator.h>
#include <operators\operators.h>
#include <renderer\mesh\mesh.h>

#include <LEDA\geo\d3_hull.h>

namespace OP {

class Translate : public Operator {
private:
    typedef leda::rational scalar_t;
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;

    IGeometry* _geom_axis;

    enum { X = 0, Y, Z };
    IGeometry* _geom_arrowHeads[3];

    void BuildAxis() {
        _geom_axis = _nb.world->CreateGeometry();
        _geom_axis->SetRenderMode(IGeometry::RenderMode::EDGES);
        leda::nb::RatPolyMesh& mesh = _geom_axis->GetRatPolyMesh();
        leda::node v0 = mesh.new_node();
        leda::node vX = mesh.new_node();
        leda::node vY = mesh.new_node();
        leda::node vZ = mesh.new_node();
        mesh.set_position(v0, point3_t(0, 0, 0));
        mesh.set_position(vX, point3_t(1, 0, 0));
        mesh.set_position(vY, point3_t(0, 1, 0));
        mesh.set_position(vZ, point3_t(0, 0, 1));
        mesh.set_reversal(mesh.new_edge(v0, vX), mesh.new_edge(vX, v0));
        mesh.set_reversal(mesh.new_edge(v0, vY), mesh.new_edge(vY, v0));
        mesh.set_reversal(mesh.new_edge(v0, vZ), mesh.new_edge(vZ, v0));
        _geom_axis->Update();
    }

    void BuildArrowHead() {
        leda::nb::RatPolyMesh mesh;
        scalar_t s = scalar_t(1) / scalar_t(10);
        leda::list<point3_t> L;
        L.push(point3_t( 0,  2 * s,  0));
        L.push(point3_t(-s, -s, -s));
        L.push(point3_t(-s, -s,  s));
        L.push(point3_t( s, -s,  s));
        L.push(point3_t( s, -s, -s));
        leda::CONVEX_HULL(L, mesh);
        mesh.compute_faces();

        float off = s.to_float() + 1.0f;

        for(int i = 0; i < 3; ++i) {
            _geom_arrowHeads[i] = _nb.world->CreateGeometry();
            _geom_arrowHeads[i]->GetRatPolyMesh() = mesh;
            _geom_arrowHeads[i]->GetRatPolyMesh().compute_faces();
            _geom_arrowHeads[i]->SetRenderMode(IGeometry::RenderMode::FACES);
        }

        leda::nb::set_color(_geom_arrowHeads[X]->GetRatPolyMesh(), R::Color::Red);
        _geom_arrowHeads[X]->Rotate(-90.0f, 0.0f, 0.0f, 1.0f);
        _geom_arrowHeads[X]->SetPosition(off, 0.0f, 0.0f);

        leda::nb::set_color(_geom_arrowHeads[Y]->GetRatPolyMesh(), R::Color::Blue);
        _geom_arrowHeads[Y]->SetPosition(0.0f, off, 0.0f);

        leda::nb::set_color(_geom_arrowHeads[Z]->GetRatPolyMesh(), R::Color::Green);
        _geom_arrowHeads[Z]->Rotate(90.0f, 1.0f, 0.0f, 0.0f);
        _geom_arrowHeads[Z]->SetPosition(0.0f, 0.0f, off);

        for(int i = 0; i < 3; ++i) _geom_arrowHeads[i]->Update();
    }
public:
    void Register(const Nubuck& nb, Invoker& invoker) override {
        _nb = nb;

        QAction* action = _nb.ui->GetSceneMenu()->addAction("Translate");
        QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));

        BuildAxis();
        BuildArrowHead();
    }

    void Invoke() override {
    }

    void Finish() override {
    }
};

} // namespace OP