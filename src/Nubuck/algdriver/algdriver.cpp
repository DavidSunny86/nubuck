#include <nubuck_private.h>
#include "algdriver.h"

namespace ALG {

    Algorithm gs_algorithm;

    void Driver::SetPhase(GEN::Pointer<IPhase> phase) {
        if(_phase.IsValid()) _phase->Leave();
        _phase = phase;
        if(_phase.IsValid()) _phase->Enter();
    }

    Driver::Driver(const GEN::Pointer<IAlgorithm>& algorithm, const GEN::Pointer<IPhase>& phase) 
        : _algorithm(algorithm)  
    { 
        SetPhase(phase);
    }

    void Driver::Terminate(void) {
        SetPhase(GEN::Pointer<IPhase>());
    }

    void Driver::Step(void) {
        if(_phase.IsValid()) {
            if(IPhase::DONE == _phase->Step()) {
                SetPhase(GEN::Pointer<IPhase>(_phase->NextPhase()));
            }
        }
    }

    void Driver::Next(void) {
        if(_phase.IsValid()) {
            while(IPhase::DONE != _phase->Step());
            SetPhase(GEN::Pointer<IPhase>(_phase->NextPhase()));
        }
    }

    void Driver::Run(void) {
        if(_algorithm->Run()) {
            SetPhase(GEN::Pointer<IPhase>());
            return;
        }
        while(_phase.IsValid() && !_phase->IsDone()) Next();
    }

    DWORD Driver::Thread_Func(void) {
        return 0;
    }

    Algorithm::Algorithm(void) : _algAlloc(NULL), _driver(NULL) { }

    void Algorithm::SetAlloc(algAlloc_t algAlloc) {
        _algAlloc = algAlloc;
    }

    void Algorithm::Init(const graph_t& G) {
        assert(NULL != _algAlloc);
        _G = G;
        Reset();
    }

    void Algorithm::Reset(void) {
        if(_driver) _driver->Terminate();
        GEN::Pointer<IAlgorithm> algorithm(_algAlloc());
        GEN::Pointer<IPhase> phase(algorithm->Init(nubuck, _G));
        _driver = new Driver(algorithm, phase);
    }

} // namespace ALG