#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>

#include "ui_op_delaunay3d.h"

namespace OP {

class Delaunay3DPanel : public OperatorPanel {
    Q_OBJECT
private:
    Ui::Delaunay3D _ui;
private slots:
    void OnScaleChanged(leda::rational value);
public:
    Delaunay3DPanel(QWidget* parent = NULL);

    void Invoke() override;
};

class Delaunay3D : public Operator {
private:
    struct Simplex {
        leda::node          verts[4];
        leda::rat_vector    localPos[4];
        leda::rat_vector    center;
    };
    nb::geometry              geom;
    std::vector<Simplex> _simplices;

    void Event_SetScale(const EV::Arg<double>& event);
public:
    Delaunay3D();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP