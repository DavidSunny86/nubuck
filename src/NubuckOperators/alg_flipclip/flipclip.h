#pragma once

#include <QSlider>

#include "globals.h"
#include "phase_init.h"

BEGIN_EVENT_DEF(FlipClip_DistanceChanged)
    float dist;
END_EVENT_DEF

class FlipClipPanel : public OP::ALG::StandardAlgorithmPanel {
    Q_OBJECT
private:
    QSlider* _dist;
private slots:
    void OnDistanceChanged(int value);
public:
    enum { MAX_DISTANCE = 500 };

    FlipClipPanel();

    void Invoke() override;
};

class FlipClip : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;

    void Event_DistanceChanged(const EV::Event& event);
protected:
    const char* GetName() const;

    OP::ALG::Phase* Init(const Nubuck& nb);
public:
    FlipClip();
};