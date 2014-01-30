#pragma once

#include <QObject>
#include <QMenu>

#include <Nubuck\nubuck.h>
#include <operators\operator.h>
#include <operators\operators.h>

#include <LEDA\geo\d3_hull.h>

namespace OP {

class ConvexHull : public Operator {
private:
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override {
        _nb = nb;

        QAction* action = _nb.ui->GetObjectMenu()->addAction("Convex Hull");
        QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
    }

    void Invoke() override {
        _nb.ui->SetOperatorName("Convex Hull");

        IGeometry* cloud = ((W::World*)_nb.world)->GetSelection().GetGeometryList().front();
        assert(cloud);

        leda::nb::RatPolyMesh& cloudMesh = cloud->GetRatPolyMesh();
        leda::list<point3_t> L;
        leda::node v;
        forall_nodes(v, cloudMesh) L.push_back(cloudMesh.position_of(v));

        IGeometry* chull = _nb.world->CreateGeometry();
        chull->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
        leda::nb::RatPolyMesh& chullMesh = chull->GetRatPolyMesh();
        leda::CONVEX_HULL(L, chullMesh);
        chullMesh.compute_faces();
        chull->Update();

        float x = cloud->GetPosX();
        float y = cloud->GetPosY();
        float z = cloud->GetPosZ();
        chull->SetPosition(x, y, z);

        cloud->Destroy();
        _nb.world->SelectGeometry(chull);
    }

    void Finish() override {
    }
};

} // namespace OP