#include <nubuck_private.h>
#include <system\locks\scoped_lock.h>
#include <Nubuck\operators\operator_invoker.h>
#include <UI\window_events.h>
#include <world\world_events.h>
#include "operator_events.h"
#include "operator_driver.h"
#include "operators.h"

namespace OP {

Operators g_operators;

void Operators::UnloadModules() {
	for(unsigned i = 0; i < _ops.size(); ++i) {
        OperatorDesc& desc = _ops[i];
		if(desc.module) {
            FreeLibrary(desc.module);
			desc.module = NULL;
		}
	}
}

void Operators::Event_ActionFinished(const EV::Event& event) {
    _actionsPending--;
}

void Operators::OnInvokeOperator(unsigned id) {
    if(0 < _actionsPending) {
		printf("op still busy...\n");
        return;
	}

    printf("invoking operator with id = %d\n", id);
    UI::OperatorPanel::Instance()->Clear();
    Operator* op = _ops[id].op;
    nubuck.ui->SetOperatorPanel(_ops[id].panel);

	if(!_driver.IsValid()) {
        _driver = GEN::MakePtr(new Driver(_activeOps, _activeOpsMtx, _meshJobs, _meshJobsMtx));
        _driver->Thread_StartAsync();
	}
	EV::Params_OP_Push args = { op };
	_driver->Send(EV::def_OP_Push.Create(args));
}

Operators::Operators() : _actionsPending(0) {
    AddEventHandler(EV::def_OP_ActionFinished, this, &Operators::Event_ActionFinished);
}

Operators::~Operators() {
    UnloadModules();
}

void Operators::FrameUpdate() {
    HandleEvents();
}

unsigned Operators::Register(QWidget* panel, Operator* op, HMODULE module) {
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
    if(0 < _actionsPending) {
		printf("op still busy...\n");
        return;
	}

    _actionsPending++;
    printf("Operators::InvokeAction\n");
    _driver->Send(event);
}

void Operators::InvokeEvent(const EV::Event& event) {
	_driver->Send(event);
}

void Operators::SetInitOp(unsigned id) {
    assert(_activeOps.empty());
    OnInvokeOperator(id);
}

void Operators::GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
	if(!_renderThread.IsValid()) {
        _renderThread = GEN::MakePtr(new RenderThread(_activeOps, _activeOpsMtx, _meshJobs, _meshJobsMtx));
        _renderThread->Thread_StartAsync();
	}
    
	SYS::ScopedLock lockJobs(_meshJobsMtx);
	meshJobs.insert(meshJobs.end(), _meshJobs.begin(), _meshJobs.end());
}

void Operators::OnCameraChanged() {
	_driver->Send(EV::def_CameraChanged.Create(EV::Params_CameraChanged()));
}

bool Operators::MouseEvent(const EV::Event& event) {
    if(0 < _actionsPending) return false;

    int ret = 0;
	EV::Params_Mouse args = EV::def_Mouse.GetArgs(event);
	args.ret = &ret;
	Operator* oldOp = _driver->GetActiveOperator();
	EV::Event ev2 = EV::def_Mouse.Create(args);
	_driver->SendAndWait(ev2);
	if(ret) {
		UI::OperatorPanel::Instance()->Clear();
        int id = -1;
		for(int i = 0; i < _ops.size(); ++i) {
			if(_ops[i].op == _driver->GetActiveOperator()) id = i;
		} 
        assert(0 <= id);
		if(oldOp != _ops[id].op) nubuck.ui->SetOperatorPanel(_ops[id].panel);
        return true;
	}
    return false;
}

} // namespace OP