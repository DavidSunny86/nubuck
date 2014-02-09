#pragma once

#include <QObject>
#include <QMenu>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include <UI\simple_panel\simple_panel.h>

#include <LEDA\geo\d3_hull.h>

namespace OP {
namespace GEN {

class InCube : public QObject, public Operator {
    Q_OBJECT
private:
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;

    QSpinBox* _sbSize;
    QSpinBox* _sbRadius;

    int _size;
    int _radius;

    IGeometry* _bbox;
    IGeometry* _cloud;

    void CreateOperatorPanel() {
        UI::SimplePanel* panel = new UI::SimplePanel();
        panel->AddLabel("Random Points in Cube");

        _sbRadius = panel->AddSpinBox("radius", 1, 100);
        _sbRadius->setValue(_radius);
        connect(_sbRadius, SIGNAL(valueChanged(int)), this, SLOT(OnArgsChanged(int)));

        _sbSize = panel->AddSpinBox("size", 1, 10000);
        _sbSize->setValue(_size);
        connect(_sbSize, SIGNAL(valueChanged(int)), this, SLOT(OnArgsChanged(int)));

        _nb.ui->SetOperatorPanel(panel);
    }

    void UpdateBBox() {
        leda::nb::RatPolyMesh& mesh = _bbox->GetRatPolyMesh();
        mesh.clear();

        int r = _radius;
        leda::list<point3_t> L;
        L.push(point3_t(-r, -r, -r));
        L.push(point3_t(-r, -r,  r));
        L.push(point3_t( r, -r,  r));
        L.push(point3_t( r, -r, -r));
        L.push(point3_t(-r,  r, -r));
        L.push(point3_t(-r,  r,  r));
        L.push(point3_t( r,  r,  r));
        L.push(point3_t( r,  r, -r));

        leda::D3_HULL(L, mesh);
        mesh.compute_faces();

        _bbox->Update();
    }

    void UpdateCloud() {
        leda::nb::RatPolyMesh& mesh = _cloud->GetRatPolyMesh();
        mesh.clear();

        leda::list<point3_t> L;
        leda::random_d3_rat_points_in_cube(_size, _radius, L);

        leda::list_item it;
        forall_items(it, L) {
            mesh.set_position(mesh.new_node(), L[it]);
        }

        _cloud->Update();
        _nb.world->SelectGeometry(_cloud);
    }

    void Update() {
        UpdateBBox();
        UpdateCloud();
    }
public slots:
    void OnArgsChanged(int) {
        _size = _sbSize->value();
        _radius = _sbRadius->value();
        Update();
    }
public:
    InCube() : _size(5), _radius(2), _bbox(NULL), _cloud(NULL) { }

    void Register(const Nubuck& nb, Invoker& invoker) override {
        _nb = nb;

        QMenu* sceneMenu = _nb.ui->GetSceneMenu();
        QAction* action = sceneMenu->addAction("Random Points in Cube");
        QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
    }

    void Invoke() override {
        _nb.ui->SetOperatorName("Random Points in Cube");
        CreateOperatorPanel();

        _bbox = _nb.world->CreateGeometry();
        _bbox->SetRenderMode(IGeometry::RenderMode::EDGES);
        // _bbox->SetShadingMode(IGeometry::ShadingMode::FAST);
        _cloud = _nb.world->CreateGeometry();
        _cloud->SetRenderMode(IGeometry::RenderMode::NODES);

        Update();

        _nb.world->SelectGeometry(_cloud);
    }

    void Finish() override {
        _bbox->Destroy();
    }
};

} // namespace GEN
} // namespace OP