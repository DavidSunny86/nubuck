#include <nubuck_private.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\operators\operator_invoker.h>
#include <UI\window_events.h>
#include <UI\userinterface.h>
#include <world\world_events.h>
#include <world\world.h>
#include "operator_events.h"
#include "operator_driver.h"
#include "operators.h"

namespace OP {

Operators g_operators;

void Operators::Event_ActionFinished(const EV::Event& event) {
    _actionsPending--;
}

void Operators::Event_SetPanel(const EV::Event& event) {
    const EV::Params_OP_SetPanel& args = EV::def_OP_SetPanel.GetArgs(event);
    Operator* op = args.op;
    OperatorPanel* panel = NULL;
    for(unsigned i = 0; !panel && i < _ops.size(); ++i) {
        if(_ops[i].op == op) panel = _ops[i].panel;
    }
    assert(panel);
    g_ui.GetOperatorPanel().Clear();
    nubuck.ui->SetOperatorPanel(panel);
}

void Operators::Event_ForwardToDriver(const EV::Event& event) {
    _driver->Send(event);
}

void Operators::OnInvokeOperator(unsigned id) {
    if(BUSY_THRESHOLD < _actionsPending) {
        printf("INFO - Operators::OnInvokeOperator: wait for driver.\n");
        return;
	}

    printf("invoking operator with id = %d\n", id);
    g_ui.GetOperatorPanel().Clear();
    Operator* op = _ops[id].op;
    nubuck.ui->SetOperatorPanel(_ops[id].panel);

	if(!_driver.IsValid()) {
        _driver = GEN::MakePtr(new Driver(_activeOps, _activeOpsMtx, _meshJobs, _meshJobsMtx));
        _driver->Thread_StartAsync();
	}
	EV::Params_OP_Push args = { op };
    _ops[id].panel->Invoke();
    _actionsPending++;
	_driver->Send(EV::def_OP_Push.Create(args));
}

Operators::Operators() : _actionsPending(0) {
    AddEventHandler(EV::def_OP_ActionFinished, this, &Operators::Event_ActionFinished);
    AddEventHandler(EV::def_OP_SetPanel, this, &Operators::Event_SetPanel);

    // forward other known events
    AddEventHandler(EV::def_EditModeChanged, this, &Operators::Event_ForwardToDriver);
    AddEventHandler(EV::def_SelectionChanged, this, &Operators::Event_ForwardToDriver);
}

Operators::~Operators() {
    UnloadModules();
}

void Operators::FrameUpdate() {
    HandleEvents();
}

unsigned Operators::Register(OperatorPanel* panel, Operator* op, HMODULE module) {
    unsigned id = _ops.size();

    Invoker* invoker = new Invoker(id);
    connect(invoker, SIGNAL(SigInvokeOperator(unsigned)), this, SLOT(OnInvokeOperator(unsigned)));

    op->Register(nubuck, *invoker);

    OperatorDesc desc;
    desc.id = id;
    desc.op = op;
    desc.invoker = invoker;
	desc.module = module;
    desc.panel = panel;

    _ops.push_back(desc);

    return id;
}

void Operators::InvokeAction(const EV::Event& event) {
    if(BUSY_THRESHOLD < _actionsPending) {
	    printf("INFO - Operators::InvokeAction: wait for driver.\n");
        return;
    }
    _actionsPending++;
    _driver->Send(event);
}

void Operators::SetInitOp(unsigned id) {
    assert(_activeOps.empty());
    OnInvokeOperator(id);
}

void Operators::GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
	if(!_renderThread.IsValid()) {
        _renderThread = GEN::MakePtr(new RenderThread(_activeOps, _activeOpsMtx, _meshJobs, _meshJobsMtx));
        // _renderThread->Thread_StartAsync();
	}

    // gather jobs synchronously
    // NOTE: blocks when driver is busy. eg. op_loop
    _renderThread->GatherJobs();
    
	SYS::ScopedLock lockJobs(_meshJobsMtx);
	meshJobs.insert(meshJobs.end(), _meshJobs.begin(), _meshJobs.end());
}

void Operators::OnCameraChanged() {
    _driver->Send(EV::def_CameraChanged.Create(EV::Params_CameraChanged()));
}

bool Operators::MouseEvent(const EV::Event& event) {
    _driver->Send(event);
    return true;
}

NUBUCK_API void SendToOperator(const EV::Event& event) {
    g_operators.InvokeAction(event);
}

} // namespace OP