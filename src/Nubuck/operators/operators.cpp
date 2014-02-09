#include <nubuck_private.h>
#include <Nubuck\operators\operator_invoker.h>
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

void Operators::OnInvokeOperator(unsigned id) {
    printf("invoking operator with id = %d\n", id);
    Operator* op = NULL;
    op = ActiveOperator();
    if(op) {
        op->Finish();
        if(1 < _activeOps.size()) _activeOps.pop_back();
    }
    UI::OperatorPanel::Instance()->Clear();
    op = _ops[id].op;
    _activeOps.push_back(op);
    op->Invoke();
}

Operators::~Operators() {
    UnloadModules();
}

unsigned Operators::Register(Operator* op, HMODULE module) {
    unsigned id = _ops.size();

    Invoker* invoker = new Invoker(id);
    QObject::connect(invoker, SIGNAL(SigInvokeOperator(unsigned)), this, SLOT(OnInvokeOperator(unsigned)));

    op->Register(nubuck, *invoker);

    OperatorDesc desc;
    desc.id = id;
    desc.invoker = invoker;
    desc.op = op;
	desc.module = module;

    _ops.push_back(desc);

    return id;
}

void Operators::SetInitOp(unsigned id) {
    assert(_activeOps.empty());
    OnInvokeOperator(id);
}

Operator* Operators::ActiveOperator() {
    if(_activeOps.empty()) return NULL;
    return _activeOps.back();
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
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnCameraChanged();
    }
}

bool Operators::OnMouseDown(const M::Vector2& mouseCoords, bool shiftKey) {
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