#include <nubuck_private.h>
#include <Nubuck\operators\operator_invoker.h>
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
    printf("invoking operator with id = %d\n", id);
    UI::OperatorPanel::Instance()->Clear();
    Operator* op = _ops[id].op;
    nubuck.ui->SetOperatorPanel(_ops[id].panel);

	if(!_driver.IsValid()) {
        _driver = GEN::MakePtr(new Driver(_activeOps, _activeOpsMtx));
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
    _actionsPending++;
    printf("Operators::InvokeAction\n");
    _driver->Send(event);
}

void Operators::SetInitOp(unsigned id) {
    assert(_activeOps.empty());
    OnInvokeOperator(id);
}

void Operators::GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
    for(unsigned i = 0; i < _activeOps.size(); ++i)
        _activeOps[i]->GetMeshJobs(meshJobs);
}

void Operators::SelectGeometry() {
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnGeometrySelected();
    }
}

void Operators::OnCameraChanged() {
    if(IsActiveOperatorBusy()) return;

    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnCameraChanged();
    }
}

bool Operators::OnMouseDown(const M::Vector2& mouseCoords, bool shiftKey) {
    if(IsActiveOperatorBusy()) return false;

    for(int i = _activeOps.size() - 1; 0 <= i; --i) {
        Operator* op = _activeOps[i];
        if(op->OnMouseDown(mouseCoords, shiftKey)) {
            UI::OperatorPanel::Instance()->Clear();
            unsigned N = _activeOps.size() - 1 - i;
            for(unsigned j = 0; j < N; ++j) {
                _activeOps.back()->Finish();
                _activeOps.pop_back();
            }
            if(i != _activeOps.size() - 1) op->Invoke();
            return true;
        }
    }
    return false;
}

bool Operators::OnMouseMove(const M::Vector2& mouseCoords) {
    if(IsActiveOperatorBusy()) return false;

    for(int i = _activeOps.size() - 1; 0 <= i; --i) {
        Operator* op = _activeOps[i];
        if(op->OnMouseMove(mouseCoords)) {
            UI::OperatorPanel::Instance()->Clear();
            unsigned N = _activeOps.size() - 1 - i;
            for(unsigned j = 0; j < N; ++j) {
                _activeOps.back()->Finish();
                _activeOps.pop_back();
            }
            if(i != _activeOps.size() - 1) op->Invoke();
            return true;
        }
    }
    return false;
}

bool Operators::OnMouseUp(const M::Vector2& mouseCoords) {
    if(IsActiveOperatorBusy()) return false;

    for(int i = _activeOps.size() - 1; 0 <= i; --i) {
        Operator* op = _activeOps[i];
        if(op->OnMouseUp(mouseCoords)) {
            UI::OperatorPanel::Instance()->Clear();
            unsigned N = _activeOps.size() - 1 - i;
            for(unsigned j = 0; j < N; ++j) {
                _activeOps.back()->Finish();
                _activeOps.pop_back();
            }
            if(i != _activeOps.size() - 1) op->Invoke();
            return true;
        }
    }
    return false;
}

} // namespace OP