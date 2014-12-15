#pragma once

#include <QCheckBox>
#include <QSlider>

#include <Nubuck\events\events.h>
#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

class NBW_SpinBox;

extern EV::ConcreteEventDef<EV::Event> ev_toggleParaboloid;
extern EV::ConcreteEventDef<EV::Event> ev_toggleConvexHull;
extern EV::ConcreteEventDef<EV::Arg<float> > ev_setConvexHullScale;

class D2_Delaunay_BruteForce_Panel : public OP::ALG::StandardAlgorithmPanel {
    Q_OBJECT
private:
    QCheckBox*      _btnToggleParaboloid;
    QCheckBox*  	_btnToggleConvexHull;
    NBW_SpinBox*    _sbConvexHullScale;
private slots:
    void OnToggleParaboloid();
    void OnToggleConvexHull();
    void OnConvexHullScaleChanged(leda::rational value);
public:
    D2_Delaunay_BruteForce_Panel();

    void Invoke() override;
};

class D2_Delaunay_BruteForce : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;

    bool _isParaboloidVisible;
    bool _isConvexHullVisible;

    void Event_ToggleParaboloid(const EV::Event& event);
    void Event_ToggleConvexHull(const EV::Event& event);
    void Event_SetConvexHullScale(const EV::Arg<float>& event);
protected:
    const char*     GetName() const override;
    OP::ALG::Phase* Init() override;
public:
    D2_Delaunay_BruteForce();
};