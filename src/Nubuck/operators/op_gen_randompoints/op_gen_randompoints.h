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
    int     domain;
    int     size;
    int     radius;
    bool    save;
END_EVENT_DEF

class NBW_SpinBox;

namespace OP {
namespace GEN {

class RandomPointsPanel : public UI::SimplePanel {
    Q_OBJECT
private:
    QComboBox*      _cbDomain;
    NBW_SpinBox*    _sbSize;
    NBW_SpinBox*    _sbRadius;
    QCheckBox*      _cbSave;

    void _OnArgsChanged();
public slots:
    void OnArgsChanged(leda::rational);
    void OnArgsChanged(int);
    void OnArgsChanged(bool);
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
            ON_PARABOLOID,
            IN_DISC
        };
    };
private:
    typedef leda::d3_rat_point point3_t;

    leda::nb::RatPolyMesh   _spherePrefab;
    leda::nb::RatPolyMesh   _hemispherePrefab;
    leda::nb::RatPolyMesh   _discPrefab;

    Nubuck _nb;

    IGeometry* _hull;
    IGeometry* _cloud;
    IGeometry* _cloudCopy;

    Domain::Enum    _lastDomain;
    int             _lastSize;
    int             _lastRadius;
    bool            _lastSave;

    void UpdateHull(Domain::Enum domain, int radius);
    void UpdateCloud(Domain::Enum domain, int size, int radius);

    void Event_Update(const EV::Event& event);
public:
    enum {
        DEFAULT_DOMAIN  = Domain::IN_BALL,
        DEFAULT_SIZE    = 5,
        DEFAULT_RADIUS  = 2,
        DEFAULT_SAVE    = 1
    };

    RandomPoints();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override;
};

} // namespace GEN
} // namespace OP