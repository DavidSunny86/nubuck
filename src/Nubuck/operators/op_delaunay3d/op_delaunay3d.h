#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>

#include "ui_op_delaunay3d.h"

BEGIN_EVENT_DEF(OP_Delaunay3D_SetScale)
    int value;
    int max;
END_EVENT_DEF

namespace OP {

class Delaunay3DPanel : public OperatorPanel {
    Q_OBJECT
private:
    Ui::Delaunay3D _ui;
private slots:
    void OnScaleChanged(int value);
public:
    Delaunay3DPanel(QWidget* parent = NULL);

    void Invoke() override;
};

class Delaunay3D : public Operator {
private:
    struct Simplex {
        IGeometry*          geom;
        leda::rat_vector    center;
    };
    std::vector<Simplex> _simplices;

    Nubuck  _nb;

    void Event_SetScale(const EV::Event& event);
public:
    Delaunay3D();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP