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

    std::vector<OperatorDesc>   _ops;       // all registered operators
    std::vector<Operator*>      _activeOps; // active operators
    SYS::SpinLock               _activeOpsMtx;
    unsigned                    _actionsPending;

	std::vector<R::MeshJob>     _meshJobs;
    SYS::SpinLock               _meshJobsMtx;

    GEN::Pointer<Driver>        _driver;

    bool IsActiveOperatorBusy() const { return 0 <_actionsPending; }

    void UnloadModules();

    void Event_ActionFinished(const EV::Event& event);
    void Event_SetPanel(const EV::Event& event);
    void Event_EditModeChanged(const EV::Event& event);
    void Event_Default(const EV::Event& event, const char* className);
public slots:
    void OnInvokeOperator(unsigned id);
public:
    Operators();
    ~Operators();

    void FrameUpdate();

    unsigned Register(OperatorPanel* panel, Operator* op, HMODULE module = NULL);

    void InvokeAction(const EV::Event& event);
    void InvokeEvent(const EV::Event& event);

    void SetInitOp(unsigned id);

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs);

    void OnCameraChanged();
    bool MouseEvent(const EV::Event& event);
};

void LoadOperators();

extern Operators g_operators;

} // namespace OP