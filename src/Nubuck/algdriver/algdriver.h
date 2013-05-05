#pragma once

#include <Nubuck\nubuck.h>
#include <generic\pointer.h>
#include <generic\singleton.h>
#include <common\types.h>

namespace ALG {

    class Driver : public GEN::Singleton<Driver> {
        friend class GEN::Singleton<Driver>;
    private:
        algAlloc_t _algAlloc;

        graph_t _G;

        GEN::Pointer<IAlgorithm>    _algorithm;
        GEN::Pointer<IPhase>        _phase;

        void SetPhase(GEN::Pointer<IPhase> phase);
    public:
        Driver(void);

        void SetAlloc(algAlloc_t algAlloc);
        void Init(const graph_t& G);

        void Reset(void);
        void Step(void);
    };

} // namespace ALG