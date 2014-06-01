#include "flipclip.h"

const char* FlipClip::GetName() const { return "Flip & Clip"; }

OP::ALG::Phase* FlipClip::Init(const Nubuck& nb) {
    _g.nb = nb;
    return new Phase_Init(_g);
}