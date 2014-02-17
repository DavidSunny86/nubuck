#pragma once

#include <QObject>
#include <QMenu>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <UI\simple_panel\simple_panel.h>
#include <operators\operators.h>

#include <LEDA\geo\d3_hull.h>

BEGIN_EVENT_DEF(OP_InCube_Update)
    int size;
    int radius;
END_EVENT_DEF

namespace OP {
namespace GEN {

class InCubePanel : public UI::SimplePanel {
    Q_OBJECT
private:
    QSpinBox* _sbSize;
    QSpinBox* _sbRadius;
public slots:
    void OnArgsChanged(int);
public:
    InCubePanel(QWidget* parent = NULL);
};

class InCube : public Operator {
private:
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;

    IGeometry* _bbox;
    IGeometry* _cloud;

    void UpdateBBox(int radius);
    void UpdateCloud(int size, int radius);
    void Update(int size, int radius);

    void Event_Update(const EV::Event& event);
public:
    InCube();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override;
};

} // namespace GEN
} // namespace OP