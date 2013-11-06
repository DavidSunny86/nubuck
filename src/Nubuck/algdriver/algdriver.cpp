#include <nubuck_private.h>
#include "algdriver.h"

namespace ALG {

    Algorithm gs_algorithm;

    void Driver::SetPhase(GEN::Pointer<IPhase> phase) {
        if(_phase.IsValid()) _phase->Leave();
        _phase = phase;
        if(_phase.IsValid()) _phase->Enter();
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

    Driver::Driver(const GEN::Pointer<IAlgorithm>& algorithm, const GEN::Pointer<IPhase>& phase) 
        : _cmdsSem(0), _algorithm(algorithm)
    { 
        SetPhase(phase);
    }

    void Driver::AddCommand(unsigned cmd) {
        _cmdsMtx.Lock();
        _cmds.push(cmd);
        _cmdsMtx.Unlock();
        _cmdsSem.Signal();
    }

    DWORD Driver::Thread_Func(void) {
        while(true) {
            _cmdsSem.Wait();
            _cmdsMtx.Lock();
            assert(!_cmds.empty());
            unsigned cmd = _cmds.front();
            _cmds.pop();
            _cmdsMtx.Unlock();

            switch(cmd) {
            case CMD_STEP: Step(); break;
            case CMD_NEXT: Next(); break;
            case CMD_RUN: Run(); break;
            };
        }
        return 0;
    }

    Algorithm::Algorithm(void) : _init(false), _algAlloc(NULL), _driver(NULL) { }

    void Algorithm::SetAlloc(algAlloc_t algAlloc) {
        _algAlloc = algAlloc;
    }

    void Algorithm::Init(const graph_t& G) {
        assert(NULL != _algAlloc);
        _G = G;
        _init = true;
        Reset();
    }

    void Algorithm::Reset(void) {
        if(_driver) {
            _driver->Thread_Kill();
            delete _driver;
            _driver = NULL;
        }

        if(_init) {
            GEN::Pointer<IAlgorithm> algorithm(_algAlloc());
            GEN::Pointer<IPhase> phase(algorithm->Init(nubuck, _G));
            _driver = new Driver(algorithm, phase);
            _driver->Thread_StartAsync();
        }
    }

    void Algorithm::Step(void) { if(_driver) _driver->AddCommand(Driver::CMD_STEP); }
    void Algorithm::Next(void) { if(_driver) _driver->AddCommand(Driver::CMD_NEXT); }
    void Algorithm::Run(void) { if(_driver) _driver->AddCommand(Driver::CMD_RUN); }

    GEN::Pointer<IPhase> Algorithm::GetPhase(void) {
        if(_driver) return _driver->GetPhase();
        return GEN::Pointer<IPhase>();
    }

} // namespace ALG