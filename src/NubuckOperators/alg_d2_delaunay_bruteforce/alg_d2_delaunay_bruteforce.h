#pragma once

#include <QCheckBox>
#include <QSlider>

#include <Nubuck\events\events.h>
#include <Nubuck\operators\standard_algorithm.h>
#include "globals.h"

BEGIN_EVENT_DEF_CS(ToggleParaboloid)
END_EVENT_DEF_CS

BEGIN_EVENT_DEF_CS(ToggleConvexHull)
END_EVENT_DEF_CS

BEGIN_EVENT_DEF_CS(SetConvexHullScale)
    float scale;
END_EVENT_DEF_CS

class D2_Delaunay_BruteForce_Panel : public OP::ALG::StandardAlgorithmPanel {
    Q_OBJECT
private:
    QCheckBox*  _btnToggleParaboloid;
    QCheckBox*  _btnToggleConvexHull;
    QSlider*    _sldConvexHullScale;
private slots:
    void OnToggleParaboloid();
    void OnToggleConvexHull();
    void OnConvexHullScaleChanged(int value);
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
    void Event_SetConvexHullScale(const EV::Event& event);
protected:
    const char*     GetName() const override;
    OP::ALG::Phase* Init() override;
public:
    D2_Delaunay_BruteForce();
};