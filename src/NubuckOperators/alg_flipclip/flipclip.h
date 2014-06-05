#pragma once

#include <QSlider>

#include "globals.h"
#include "phase_init.h"

class FlipClipPanel : public OP::ALG::StandardAlgorithmPanel {
private:
    QSlider* _dist;
public:
    FlipClipPanel();
};

class FlipClip : public OP::ALG::StandardAlgorithm {
private:
    Globals _g;
protected:
    const char* GetName() const;

    OP::ALG::Phase* Init(const Nubuck& nb);
};