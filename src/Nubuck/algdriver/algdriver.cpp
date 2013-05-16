#include <nubuck_private.h>
#include "algdriver.h"

namespace ALG {

    void Driver::SetPhase(GEN::Pointer<IPhase> phase) {
        if(_phase.IsValid()) _phase->Leave();
        _phase = phase;
        if(_phase.IsValid()) _phase->Enter();
    }

    Driver::Driver(void) : _algAlloc(NULL) { }

    void Driver::SetAlloc(algAlloc_t algAlloc) {
        _algAlloc = algAlloc;
    }

    void Driver::Init(const graph_t& G) {
        assert(NULL != _algAlloc);
        _G = G;
        Reset();
    }

    void Driver::Reset(void) {
        SetPhase(GEN::Pointer<IPhase>()); // release phase before algorithm
        _algorithm = GEN::Pointer<IAlgorithm>(_algAlloc());
        SetPhase(GEN::Pointer<IPhase>(_algorithm->Init(nubuck, _G)));
    }

    void Driver::Step(void) {
        if(_phase.IsValid()) {
            if(IPhase::DONE == _phase->Step()) {
                SetPhase(GEN::Pointer<IPhase>(_phase->NextPhase()));
            }
        }
    }

} // namespace ALG