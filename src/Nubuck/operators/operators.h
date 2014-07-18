#pragma once

#include <assert.h>

#include <QObject>

#include <vector>
#include <Nubuck\operators\operator.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <Nubuck\events\events.h>
#include <renderer\renderer.h>
#include <system\thread\thread.h>
#include <Nubuck\system\locks\scoped_lock.h>

namespace OP {

class Driver;

class Operators : public QObject, public EV::EventHandler<> {
    Q_OBJECT
private:
    struct RenderThread : SYS::Thread {
        std::vector<Operator*>&     activeOps;
        SYS::SpinLock&          	activeOpsMtx;
        std::vector<R::MeshJob>&    meshJobs;
        SYS::SpinLock&              meshJobsMtx;

        RenderThread(
			std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx,
            std::vector<R::MeshJob>& meshJobs, SYS::SpinLock& meshJobsMtx) :
		    activeOps(activeOps), activeOpsMtx(activeOpsMtx),
			meshJobs(meshJobs), meshJobsMtx(meshJobsMtx) { }

        void GatherJobs() {
            SYS::ScopedLock lockOps(activeOpsMtx);
            SYS::ScopedLock lockJobs(meshJobsMtx);
            meshJobs.clear();
            for(unsigned i = 0; i < activeOps.size(); ++i)
                activeOps[i]->GetMeshJobs(meshJobs);
        }

        DWORD Thread_Func() {
            int cnt = 0;
            while(true) {
                GatherJobs();
                Sleep(100);
			}
		}
	};
    GEN::Pointer<RenderThread> _renderThread;

    DECL_HANDLE_EVENTS(Operators)

    struct OperatorDesc {
        unsigned        id;
        Operator*   	op;
        Invoker*    	invoker;
        OperatorPanel*  panel;
        HMODULE         module;
    };

    std::vector<OperatorDesc>   _ops;               // all registered operators
    std::vector<Operator*>      _activeOps;         // active operators
    SYS::SpinLock               _activeOpsMtx;

    enum { BUSY_THRESHOLD = 24 };                   // driver is considered busy iff threshold < actionsPending
    unsigned                    _actionsPending;    // number of pending actions sent to the driver

	std::vector<R::MeshJob>     _meshJobs;
    SYS::SpinLock               _meshJobsMtx;

    GEN::Pointer<Driver>        _driver;

    void UpdateOperatorPanel();

    void UnloadModules();

    void Event_Push(const EV::Event& event);
    void Event_Pop(const EV::Event& event);
    void Event_ActionFinished(const EV::Event& event);
    void Event_SetPanel(const EV::Event& event);
    void Event_ForwardToDriver(const EV::Event& event);
public slots:
    void OnInvokeOperator(unsigned id);
public:
    Operators();
    ~Operators();

    unsigned GetDriverQueueSize() const;

    void FrameUpdate();

    unsigned Register(OperatorPanel* panel, Operator* op, HMODULE module = NULL);

    struct InvokationMode {
        enum Enum {
            ALWAYS = 0,
            DROP_WHEN_BUSY
        };
    };

    void InvokeAction(const EV::Event& event, InvokationMode::Enum mode = InvokationMode::DROP_WHEN_BUSY);

    void SetInitOp(unsigned id);

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs);
};

void LoadOperators();

extern Operators g_operators;

} // namespace OP