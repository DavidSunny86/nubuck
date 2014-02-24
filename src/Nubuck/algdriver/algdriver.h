#pragma once

#include <queue>

#include <Nubuck\nubuck.h>
#include <Nubuck\generic\pointer.h>
#include <Nubuck\generic\singleton.h>
#include <common\types.h>
#include <system\thread\thread.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\semaphore.h>

namespace ALG {

    class Driver : public SYS::Thread {
    private:
        std::queue<unsigned>    _cmds;
        SYS::SpinLock           _cmdsMtx; // protects _cmds
        SYS::Semaphore          _cmdsSem; // used to block on empty command queue

        GEN::Pointer<IAlgorithm>    _algorithm;
        GEN::Pointer<IPhase>        _phase;
        const graph_t&              _G;

        void SetPhase(GEN::Pointer<IPhase> phase);

        void Step(void);
        void Next(void);
        void Run(void);
    public:
        Driver(const GEN::Pointer<IAlgorithm>& algorithm, const graph_t& G);

        GEN::Pointer<IPhase> GetPhase(void) { return _phase; }

        enum Command { CMD_STEP = 0, CMD_NEXT, CMD_RUN, CMD_TERMINATE };
        void AddCommand(unsigned cmd);

        DWORD Thread_Func(void);
    };

    class Algorithm {
    private:
        bool        _init; // true iff Init() has been called at least once
        algAlloc_t  _algAlloc;
        graph_t     _G;
        Driver*     _driver;
    public:
        Algorithm(void);

        void SetAlloc(algAlloc_t algAlloc);
        void Init(const graph_t& G);

        void Reset(void);

        void Step(void);
        void Next(void);
        void Run(void);

        GEN::Pointer<IPhase> GetPhase(void);
    };

    // global interface to client algorithm
    extern Algorithm gs_algorithm;

} // namespace ALG