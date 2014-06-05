#include "phase_clip.h"

Phase_Clip::Phase_Clip(Globals& g) : _g(g) { }

void Phase_Clip::Enter() {
    _g.nb.log->printf("entering phase 'clip'\n");
}

Phase_Clip::StepRet::Enum Phase_Clip::Step() {
    return StepRet::DONE;
}