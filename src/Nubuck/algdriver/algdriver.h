#pragma once

#include <Nubuck\nubuck.h>
#include <generic\pointer.h>
#include <generic\singleton.h>
#include <common\types.h>
#include <system\thread\thread.h>

namespace ALG {

    class Driver : public SYS::Thread {
    private:
        GEN::Pointer<IAlgorithm>    _algorithm;
        GEN::Pointer<IPhase>        _phase;

        void SetPhase(GEN::Pointer<IPhase> phase);
    public:
        Driver(const GEN::Pointer<IAlgorithm>& algorithm, const GEN::Pointer<IPhase>& phase);

        void Terminate(void);

        void Step(void);
        void Next(void);
        void Run(void);

        DWORD Thread_Func(void);
    };

    class Algorithm {
    private:
        algAlloc_t  _algAlloc;
        graph_t     _G;
        Driver*     _driver;
    public:
        Algorithm(void);

        void SetAlloc(algAlloc_t algAlloc);
        void Init(const graph_t& G);

        void Reset(void);

        void Step(void) { _driver->Step(); }
        void Next(void) { _driver->Next(); }
        void Run(void) { _driver->Run(); }
    };

    // global interface to client algorithm
    extern Algorithm gs_algorithm;

} // namespace ALG