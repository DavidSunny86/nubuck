#pragma once

#include <QSlider>
#include <QCheckBox>

#include "globals.h"
#include "phase_init.h"

extern EV::ConcreteEventDef<EV::Arg<float> > ev_distanceChanged;
extern EV::ConcreteEventDef<EV::Arg<bool> > ev_runConfChanged; // arg = haltBeforeStitching

class FlipClipPanel : public OP::ALG::StandardAlgorithmPanel {
    Q_OBJECT
private:
    QSlider* _dist;
    QCheckBox* _haltBeforeStitching;
private slots:
    void OnDistanceChanged(int value);
    void OnHaltBeforeStitchingToggled(bool isChecked);
public:
    enum { MAX_DISTANCE = 500 };

    FlipClipPanel();

    void Invoke() override;
};

class FlipClip : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;

    void Event_DistanceChanged(const EV::Arg<float>& event);
    void Event_RunConfChanged(const EV::Arg<bool>& event);
protected:
    const char* GetName() const;

    OP::ALG::Phase* Init();
public:
    FlipClip();
};