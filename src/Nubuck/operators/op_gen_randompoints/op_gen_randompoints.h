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

BEGIN_EVENT_DEF(OP_RandomPoints_Update)
    int domain;
    int size;
    int radius;
END_EVENT_DEF

namespace OP {
namespace GEN {

class RandomPointsPanel : public UI::SimplePanel {
    Q_OBJECT
private:
    QComboBox*  _cbDomain;
    QSpinBox*   _sbSize;
    QSpinBox*   _sbRadius;
public slots:
    void OnArgsChanged(int);
public:
    RandomPointsPanel(QWidget* parent = NULL);
};

class RandomPoints : public Operator {
public:
    struct Domain {
        enum Enum {
            IN_BALL = 0,
            IN_CUBE,
            IN_HEMISPHERE,
            ON_SPHERE,
            ON_HEMISPHERE,
            ON_PARABOLOID
        };
    };
private:
    typedef leda::d3_rat_point point3_t;

    leda::nb::RatPolyMesh   _spherePrefab;
    leda::nb::RatPolyMesh   _hemispherePrefab;

    Nubuck _nb;

    IGeometry* _hull;
    IGeometry* _cloud;

    Domain::Enum    _lastDomain;
    int             _lastSize;
    int             _lastRadius;

    void UpdateHull(Domain::Enum domain, int radius);
    void UpdateCloud(Domain::Enum domain, int size, int radius);

    void Event_Update(const EV::Event& event);
public:
    enum {
        DEFAULT_DOMAIN  = Domain::IN_BALL,
        DEFAULT_SIZE    = 5,
        DEFAULT_RADIUS  = 2
    };

    RandomPoints();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override;
};

} // namespace GEN
} // namespace OP